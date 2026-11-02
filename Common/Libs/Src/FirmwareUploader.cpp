#include "FirmwareUploader.h"
#include "Clock.h"
#include <cstring>
#include <cstdio>

namespace {
constexpr unsigned FW_MAX_BOARD_ID = 7; // Matches transmit_stm.py BOARD_ID_MAP (0–7).
constexpr unsigned FW_MAX_RETRIES = 3; // Retries after the initial block attempt.
constexpr uint32_t FW_COMMAND_TIMEOUT_MS = 100;
constexpr uint32_t FW_BLOCK_TIMEOUT_MS = 1000;
constexpr uint32_t FW_QUEUE_TIMEOUT_MS = 1000;
constexpr uint32_t FW_RESULT_TIMEOUT_MS = 2000;
constexpr uint32_t FW_DRAIN_TIMEOUT_MS = 1000;
}

FirmwareUploader* FirmwareUploader::s_instance_ = nullptr;

FirmwareUploader::FirmwareUploader(UART &uart, uint32_t queue_depth)
    : uart_(uart), queue_(queue_depth, sizeof(UploadBlock))
{
}

void FirmwareUploader::start()
{
    if (s_instance_) return; // The Thread API supports one uploader.
    s_instance_ = this;
    consumer_thread_.start([]{ s_instance_->consumer_task(s_instance_); });
    UART_listener_thread_.start([]{ s_instance_->uart_listener_task(s_instance_); });
}

// #129: size and image CRC make completion explicit. FW-prefixed responses
// can coexist with ordinary log lines on the same UART.
void FirmwareUploader::handle_uart_mock()
{
    char command[64];
    if (read_line(command, sizeof(command), FW_COMMAND_TIMEOUT_MS) < 0) return;
    unsigned board, expected_crc;
    unsigned long image_size;
    char extra;
    if (std::sscanf(command, "UPLOAD %u %lu %x %c", &board, &image_size, &expected_crc, &extra) != 3 ||
        board > FW_MAX_BOARD_ID || !image_size || image_size > FW_MAX_FIRMWARE_SIZE || expected_crc > 0xffff) {
        uart_.write((uint8_t*)"FW FAIL\n", 8);
        return;
    }

    uint32_t offset = 0;
    unsigned retries = 0;
    bool ok = true;
    UploadBlock block{};
    while (offset < image_size) {
        uint8_t packet[FW_BLOCK_SIZE + 2];
        if (uart_.write((uint8_t*)"FW REQ\n", 7) != 0 || !read_exact(packet, sizeof(packet), FW_BLOCK_TIMEOUT_MS)) {
            ok = false;
            break; // A timeout is a failed transfer, never end-of-file.
        }
        const uint16_t received_crc = (uint16_t(packet[FW_BLOCK_SIZE]) << 8) | packet[FW_BLOCK_SIZE + 1];
        if (crc16_hqx(packet, FW_BLOCK_SIZE) != received_crc) {
            if (++retries > FW_MAX_RETRIES || uart_.write((uint8_t*)"FW NACK\n", 8) != 0) {
                ok = false;
                break;
            }
            continue;
        }
        retries = 0;
        std::memcpy(block.data, packet, FW_BLOCK_SIZE);
        block.length = (image_size - offset < FW_BLOCK_SIZE) ? image_size - offset : FW_BLOCK_SIZE;
        if (queue_.append_to_back(&block, FW_QUEUE_TIMEOUT_MS) != 0) {
            ok = false;
            break;
        }
        offset += block.length;
        if (uart_.write((uint8_t*)"FW ACK\n", 7) != 0) { ok = false; break; }
    }

    // The end marker is ordered after accepted blocks, including on failure.
    // Wait for the consumer's count/CRC before reporting and reusing its queue.
    block.length = 0;
    UploadResult result{};
    if (queue_.append_to_back(&block, FW_QUEUE_TIMEOUT_MS) != 0 || result_queue_.get(&result, FW_RESULT_TIMEOUT_MS) != 0) {
        uart_.write((uint8_t*)"FW FAIL\n", 8);
        Clock::sleep_forever(); // Infrastructure failure: don't mix later uploads with undrained data.
        return;
    }
    ok = ok && offset == image_size && result.bytes == image_size && result.crc == expected_crc;
    char response[48];
    const int length = ok ? std::snprintf(response, sizeof(response), "FW DONE %lu %04x\n",
                                         static_cast<unsigned long>(result.bytes), result.crc)
                          : std::snprintf(response, sizeof(response), "FW FAIL\n");
    uart_.write(reinterpret_cast<uint8_t*>(response), length);
    if (!ok) {
        // Discard a late/partial packet until the link is quiet, with a bound.
        uint8_t byte;
        const uint32_t started = Clock::get_current_time();
        while (uint32_t(Clock::get_current_time() - started) < FW_DRAIN_TIMEOUT_MS && uart_.read(&byte, 1, FW_COMMAND_TIMEOUT_MS) == 0) {}
    }
}

void FirmwareUploader::consumer_task(void *arg)
{
    FirmwareUploader *self = static_cast<FirmwareUploader *>(arg);
    UploadBlock block{};
    UploadResult result{0, FW_CRC16_INIT};
    while (true) {
        if (self->queue_.get(&block, FW_QUEUE_TIMEOUT_MS) != 0) { Clock::sleep_for(1); continue; }
        if (block.length == 0) {
            self->result_queue_.append_to_back(&result, FW_QUEUE_TIMEOUT_MS);
            result = {0, FW_CRC16_INIT};
        } else {
            result.crc = crc16_hqx(block.data, block.length, result.crc);
            result.bytes += block.length;
        }
    }
}

void FirmwareUploader::uart_listener_task(void* arg)
{
    FirmwareUploader *self = static_cast<FirmwareUploader *>(arg);
    while (true) self->handle_uart_mock();
}

bool FirmwareUploader::read_exact(uint8_t *buf, uint16_t len, uint32_t timeout_ms)
{
    return uart_.read(buf, len, timeout_ms) == 0;
}

int FirmwareUploader::read_line(char *buf, uint16_t max_len, uint32_t timeout_ms)
{
    for (uint16_t i = 0; i < max_len - 1; i++)
    {
        if (uart_.read((uint8_t *)&buf[i], 1, timeout_ms) != 0)
            return -1;
        if (buf[i] == '\n')
        {
            buf[i + 1] = '\0';
            return i + 1;
        }
    }
    return -1;
}

uint16_t FirmwareUploader::crc16_hqx(const uint8_t* data, uint16_t len, uint16_t crc) {
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

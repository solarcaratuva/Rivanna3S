#include "FirmwareUploader.h"
#include "Clock.h"
#include "log.h"
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
// The mock consumer only receives blocks and computes their CRC.
// Keep the UART listener's existing stack for parsing and fault logging.
constexpr uint16_t FW_CONSUMER_STACK_WORDS = 256;
constexpr uint16_t FW_UART_STACK_WORDS = 1000;
// Rearm command-byte reception before CPU-bound application tasks run again.
// A 1 ms tick at the application priority can overrun the 115200-baud FIFO.
constexpr UBaseType_t FW_UART_PRIORITY = tskIDLE_PRIORITY + 2;
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
    consumer_thread_.start(
        []{ s_instance_->consumer_task(s_instance_); },
        FW_CONSUMER_STACK_WORDS);
    UART_listener_thread_.start(
        []{ s_instance_->uart_listener_task(s_instance_); },
        FW_UART_STACK_WORDS, FW_UART_PRIORITY);
}

bool FirmwareUploader::send_line(const char *line)
{
    return uart_.write(reinterpret_cast<uint8_t*>(const_cast<char*>(line)), std::strlen(line)) == 0;
}

// #129: size and image CRC make completion explicit. FW-prefixed responses
// can coexist with ordinary log lines on the same UART.
void FirmwareUploader::handle_uart_mock()
{
    char command[64];
    if (read_line(command, sizeof(command), FW_COMMAND_TIMEOUT_MS) < 0) return;
    if (std::strncmp(command, "UPLOAD ", 7) != 0) return; // Discard leading stray bytes or blank lines

    unsigned board, expected_crc;
    unsigned long image_size;
    char extra;
    if (std::sscanf(command, "UPLOAD %u %lu %x %c", &board, &image_size, &expected_crc, &extra) != 3 ||
        board > FW_MAX_BOARD_ID || !image_size || image_size > FW_MAX_FIRMWARE_SIZE || expected_crc > 0xffff) {
        send_line("FW FAIL\n");
        return;
    }

    uint32_t offset = 0;
    unsigned retries = 0;
    bool ok = true;
    UploadBlock block{};
    while (offset < image_size) {
        uint8_t packet[FW_BLOCK_SIZE + 2];
        if (!send_line("FW REQ\n") || !read_exact(packet, sizeof(packet), FW_BLOCK_TIMEOUT_MS)) {
            ok = false;
            break; // A timeout is a failed transfer, never end-of-file.
        }
        const uint16_t received_crc = (uint16_t(packet[FW_BLOCK_SIZE]) << 8) | packet[FW_BLOCK_SIZE + 1];
        if (crc16_hqx(packet, FW_BLOCK_SIZE) != received_crc) {
            if (++retries > FW_MAX_RETRIES || !send_line("FW NACK\n")) {
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
        if (!send_line("FW ACK\n")) { ok = false; break; }
    }

    // The end marker is ordered after accepted blocks, including on failure.
    // Wait for the consumer's count/CRC before reporting and reusing its queue.
    block.length = 0;
    UploadResult result{};
    if (queue_.append_to_back(&block, FW_QUEUE_TIMEOUT_MS) != 0 || result_queue_.get(&result, FW_RESULT_TIMEOUT_MS) != 0) {
        send_line("FW FAIL\n");
        log_fault("FirmwareUploader: consumer queue failure; listener permanently stopped until reset");
        Clock::sleep_forever();
        return;
    }
    ok = ok && offset == image_size && result.bytes == image_size && result.crc == expected_crc;
    char response[48];
    if (ok) {
        std::snprintf(response, sizeof(response), "FW DONE %lu %04x\n",
                      static_cast<unsigned long>(result.bytes), result.crc);
    } else {
        std::snprintf(response, sizeof(response), "FW FAIL\n");
    }
    if (!ok) {
        // Drain late packet bytes before reporting failure so a sender
        // restarting after FW FAIL does not have its command discarded.
        uint8_t byte;
        const uint32_t started = Clock::get_current_time();
        while (uint32_t(Clock::get_current_time() - started) < FW_DRAIN_TIMEOUT_MS && uart_.read(&byte, 1, FW_COMMAND_TIMEOUT_MS) == 0) {}
    }
    send_line(response);
}

void FirmwareUploader::consumer_task(void *arg)
{
    FirmwareUploader *self = static_cast<FirmwareUploader *>(arg);
    UploadBlock block{};
    UploadResult result{0, FW_CRC16_INIT};
    while (true) {
        // Blocks up to FW_QUEUE_TIMEOUT_MS; loop and wait again if queue is empty.
        if (self->queue_.get(&block, FW_QUEUE_TIMEOUT_MS) != 0) {
            continue;
        }
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
    uint16_t used = 0;
    while (used < max_len - 1) {
        uint16_t received = 0;
        // Buffer the command in the ISR instead of rescheduling for each byte.
        if (uart_.read_until_idle(reinterpret_cast<uint8_t*>(buf + used),
                                  max_len - used - 1, received, timeout_ms) != 0 || !received)
            return -1;
        const uint16_t end = used + received;
        for (uint16_t i = used; i < end; ++i) {
            if (buf[i] == '\n') {
                buf[i + 1] = '\0';
                return i + 1;
            }
        }
        used = end;
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

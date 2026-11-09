#pragma once

#include <stdint.h>
#include "UART.h"
#include "FiniteQueue.h"
#include "thread.h"

#define FW_BLOCK_SIZE 64 // Must match Python sender's block size.
#define FW_MAX_FIRMWARE_SIZE (4096 * 1024) // Mock transport limit, not flash capacity.
#define FW_CRC16_INIT 0xFFFF
#define FW_UPLOAD_BAUD 115200 // Default baud rate matching transmit_stm.py.

// Laptop-to-board receiver with a streaming mock consumer.
// Destination IDs 0–7 are accepted; no CAN routing or flash operations occur.
class FirmwareUploader
{
public:
    explicit FirmwareUploader(UART &uart, uint32_t queue_depth = 8);
    void start(); // Start the single UART listener and mock consumer once.

    // Matches Python's binascii.crc_hqx, seeded with FFFF.
    static uint16_t crc16_hqx(const uint8_t *data, uint16_t len, uint16_t crc = FW_CRC16_INIT);

private:
    struct UploadBlock {
        uint8_t data[FW_BLOCK_SIZE];
        uint16_t length; // Zero marks the end of a mock transfer.
    };
    struct UploadResult { uint32_t bytes; uint16_t crc; };
    FiniteQueue result_queue_{1, sizeof(UploadResult)};
    UART &uart_;
    FiniteQueue queue_;
    Thread consumer_thread_;
    Thread uart_listener_thread_;
    static FirmwareUploader* s_instance_;

    void handle_upload_command();
    bool parse_upload_command(const char *command, uint32_t &image_size,
                              uint16_t &expected_crc);
    bool receive_blocks(uint32_t image_size);
    void finish_upload(bool ok, uint32_t image_size, uint16_t expected_crc);
    static void consumer_task(void* arg);
    static void uart_listener_task(void* arg);
    bool send_line(const char *line);
    int read_line(char *buf, uint16_t max_len, uint32_t timeout_ms);
};

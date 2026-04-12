#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "UART.h"
#include "FiniteQueue.h"
#include "thread.h"
#include "Clock.h"

#define FW_BLOCK_SIZE         64         // Must match Python sender's block size
#define FW_MAX_FIRMWARE_SIZE  (4096 * 1024)  // 4 MB
#define FW_CRC16_INIT         0xFFFF

class FirmwareUploader {
public:
    /**
     * @brief Construct a FirmwareUploader.
     * @param uart          Reference to the UART peripheral to use.
     * @param queue_depth   Number of blocks the internal queue can hold.
     */
    FirmwareUploader(UART& uart, uint32_t queue_depth = 100);

    /**
     * @brief Start the background consumer thread that drains received blocks.
     *        Call once before entering the main command loop.
     */
    void start();

    /**
     * @brief Block until a valid board-ID command line arrives, then receive
     *        the firmware stream for that board.
     *
     *        The function reads a single-character board ID (0–4) followed by
     *        '\n', then calls handle_upload() internally.
     *
     *        Returns the board_id that was uploaded to, or -1 on parse error.
     */
    int run_once();

    /**
     * @brief CRC-16/HQXA (matches Python's binascii.crc_hqx).
     *        Exposed publicly so callers can verify blocks independently.
     */
    static uint16_t crc16_hqx(const uint8_t* data, uint16_t len);

private:
    UART&         uart_;
    FiniteQueue   queue_;
    Thread        consumer_thread_;
    Clock         clock_;

    /**
     * @brief Receive one firmware stream over UART into the queue.
     * @return true on clean completion (DONE), false on overflow or error.
     */
    bool handle_upload();

    /** @brief Background thread entry: drains queue_ and processes blocks. */
    static void consumer_task(void* arg);

    /** @brief Read exactly `len` bytes with a millisecond timeout. */
    bool read_exact(uint8_t* buf, uint16_t len, uint32_t timeout_ms);

    /** @brief Read a '\n'-terminated line. Returns byte count or -1 on timeout. */
    int  read_line(char* buf, uint16_t max_len, uint32_t timeout_ms);
};
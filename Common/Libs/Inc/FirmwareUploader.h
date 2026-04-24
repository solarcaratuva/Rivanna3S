#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "UART.h"
#include "FiniteQueue.h"
#include "thread.h"
#include "Clock.h"
#include "CanInterface.h"

#define FW_BLOCK_SIZE 64                   // Must match Python sender's block size
#define FW_MAX_FIRMWARE_SIZE (4096 * 1024) // 4 MB
#define FW_CRC16_INIT 0xFFFF

class FirmwareUploader
{
public:
    /**
     * @brief Construct a FirmwareUploader.
     * @param uart          Reference to the UART peripheral to use.
     * @param can           Reference to the CAN peripheral to use for communication with target boards.
     * @param queue_depth   Number of blocks the internal queue can hold.
     * @param current_board  ID of the board this instance is running on (0-4). Used for CAN routing.
     */
    FirmwareUploader(UART &uart, CanInterface &can, uint32_t current_board, uint32_t queue_depth = 100);

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
    static uint16_t crc16_hqx(const uint8_t *data, uint16_t len, uint16_t crc = FW_CRC16_INIT);

    /**
     * @brief Set the active flash bank (1 or 2).
     *        Bank 1: 0x08000000, Bank 2: 0x08100000
     */
    void set_flash_bank(uint8_t bank);

private:
    UART &uart_;
    CanInterface &can_;
    FiniteQueue queue_;
    Thread consumer_thread_;
    Thread UART_listener_thread_;
    Clock clock_;
    uint32_t flash_base_addr_ = 0x08100000UL; // Default to Bank 2
    uint32_t current_board_;
    uint32_t is_host_ = 0; // 0 for target, 1 for host
    static FirmwareUploader* s_instance_; // For static callback access

    /**
     * @brief Write a block of data to flash at the given address.
     * @param address Flash address to write to.
     * @param data Pointer to data buffer.
     * @param len Length of data in bytes (must be multiple of 32 for flash word).
     * @return true on success, false on failure.
     */
    bool write_flash(uint32_t address, const uint8_t *data, size_t len);

    /**
     * @brief Receive one firmware stream over UART into the queue.
     * @return true on clean completion (DONE), false on overflow or error.
     */
    bool handle_upload();

    /** @brief Background thread entry: drains queue_ and processes blocks. */
    static void consumer_task(void* arg);

    /** @brief Background thread entry: listens for UART commands to trigger uploads. */
    static void uart_listener_task(void* arg);

    /** @brief Read exactly `len` bytes with a millisecond timeout. */
    bool read_exact(uint8_t *buf, uint16_t len, uint32_t timeout_ms);

    /** @brief Read a '\n'-terminated line. Returns byte count or -1 on timeout. */
    int read_line(char *buf, uint16_t max_len, uint32_t timeout_ms);

    //--------------------------------------------------
    // TARGET BOARD UPDATE HANDLING
    //--------------------------------------------------

    /** @brief Handle UpdateControl CAN messsage */
    void receive_update_control(SerializedCanMessage &msg);

    /** @brief Handle UpdateData CAN message */
    void receive_update_data(SerializedCanMessage &msg);

    /**
     * @brief Begin mass erase after SETUP command.
     */
    bool mass_erase();

    /**
     * @brief Handle DONE command from host.
     */
    void target_handle_done();

    //--------------------------------------------------
    // TARGET CAN TRANSMIT HELPERS
    //--------------------------------------------------

    void target_send_setup_ack();
    void target_send_ready_for_data();
    void target_send_done_with_crc(uint16_t crc);

    //--------------------------------------------------
    // TARGET UPDATE STATE
    //--------------------------------------------------

    enum class TargetUpdateState
    {
        IDLE,
        ERASING,
        READY_FOR_DATA,
        RECEIVING_DATA,
        FINALIZING
    };

    enum class HostBoardState
    {
        NONE,
        SETUP,
        SENDING_DATA,
        DONE,
        SUCCESS,
        FAILED
    };

    TargetUpdateState target_state_;

    HostBoardState host_board_state_ = HostBoardState::NONE;

    uint16_t target_running_crc_;
    uint16_t host_running_crc_;
};
#include "FirmwareUploader.h"
#include "log.h" // log_info / log_error — remove if not in your project
#include "FreeRTOS.h"
#include "task.h"
#include "stm32h7xx_hal.h"
#include <cstring>
#include "Rivanna3SCanStructs.h"

// ---------------------------------------------------------------------------
// Construction & lifecycle
// ---------------------------------------------------------------------------

FirmwareUploader::FirmwareUploader(UART &uart, CanInterface &can, uint32_t queue_depth)
    : uart_(uart),
      can_(can),
      queue_(queue_depth, FW_BLOCK_SIZE)
{
}

void FirmwareUploader::start()
{
    // Pass `this` so the static trampoline can reach instance members.
    // consumer_thread_.start([]() {
    //     // FreeRTOS tasks must not return; loop handled inside consumer_task.
    // });
    // Re-implement with proper argument passing via your Thread API:
    // consumer_thread_.start(consumer_task, this);
    //
    // If your Thread class only accepts a plain function pointer (no arg),
    // store `this` in a file-scope pointer and call it from a wrapper lambda:
    static FirmwareUploader* s_instance = this;
    consumer_thread_.start([]{ s_instance->consumer_task(s_instance); });
    UART_listener_thread_.start([]{ s_instance->uart_listener_task(s_instance); });
}

// ---------------------------------------------------------------------------
// Public command loop helper
// ---------------------------------------------------------------------------

int FirmwareUploader::run_once()
{
    char cmd_buf[32];

    if (read_line(cmd_buf, sizeof(cmd_buf), portMAX_DELAY) < 0)
        return -1;

    int board_id = cmd_buf[0];
    if (board_id >= '0' && board_id <= '9')
        board_id -= '0';

    if (board_id < 0 || board_id > 9)
        return -1;

    handle_upload(); // board_id available here for future CAN routing
    return board_id;
}

// ---------------------------------------------------------------------------
// Upload state machine
// ---------------------------------------------------------------------------

bool FirmwareUploader::handle_upload()
{
    uint8_t packet[FW_BLOCK_SIZE + 2]; // block + 2-byte CRC
    uint32_t offset = 0;

    while (offset < FW_MAX_FIRMWARE_SIZE)
    {
        uart_.write((uint8_t *)"REQ", 3);

        // Receive block + CRC; timeout means the sender has finished.
        if (!read_exact(packet, sizeof(packet), 1000))
        {
            uart_.write((uint8_t *)"DONE", 4);
            return true;
        }

        // Verify CRC
        uint16_t received_crc =
            ((uint16_t)packet[FW_BLOCK_SIZE] << 8) | packet[FW_BLOCK_SIZE + 1];
        uint16_t computed_crc = crc16_hqx(packet, FW_BLOCK_SIZE);

        if (computed_crc != received_crc)
        {
            uart_.write((uint8_t *)"NACK", 4);
            continue;
        }

        // Guard against overflow
        if (offset + FW_BLOCK_SIZE > FW_MAX_FIRMWARE_SIZE)
        {
            uart_.write((uint8_t *)"ERR", 4);
            return false;
        }

        // Push into queue
        int result = queue_.append_to_back(packet, portMAX_DELAY);
        if (result != 0)
        {
            if (result == -2)
                uart_.write((uint8_t *)"FAIL", 4);
            else if (result == -1)
                uart_.write((uint8_t *)"INIT\n", 5);

            uart_.write((uint8_t *)"NACK", 4);
            continue;
        }

        offset += FW_BLOCK_SIZE;
        uart_.write((uint8_t *)"ACK\n", 4);
    }

    uart_.write((uint8_t *)"ERR\n", 4);
    return false;
}

// ---------------------------------------------------------------------------
// Background Threads
// ---------------------------------------------------------------------------

void FirmwareUploader::consumer_task(void *arg)
{
    FirmwareUploader *self = static_cast<FirmwareUploader *>(arg);
    uint8_t block[FW_BLOCK_SIZE];
    uint32_t flash_addr = self->flash_base_addr_;
    bool targetstate_finished = false; // This should be set externally when the target state is finished
    while (true)
    {
        if (self->queue_.get(block, FW_BLOCK_SIZE))
        {
            // Write block to flash
            if (!self->write_flash(flash_addr, block, FW_BLOCK_SIZE))
            {
                log_fault("Flash write failed at 0x%08lX", flash_addr);
            }
            else
            {
                log_info("Block written to flash @ 0x%08lX", flash_addr);
            }
            flash_addr += FW_BLOCK_SIZE;
        }

        // Swap bank if finalizing_ is set and queue is empty
        if (self->FINALIZING && self->queue_.size() == 0)
        {
            if (self->flash_base_addr_ == 0x08000000UL)
                self->set_flash_bank(2);
            else
                self->set_flash_bank(1);
            log_info("Switched flash bank. New base: 0x%08lX", self->flash_base_addr_);
            flash_addr = self->flash_base_addr_;
            self->FINALIZING = false; // Reset flag
        }

        self->clock_.sleep_for(2);
    }
}

void FirmwareUploader::uart_listener_task(void* arg) {
    FirmwareUploader* self = static_cast<FirmwareUploader*>(arg);
    char cmd_buf[32];

    while (true) {
        if (self->read_line(cmd_buf, sizeof(cmd_buf), 10) > 0) {
            int board_id = cmd_buf[0];
            if (board_id >= '0' && board_id <= '9')
                board_id -= '0';
            else
                continue;

            
            self->is_host_ = 1; // Set flag to indicate we're in host mode

            //Create update control msg to send to target over CAN
            UpdateControl update_msg{};
            update_msg.target_board = board_id;
            update_msg.setup = 1; // signal sender is ready to send data
            can_.write(&update_msg);

        }
}

void FirmwareUploader::uart_listener_task(void* arg) {
    FirmwareUploader* self = static_cast<FirmwareUploader*>(arg);
    char cmd_buf[32];

    while (true) {
        if (self->read_line(cmd_buf, sizeof(cmd_buf), 10) > 0) {
            int board_id = cmd_buf[0];
            if (board_id >= '0' && board_id <= '9')
                board_id -= '0';
            else
                continue;

            
            self->is_host_ = 1; // Set flag to indicate we're in host mode

            //Create update control msg to send to target over CAN
            UpdateControl update_msg{};
            update_msg.target_board = board_id;
            update_msg.setup = 1; // signal sender is ready to send data
            can_.write(&update_msg);
        }
    }
}

void FirmwareUploader::set_flash_bank(uint8_t bank)
{
    if (bank == 1)
        flash_base_addr_ = 0x08000000UL; // Bank 1
    else
        flash_base_addr_ = 0x08100000UL; // Bank 2
}

// ---------------------------------------------------------------------------
// Flash writing logic (STM32H7 example)
// ---------------------------------------------------------------------------

bool FirmwareUploader::write_flash(uint32_t address, const uint8_t *data, size_t len)
{
    if (len % 32 != 0)
    {
        log_fault("write_flash: len must be multiple of 32");
        return false;
    }

    HAL_StatusTypeDef status;
    HAL_FLASH_Unlock();

    for (size_t i = 0; i < len; i += 32)
    {
        status = HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_FLASHWORD,
            address + i,
            (uint32_t)(data + i));
        if (status != HAL_OK)
        {
            log_fault("HAL_FLASH_Program failed @ 0x%08lX, err=0x%08lX", address + i, HAL_FLASH_GetError());
            HAL_FLASH_Lock();
            return false;
        }
    }

    HAL_FLASH_Lock();
    return true;
}

// ---------------------------------------------------------------------------
// UART helpers
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// CRC-16/HQXA
// ---------------------------------------------------------------------------

uint16_t FirmwareUploader::crc16_hqx(const uint8_t *data, uint16_t len)
{
    uint16_t crc = FW_CRC16_INIT;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

// ---------------------------------------------------------------------------
// Target board CAN handlers
// ---------------------------------------------------------------------------

void FirmwareUploader::target_receive_update_control(const SerializedCanMessage &msg)
{
    UpdateControl control{};
    control.deserialize(&msg);

    int target_board = control.target_board;
    if (target_board == current_board_)
    {
        if (control.setup)
        {
            target_send_setup_ack();
            target_begin_mass_erase();
        }
        if (control.done)
        {
            target_handle_done();
        }
    }
}
#include "FirmwareUploader.h"
#include "log.h"        // log_info / log_error — remove if not in your project
#include "FreeRTOS.h"
#include "task.h"
#include "Rivanna3SCanStructs.h"
#include "stm32h7xx_hal.h"
#include "stm32h743xx.h"

// ---------------------------------------------------------------------------
// Construction & lifecycle
// ---------------------------------------------------------------------------

FirmwareUploader::FirmwareUploader(UART& uart, CanInterface& can, uint32_t queue_depth, uint32_t current_board)
    : uart_(uart),
      can_(can),
      queue_(queue_depth, FW_BLOCK_SIZE),
      current_board_(current_board)
{}

void FirmwareUploader::start() {
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
}

// ---------------------------------------------------------------------------
// Public command loop helper
// ---------------------------------------------------------------------------

int FirmwareUploader::run_once() {
    char cmd_buf[32];

    if (read_line(cmd_buf, sizeof(cmd_buf), portMAX_DELAY) < 0)
        return -1;

    int board_id = cmd_buf[0];
    if (board_id >= '0' && board_id <= '9')
        board_id -= '0';

    if (board_id < 0 || board_id > 4)
        return -1;

    handle_upload();   // board_id available here for future CAN routing
    return board_id;
}

// ---------------------------------------------------------------------------
// Upload state machine
// ---------------------------------------------------------------------------

bool FirmwareUploader::handle_upload() {
    uint8_t  packet[FW_BLOCK_SIZE + 2];   // block + 2-byte CRC
    uint32_t offset = 0;

    while (offset < FW_MAX_FIRMWARE_SIZE) {
        uart_.write((uint8_t*)"REQ", 3);

        // Receive block + CRC; timeout means the sender has finished.
        if (!read_exact(packet, sizeof(packet), 1000)) {
            uart_.write((uint8_t*)"DONE", 4);
            return true;
        }

        // Verify CRC
        uint16_t received_crc =
            ((uint16_t)packet[FW_BLOCK_SIZE] << 8) | packet[FW_BLOCK_SIZE + 1];
        uint16_t computed_crc = crc16_hqx(packet, FW_BLOCK_SIZE);

        if (computed_crc != received_crc) {
            uart_.write((uint8_t*)"NACK", 4);
            continue;
        }

        // Guard against overflow
        if (offset + FW_BLOCK_SIZE > FW_MAX_FIRMWARE_SIZE) {
            uart_.write((uint8_t*)"ERR", 4);
            return false;
        }

        // Push into queue
        int result = queue_.append_to_back(packet, portMAX_DELAY);
        if (result != 0) {
            if (result == -2)
                uart_.write((uint8_t*)"FAIL", 4);
            else if (result == -1)
                uart_.write((uint8_t*)"INIT\n", 5);

            uart_.write((uint8_t*)"NACK", 4);
            continue;
        }

        offset += FW_BLOCK_SIZE;
        uart_.write((uint8_t*)"ACK\n", 4);
    }

    uart_.write((uint8_t*)"ERR\n", 4);
    return false;
}

// ---------------------------------------------------------------------------
// Background consumer
// ---------------------------------------------------------------------------

void FirmwareUploader::consumer_task(void* arg) {
    FirmwareUploader* self = static_cast<FirmwareUploader*>(arg);
    uint8_t block[FW_BLOCK_SIZE];

    

    while (true) {
        if (self->queue_.get(block, FW_BLOCK_SIZE)) {
            // TODO: forward block over CAN, write to flash, etc.
            // log_info("Block rx, first byte: %02X", block[0]);
        }
        self->clock_.sleep_for(2);
    }
}

// ---------------------------------------------------------------------------
// UART helpers
// ---------------------------------------------------------------------------

bool FirmwareUploader::read_exact(uint8_t* buf, uint16_t len, uint32_t timeout_ms) {
    return uart_.read(buf, len, timeout_ms) == 0;
}

int FirmwareUploader::read_line(char* buf, uint16_t max_len, uint32_t timeout_ms) {
    for (uint16_t i = 0; i < max_len - 1; i++) {
        if (uart_.read((uint8_t*)&buf[i], 1, timeout_ms) != 0)
            return -1;
        if (buf[i] == '\n') {
            buf[i + 1] = '\0';
            return i + 1;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// CRC-16/HQXA
// ---------------------------------------------------------------------------

uint16_t FirmwareUploader::crc16_hqx(const uint8_t* data, uint16_t len, uint16_t crc) {
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

// ---------------------------------------------------------------------------
// Target board CAN handlers
// ---------------------------------------------------------------------------

void FirmwareUploader::receive_update_control(const SerializedCanMessage &msg) {
    UpdateControl control{};
    control.deserialize(&msg);

    int target_board = control.target_board;
    if (is_host_) {
        // Do host actions
    }

    if (target_board == current_board_){
        if (control.setup) {
            target_send_setup_ack();
            target_running_crc_ = FW_CRC16_INIT; // Reset CRC for new upload
            target_state_ = TargetUpdateState::ERASING;
            if (!mass_erase()){
                return; // Erase failed, cannot proceed
            }
            target_state_ = TargetUpdateState::READY_FOR_DATA;
            target_send_ready_for_data();
        }
        if (control.done) {
            target_handle_done();
        }
    }
}

void FirmwareUploader::receive_update_data(const SerializedCanMessage &msg) {
    // No need to deserialize into a struct since it's just raw data blocks. Just forward to flash write.
    if (target_state_ == TargetUpdateState::RECEIVING_DATA) {
        // Update the running CRC with the new block of data
        target_running_crc_ = crc16_hqx(msg.data, msg.len, target_running_crc_);

        target_write_flash_block(msg.data, msg.len);
    }
}

bool FirmwareUploader::mass_erase() {
    FLASH_EraseInitTypeDef erase;
    uint32_t page_error = 0;
    uint32_t bank;

    if ((target_flash_address_ >= FLASH_BANK1_BASE) && (target_flash_address_ < FLASH_BANK2_BASE)) {
        bank = FLASH_BANK_1;
    } else if ((target_flash_address_ >= FLASH_BANK2_BASE) && (target_flash_address_ < FLASH_END)) {
        bank = FLASH_BANK_2;
    } else {
        // Invalid address
        return false;
    }

    if (HAL_FLASH_Unlock() != HAL_OK) {
        return false;
    }

    erase.TypeErase = FLASH_TYPEERASE_MASSERASE;
    erase.Banks = bank;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 2.7V to 3.6V

    if (HAL_FLASHEx_Erase(&erase, &page_error) != HAL_OK) {
        // Erase failed, lock flash and return false
        HAL_FLASH_Lock();
        return false;
    }

    HAL_FLASH_Lock();
    return true;
}

void FirmwareUploader::target_handle_done() {
    if (target_state_ == TargetUpdateState::RECEIVING_DATA) {
        target_state_ = TargetUpdateState::FINALIZING;
        HAL_FLASH_Lock();
        target_send_done_with_crc(target_running_crc_);
    }
}

bool FirmwareUploader::target_write_flash_block(const uint8_t* data, uint16_t len) {
    // Implement flash write procedure here, using data and len
    return true;
}

//--------------------------------------------------
// TARGET CAN TRANSMIT HELPERS
//--------------------------------------------------

void FirmwareUploader::target_send_setup_ack() {
    UpdateControl msg{};
    msg.target_board = current_board_;
    msg.setup_ack = 1;

    can_.write(&msg);
}

void FirmwareUploader::target_send_ready_for_data() {
    if (target_state_ == TargetUpdateState::READY_FOR_DATA) {
        UpdateControl msg{};
        msg.target_board = current_board_;
        msg.ready_for_data = 1;

        can_.write(&msg);
        target_state_ = TargetUpdateState::RECEIVING_DATA;
    }
}

void FirmwareUploader::target_send_done_with_crc(uint16_t crc) {
    UpdateControl msg{};
    msg.target_board = current_board_;
    msg.done = 1;
    msg.final_crc = crc;

    can_.write(&msg);
}
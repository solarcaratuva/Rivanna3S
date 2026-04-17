#include "FirmwareUploader.h"
#include "log.h"        // log_info / log_error — remove if not in your project
#include "FreeRTOS.h"
#include "task.h"
#include "Rivanna3SCanStructs.h"

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

    if (board_id < 0 || board_id > 9)
        return -1;

    //Create update control msg to send to target over CAN
    UpdateControl update_msg{};
    update_msg.target_board = board_id;
    update_msg.setup = 1; // signal sender is ready to send data
    can_.write(&update_msg);

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

uint16_t FirmwareUploader::crc16_hqx(const uint8_t* data, uint16_t len) {
    uint16_t crc = FW_CRC16_INIT;
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

void FirmwareUploader::target_receive_update_control(const SerializedCanMessage &msg) {
    UpdateControl control{};
    control.deserialize(&msg);

    int target_board = control.target_board;
    if (target_board == current_board_){
        if (control.setup) {
            target_send_setup_ack();
            target_begin_mass_erase();
        }
        if (control.done) {
            target_handle_done();
        }
    }
    
}
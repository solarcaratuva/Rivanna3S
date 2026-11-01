#include "FirmwareUploader.h"
#include "log.h" // log_info / log_error — remove if not in your project
#include "FreeRTOS.h"
#include "task.h"
#include <cstring>
#include <cstdio>
#include "Rivanna3SCanStructs.h"
#include "stm32_hal.h"

// ---------------------------------------------------------------------------
// Construction & lifecycle
// ---------------------------------------------------------------------------

FirmwareUploader* FirmwareUploader::s_instance_ = nullptr;

FirmwareUploader::FirmwareUploader(UART &uart, CanInterface &can, uint32_t current_board, uint32_t queue_depth)
    : uart_(uart),
      can_(can),
      queue_(queue_depth, sizeof(UploadBlock)),
      current_board_(current_board)
{
}

void FirmwareUploader::start(bool uart_mock)
{
    if (s_instance_) return; // The callback/Thread API supports one uploader.
    s_instance_ = this;
    uart_mock_ = uart_mock;
    consumer_thread_.start([]{ s_instance_->consumer_task(s_instance_); });
    UART_listener_thread_.start([]{ s_instance_->uart_listener_task(s_instance_); });
    if (!uart_mock_) {
        can_.register_callback(UpdateControl::get_message_ID(), [](const SerializedCanMessage &msg){ s_instance_->receive_update_control(msg); });
        can_.register_callback(UpdateData::get_message_ID(), [](const SerializedCanMessage &msg){ s_instance_->receive_update_data(msg); });
    }
}

// #129: keep the block/CRC/REQ/ACK exchange; size and image CRC make completion
// explicit. FW-prefixed response lines can coexist with ordinary log lines.
void FirmwareUploader::handle_uart_mock()
{
    char command[64];
    if (read_line(command, sizeof(command), 100) < 0) return;
    if (std::strncmp(command, "UPLOAD ", 7) != 0) return;
    unsigned board, expected_crc;
    unsigned long image_size;
    char extra;
    if (std::sscanf(command, "UPLOAD %u %lu %x %c", &board, &image_size, &expected_crc, &extra) != 3 ||
        board > 7 || !image_size || image_size > FW_MAX_FIRMWARE_SIZE || expected_crc > 0xffff) {
        uart_.write((uint8_t*)"FW FAIL\n", 8);
        return;
    }

    uint32_t offset = 0;
    unsigned retries = 0;
    bool ok = true;
    UploadBlock block{};
    while (offset < image_size) {
        uint8_t packet[FW_BLOCK_SIZE + 2];
        if (uart_.write((uint8_t*)"FW REQ\n", 7) != 0 || !read_exact(packet, sizeof(packet), 1000)) {
            ok = false;
            break; // A timeout is a failed transfer, never end-of-file.
        }
        const uint16_t received_crc = (uint16_t(packet[FW_BLOCK_SIZE]) << 8) | packet[FW_BLOCK_SIZE + 1];
        if (crc16_hqx(packet, FW_BLOCK_SIZE) != received_crc) {
            if (++retries > 3 || uart_.write((uint8_t*)"FW NACK\n", 8) != 0) {
                ok = false;
                break;
            }
            continue;
        }
        retries = 0;
        std::memcpy(block.data, packet, FW_BLOCK_SIZE);
        block.length = (image_size - offset < FW_BLOCK_SIZE) ? image_size - offset : FW_BLOCK_SIZE;
        if (queue_.append_to_back(&block, 1000) != 0) {
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
    if (queue_.append_to_back(&block, 1000) != 0 || result_queue_.get(&result, 2000) != 0) {
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
        while (uint32_t(Clock::get_current_time() - started) < 1000 && uart_.read(&byte, 1, 100) == 0) {}
    }
}

// ---------------------------------------------------------------------------
// Upload state machine
// ---------------------------------------------------------------------------

bool FirmwareUploader::handle_upload()
{
    uint8_t packet[FW_BLOCK_SIZE + 2]; // block + 2-byte CRC
    uint32_t offset = 0;
    host_running_crc_ = FW_CRC16_INIT; 


    while (offset < FW_MAX_FIRMWARE_SIZE)
    {
        uart_.write((uint8_t *)"REQ", 3);
    
        // Receive block + CRC; timeout means the sender has finished.
        if (!read_exact(packet, sizeof(packet), 1000))
        {
            uart_.write((uint8_t *)"DONE", 4);
            host_board_state_ = HostBoardState::DONE;

            while(host_board_state_ != HostBoardState::SUCCESS && host_board_state_ != HostBoardState::FAILED){
                // Wait for target to respond with final status
                clock_.sleep_for(1);
            }
            if(host_board_state_ == HostBoardState::SUCCESS){
                uart_.write((uint8_t *)"SUCCESS\n", 8);
            }
            else{
                uart_.write((uint8_t *)"FAILED\n", 7);
            }
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
        UploadBlock block{};
        std::memcpy(block.data, packet, FW_BLOCK_SIZE);
        block.length = FW_BLOCK_SIZE;
        int result = queue_.append_to_back(&block, portMAX_DELAY);
        if (result != 0)
        {
            if (result == -2)
                uart_.write((uint8_t *)"FAIL", 4);
            else if (result == -1)
                uart_.write((uint8_t *)"INIT\n", 5);

            uart_.write((uint8_t *)"NACK", 4);
            continue;
        }

        host_running_crc_ = crc16_hqx(packet, FW_BLOCK_SIZE, host_running_crc_);
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
    UploadBlock block{};
    if (self->uart_mock_) {
        UploadResult result{0, FW_CRC16_INIT};
        while (true) {
            if (self->queue_.get(&block, 1000) != 0) { Clock::sleep_for(1); continue; }
            if (block.length == 0) {
                self->result_queue_.append_to_back(&result, 1000);
                result = {0, FW_CRC16_INIT};
            } else {
                result.crc = crc16_hqx(block.data, block.length, result.crc);
                result.bytes += block.length;
            }
        }
    }
    uint32_t flash_addr = self->flash_base_addr_;
    bool targetstate_finished = false; // This should be set externally when the target state is finished
    if(self->is_host_){
        //pop data from queue and send over CAN
        while(self->host_board_state_ != HostBoardState::SENDING_DATA){
            // Wait for target to be ready for CAN data messages
            self->clock_.sleep_for(1);
        }
        while (true){
            if (self->queue_.get(&block, FW_BLOCK_SIZE) == 0)
            {
                // Send block over CAN to target board
                SerializedCanMessage data_msg{};
                data_msg.id = UpdateData::get_message_ID();
                data_msg.len = FW_BLOCK_SIZE;
                std::memcpy(data_msg.data, block.data, FW_BLOCK_SIZE);
                self->can_.write(&data_msg);

            }
            if(self->queue_.size() == 0 && self->host_board_state_ == HostBoardState::DONE){
                // send update control message to signal target we're done sending data
                UpdateControl control_msg{};
                control_msg.done = 1;
                self->can_.write(&control_msg);

                break;
            }
        }
         

    }
    else if(self->target_state_ == TargetUpdateState::RECEIVING_DATA){
    
        while (true)
        {
            if (self->queue_.get(&block, FW_BLOCK_SIZE) == 0)
            {
                // Write block to flash
                if (!self->write_flash(flash_addr, block.data, FW_BLOCK_SIZE))
                {
                    log_fault("Flash write failed at 0x%08lX", flash_addr);
                }
                else
                {
                    log_info("Block written to flash @ 0x%08lX", flash_addr);
                }
                flash_addr += FW_BLOCK_SIZE;
            }

        // Swap bank if target is in finalizing state and queue is empty
        if (self->target_state_ == TargetUpdateState::FINALIZING && self->queue_.size() == 0)
        {
            if (self->flash_base_addr_ == 0x08000000UL)
                self->set_flash_bank(2);
            else
                self->set_flash_bank(1);
            log_info("Switched flash bank. New base: 0x%08lX", self->flash_base_addr_);
            flash_addr = self->flash_base_addr_;
            self->target_state_ = TargetUpdateState::IDLE; // Reset flag
        }

            self->clock_.sleep_for(2);
        }
    }
}

void FirmwareUploader::uart_listener_task(void* arg) {
    FirmwareUploader* self = static_cast<FirmwareUploader*>(arg);
    char cmd_buf[32];

    while (true) {
        if (self->uart_mock_) { self->handle_uart_mock(); continue; }
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
            self->can_.write(&update_msg);

            while(self->host_board_state_ != HostBoardState::SETUP){
                // Wait for target to acknowledge setup and be ready for data
                self->clock_.sleep_for(1);
            }

            self->handle_upload(); 

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
            FLASH_TYPEPROGRAM_DOUBLEWORD,
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
        if(control.setup_ack){
            host_board_state_ = HostBoardState::SETUP;
        }
        if(control.ready_for_data){
            // Target is ready for data, we can start sending blocks from the queue
            host_board_state_ = HostBoardState::SENDING_DATA;

        }
        if(control.done){
            host_board_state_ = HostBoardState::DONE;
            if(control.final_crc == host_running_crc_){
                host_board_state_ = HostBoardState::SUCCESS;
            }
            else{
                host_board_state_ = HostBoardState::FAILED;
            }

        }

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
        if (control.done)
        {
            target_handle_done();
        }
    }
}

void FirmwareUploader::receive_update_data(const SerializedCanMessage &msg) {
    // No need to deserialize into a struct since it's just raw data blocks. Just forward to flash write.
    if (target_state_ == TargetUpdateState::RECEIVING_DATA && msg.len == FW_BLOCK_SIZE) {
        // Update the running CRC with the new block of data
        target_running_crc_ = crc16_hqx(msg.data, msg.len, target_running_crc_);

        UploadBlock block{};
        std::memcpy(block.data, msg.data, FW_BLOCK_SIZE);
        block.length = FW_BLOCK_SIZE;
        queue_.append_to_back(&block, portMAX_DELAY);
    }
}

bool FirmwareUploader::mass_erase() {
    FLASH_EraseInitTypeDef erase;
    uint32_t page_error = 0;
    uint32_t bank;

    #if defined(STM32H743xx)

    if ((flash_base_addr_ >= FLASH_BANK1_BASE) && (flash_base_addr_ < FLASH_BANK2_BASE)) {
        bank = FLASH_BANK_1;
    } else if ((flash_base_addr_ >= FLASH_BANK2_BASE) && (flash_base_addr_ < FLASH_END)) {
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
    erase.VoltageRange = VOLTAGE_RANGE_3; // 2.7V to 3.6V
    

    if (HAL_FLASHEx_Erase(&erase, &page_error) != HAL_OK) {
        // Erase failed, lock flash and return false
        HAL_FLASH_Lock();
        return false;
    }

    HAL_FLASH_Lock();
    return true;


    #elif defined(STM32G474xx)
    if ((flash_base_addr_ >= 0x08000000UL) && (flash_base_addr_ < 0x08040000UL)) {
        bank = FLASH_BANK_1;
    } else if ((flash_base_addr_ >= 0x08040000UL) && (flash_base_addr_ < 0x08080000UL)) {
        bank = FLASH_BANK_2;
    } else {
        // Invalid address
        return false;
    }

    #if defined (FLASH_OPTR_DBANK)
    if (READ_BIT(FLASH->OPTR, FLASH_OPTR_DBANK) != 0U)
    #endif
    {
        /* Check the parameters */
        assert_param(IS_FLASH_BANK(bank));

        /* Set the Mass Erase Bit for the bank 1 if requested */
        if ((bank & FLASH_BANK_1) != 0U)
        {
        SET_BIT(FLASH->CR, FLASH_CR_MER1);
        }

    #if defined (FLASH_OPTR_DBANK)
        /* Set the Mass Erase Bit for the bank 2 if requested */
        if ((bank & FLASH_BANK_2) != 0U)
        {
        SET_BIT(FLASH->CR, FLASH_CR_MER2);
        }
    #endif
    }
    #if defined (FLASH_OPTR_DBANK)
    else
    {
        SET_BIT(FLASH->CR, (FLASH_CR_MER1 | FLASH_CR_MER2));
    }
    #endif

    /* Proceed to erase all sectors */
    SET_BIT(FLASH->CR, FLASH_CR_STRT);
    return true;
    #endif

}

void FirmwareUploader::target_handle_done() {
    if (target_state_ == TargetUpdateState::RECEIVING_DATA) {
        target_state_ = TargetUpdateState::FINALIZING;
        HAL_FLASH_Lock();
        target_send_done_with_crc(target_running_crc_);
    }
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
#include "SD.h"
#include "../../Common/Drivers/Inc/SPI.h"
#include "user_diskio.h"
#include "log.h"
#include "task.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

#define MESSAGE_QUEUE_BUFFER_LENGTH 16

SdCard* SdCard::_SdCard = nullptr;

SdCard::SdCard(SPI* spi_peripheral) : _mq(MESSAGE_QUEUE_BUFFER_LENGTH) {
    _spi = spi_peripheral;
    _ready = false;
    _SdCard = this;
    _retUSER = FATFS_LinkDriver(&USER_Driver, _USERPath);
    if (_retUSER != 0) {
        log_warn("***SD: driver link failed***");
        return;
    }

    if (f_mount(&_USERFatFS, (TCHAR const*)_USERPath, 1) != FR_OK) {
        log_warn("***SD: mount failed***");
        return;
    }

    const char* log_file_name = _getFileName();
    if (f_open(&_USERFile, log_file_name, FA_OPEN_APPEND | FA_WRITE | FA_OPEN_ALWAYS) != FR_OK) {
        log_warn("***SD: failed to open log file***");
        return;
    }

    _ready = true;
    _flush_thread.start(continous_flush);
}

char* _getFileName() {
    static char file_name[32];
    DIR dir;
    FILINFO file_info;
    unsigned long max_index = 0;
    file_name[0] = '\0';
    unsigned long max_idx = 0; 
        
    if (f_opendir(&dir, "/") == FR_OK) {
        while (1){
            // Empty 
            FRESULT res = f_readdir(&dir,&file_info);
            if (res != FR_OK || file_info.fname[0] == '\0') break; // Check File is valid
            char* current_name = file_info.fname;
            if (strncmp(current_name, "telem", 5) != 0) continue; // First chars should 5 
            size_t name_len = strlen(current_name);
            if (name_len <= 9 || strcmp(current_name + name_len - 4, ".log") != 0) continue; // Doesnt end in ".log"
            char* end_ptr = nullptr;
            unsigned long current_index = strtoul(current_name + 5, &end_ptr, 10); // converting to number
            // Maybe check vlaiidy of 
            max_idx = current_index + 1;
        }
        f_closedir(&dir);
    }
    snprintf(file_name, sizeof(file_name), "telem%lu.log", max_index);
    return file_name;
}

int SdCard::write(uint8_t* buffer, uint16_t len) {
    if (!_ready) return 0;
    int send_status = _mq.send(buffer, len);
    return (send_status == 0) ? len : 0;
}

void SdCard::write_and_flush_loop() {
    if (!_ready) return;
    uint8_t buffer[MESSAGE_QUEUE_BUFFER_LENGTH];
    int bytes_received = _mq.receive(buffer, MESSAGE_QUEUE_BUFFER_LENGTH);
    if (bytes_received <= 0) return;

    UINT bytes_written = 0;
    f_write(&_USERFile, buffer, static_cast<UINT>(bytes_received), &bytes_written);
    f_sync(&_USERFile); // maybe only sync every n-miliseconds
    vTaskDelay(pdMS_TO_TICKS(SD_FLUSH_INTERVAL_MS));
}

void SdCard::continous_flush() {
    if (_SdCard) _SdCard->write_and_flush_loop();
}

void SdCard::attach_to_log() {
    if (!_ready) return;
    log_set_output_callback([](const uint8_t* data, uint16_t len) {
        if (_SdCard) _SdCard->write(const_cast<uint8_t*>(data), len);
    });
}

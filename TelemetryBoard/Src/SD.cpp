#include "SD.h"
#include "../../Common/Drivers/Inc/SPI.h"
#include "user_diskio.h"
#include "log.h"
#include "task.h"
#include "string.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define SD_QUEUE_BUFFER_BYTES 2048
#define SD_MESSAGE_MAX_BYTES 256

SdCard* SdCard::_SdCard = nullptr;

SdCard::SdCard(SPI* spi_peripheral) : _mq(SD_QUEUE_BUFFER_BYTES) {
    _spi = spi_peripheral;
    _ready = false;
    _messages_since_sync = 0;
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

char* SdCard::_getFileName() {
    static char file_name[32];
    DIR dir;
    FILINFO file_info;
    unsigned long max_index = 0;
    file_name[0] = '\0';

    if (f_opendir(&dir, "/") == FR_OK) {
        while (1){
            // Empty 
            FRESULT res = f_readdir(&dir,&file_info);
            if (res != FR_OK || file_info.fname[0] == '\0') break; // Check File is valid
            char* current_name = file_info.fname;
            if (strlen(current_name) < 9) continue;

            // Check for existence of TELEM{N}.LOG
            bool has_telem_prefix =
                (tolower(current_name[0]) == 't') &&
                (tolower(current_name[1]) == 'e') &&
                (tolower(current_name[2]) == 'l') &&
                (tolower(current_name[3]) == 'e') &&
                (tolower(current_name[4]) == 'm');
            if (!has_telem_prefix) continue;
            size_t name_len = strlen(current_name);
            
            if (name_len <= 9) continue;

            const char* ext = current_name + name_len - 4;
            bool has_log_ext =
                (tolower(ext[0]) == '.') &&
                (tolower(ext[1]) == 'l') &&
                (tolower(ext[2]) == 'o') &&
                (tolower(ext[3]) == 'g');
            if (!has_log_ext) continue; // Doesn't end in .log (case-insensitive)

            char* end_ptr = nullptr;
            unsigned long current_index = strtoul(current_name + 5, &end_ptr, 10); // converting to number
            if (end_ptr == current_name + name_len - 4 && current_index >= max_index) {
                //bounds check
                max_index = current_index + 1;
            }
        }
        f_closedir(&dir);
    }
    snprintf(file_name, sizeof(file_name), "telem%lu.log", max_index);
    return file_name;
}

int SdCard::write(uint8_t* buffer, uint16_t len) {
    if (!_ready || len == 0 || len > SD_MESSAGE_MAX_BYTES) return 0;
    int send_status = _mq.send(buffer, len);
    return (send_status == 0) ? len : 0;
}

void SdCard::write_and_flush_loop() {
    if (!_ready) return;
    uint8_t buffer[SD_MESSAGE_MAX_BYTES];
    int bytes_received = _mq.receive(buffer, SD_MESSAGE_MAX_BYTES, SD_FLUSH_INTERVAL_MS);

    if (bytes_received <= 0) {
        if (_messages_since_sync > 0) {
            f_sync(&_USERFile);
            _messages_since_sync = 0;
        }
        return;
    }

    UINT bytes_written = 0;
    f_write(&_USERFile, buffer, static_cast<UINT>(bytes_received), &bytes_written);
    _messages_since_sync++;

    if (_messages_since_sync >= SD_SYNC_MESSAGE_COUNT) {
        f_sync(&_USERFile);
        _messages_since_sync = 0;
    }
}

void SdCard::continous_flush() {
    if (_SdCard) _SdCard->write_and_flush_loop();
}

void SdCard::attach_to_log() {
    if (!_ready) {
        log_warn("****************SD: attach_to_log skipped, SD not ready******************");
        return;
    }
    log_set_output_callback([](const uint8_t* data, uint16_t len) {
        if (_SdCard) _SdCard->write(const_cast<uint8_t*>(data), len);
    });
}

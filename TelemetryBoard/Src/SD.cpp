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

char* ODOMETRY_LOG_FILE = "odometry.log";

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
        log_warn("SD: mount failed");
        return;
    }

    const char* log_file_name = _getFileName();
    if (f_open(&_USERFile, log_file_name, FA_OPEN_APPEND | FA_WRITE | FA_OPEN_ALWAYS) != FR_OK) {
        log_warn("SD: failed to open log file");
        return;
    }

    // odometry file
    if (f_open(&_OdometryFile, ODOMETRY_LOG_FILE, FA_READ | FA_WRITE | FA_OPEN_ALWAYS) != FR_OK) {
        log_warn("SD: failed to open odometry file");
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

// odometry stuff
bool SdCard::write_odometry_data(uint32_t value) {
    if (! _ready) {
        return false;
    }

    if (f_lseek(&_OdometryFile, f_size(&_OdometryFile)) != FR_OK) {
        return false;
    }

    uint8_t buffer[4] = {
        static_cast<uint8_t>((value >> 24) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
    UINT bytes_written = 0;
    f_write(&_OdometryFile, buffer, 4, &bytes_written);
    f_sync(&_OdometryFile);
    return bytes_written == 4;
}

uint32_t SdCard::read_odometry_data_file() {
    if (! _ready) {
        return 0;
    }

    const FSIZE_t file_size = f_size(&_OdometryFile);

    // Move the file pointer to the end of the file
    if (f_lseek(&_OdometryFile, file_size) != FR_OK) {
        return 0;
    }

    // If the file is empty, return 0
    if (file_size < 4) {
        return 0;
    }

    // Move the file pointer back by 4 bytes to read the last entry
    if (f_lseek(&_OdometryFile, file_size - 4) != FR_OK) {
        return 0;
    }

    uint8_t buffer[4];
    UINT bytes_read = 0;
    FRESULT res = f_read(&_OdometryFile, buffer, 4, &bytes_read);

    // Leave the handle positioned at end-of-file so the next write appends.
    f_lseek(&_OdometryFile, file_size);

    if (res != FR_OK || bytes_read != 4) {
        log_warn("SD: failed to read latest odometry data");
        return 0;
    }

    uint32_t value = (static_cast<uint32_t>(buffer[0]) << 24) |
                     (static_cast<uint32_t>(buffer[1]) << 16) |
                     (static_cast<uint32_t>(buffer[2]) << 8) |
                     static_cast<uint32_t>(buffer[3]);
    return value;
}
#include "SD.h"
#include "../../Common/Drivers/Inc/SPI.h"
#include "user_diskio.h"
#include "log.h"
#include "task.h"
#define LOGFILE "telem.log"

SdCard* SdCard::_SdCard = nullptr;

SdCard::SdCard(SPI* spi_peripheral) {
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

    if (f_open(&_USERFile, LOGFILE, FA_OPEN_APPEND | FA_WRITE | FA_OPEN_ALWAYS) != FR_OK) {
        log_warn("***SD: failed to open " LOGFILE "***");
        return;
    }

    _ready = true;
    _flush_thread.start(continous_flush); // no () — pass pointer, not call result
}

int SdCard::write(uint8_t* buffer, uint16_t len) {
    if (!_ready) return 0;
    UINT byteswritten = 0;
    f_write(&_USERFile, buffer, len, &byteswritten);
    return byteswritten;
}

void SdCard::write_and_flush_loop() {
    if (!_ready) return;
    f_sync(&_USERFile);
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

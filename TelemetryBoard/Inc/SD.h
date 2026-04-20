#include "fatfs.h"
#include "thread.h"

class SPI;

#define SD_FLUSH_INTERVAL_MS 1000

class SdCard {
    private:
        SPI* _spi;
        uint8_t _retUSER;
        char _USERPath[4];
        FATFS _USERFatFS;
        FIL _USERFile;
        Thread _flush_thread;
        bool _ready;
        static SdCard* _SdCard;
        static void continous_flush();

    public:
        SdCard(SPI* spi_peripheral);
        int write(uint8_t* buffer, uint16_t len);
        void write_and_flush_loop();
        void attach_to_log();
};

#include "log.h"
#include "CanInterface.h"
#include "pindef.h"
#include "SPI.h"
#include "fatfs.h"


#define LOG_LEVEL DEBUG_LVL
const bool PIN_ON = true;
const bool PIN_OFF = false;

CanInterface main_can = CanInterface(CAN_TX, CAN_RX, CAN_STBY, 250000, CanNetwork::Main);


 void handle_all_messages(const SerializedCanMessage &msg) {
    // 1. send message over radio
    // 2. send message over LTE
}


void app_main() {
    log_configure(LOG_LEVEL, LOG_TX, LOG_RX, 250000);
    log_info("Telemetry Board starting up...");


    // Temporary Write Test
    MX_FATFS_Init();
    uint8_t sd_card_write_result = FATFS_SD_TestWrite();
    switch (sd_card_write_result){
    case 0:
        log_debug("Succesfully Wrote!");
        break;
    case 1:
        log_debug("Failed to Mount!");
        break;
    case 2:
        log_debug("Failed to Open/Create!");
        break;
    case 3:
        log_debug("Failed to Write bytes!");
        break;
    }
    

    main_can.register_always_callback(handle_all_messages);

    log_info("Telemetry Board initialized");
}

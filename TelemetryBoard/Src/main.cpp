#include "log.h"
#include "CanInterface.h"
#include "pindef.h"
#include "../../Common/Drivers/Inc/SPI.h"
#include "SD.h"
#include "task.h"

#define LOG_LEVEL DEBUG_LVL

CanInterface main_can = CanInterface(CAN_TX, CAN_RX, CAN_STBY, 250000, CanNetwork::Main);

void handle_all_messages(const SerializedCanMessage &msg) {
    // 1. send message over radio
    // 2. send message over LTE
}

void app_main() {
    log_configure(LOG_LEVEL, LOG_TX, LOG_RX, 921600);
    log_info("Telemetry Board starting up...");
    
    static SPI sd_spi(SPI2_MOSI, SPI2_MISO, SPI2_SCK, 400000);
    static SdCard sd_card(&sd_spi);
    sd_card.attach_to_log();
    
    main_can.register_always_callback(handle_all_messages);

    Clock::sleep_for(0xFFFFFFFF);
}

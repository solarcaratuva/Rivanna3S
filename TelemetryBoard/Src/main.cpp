#include "log.h"
#include "CanInterface.h"
#include "pindef.h"
#include "../../Common/Drivers/Inc/SPI.h"
#include "SD.h"
#include "task.h"
#include "heartbeat.h"
#include <cstdio>

#define LOG_LEVEL DEBUG_LVL

CanInterface main_can = CanInterface(CAN_TX, CAN_RX, CAN_STBY, 250000, CanNetwork::Main);

static SdCard* global_sd_card = nullptr;

void log_can_frame(const SerializedCanMessage& msg) {
    char line[96];
    
    unsigned long time_ms = (unsigned long)xTaskGetTickCount();

    int len = snprintf(
        line,
        sizeof(line),
        "%lu,0x%03X,%u,%02X %02X %02X %02X %02X %02X %02X %02X\n",
        time_ms,
        (unsigned int)msg.id,
        (unsigned int)msg.len,
        msg.data[0], msg.data[1], msg.data[2], msg.data[3],
        msg.data[4], msg.data[5], msg.data[6], msg.data[7]
    );

    if (len > 0) {
        global_sd_card->write((uint8_t*)line, (uint16_t)len);
    }
}

void handle_all_messages(const SerializedCanMessage &msg) {
    // 1. send message over radio
    // 2. send message over LTE
    log_can_frame(msg);
}

void log_missed_heartbeat() {
    log_fault("missed heartbeat callback func");
}

void app_main() {
    HeartbeatSafetySystem::setup(&main_can, log_missed_heartbeat, Node::TelemetryBoard);

    log_configure(LOG_LEVEL, LOG_TX, LOG_RX, 921600);
    log_info("Telemetry Board starting up...");
    
    static SPI sd_spi(SPI2_MOSI, SPI2_MISO, SPI2_SCK, 400000);
    static SdCard sd_card(&sd_spi);
    global_sd_card = &sd_card;
    // sd_card.attach_to_log();
    
    main_can.register_always_callback(handle_all_messages);

    Clock::sleep_for(0xFFFFFFFF);
}

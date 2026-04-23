#include "log.h"
#include "CanInterface.h"
#include "pindef.h"
#include "../../Common/Drivers/Inc/SPI.h"
#include "SD.h"
#include "task.h"
#include "heartbeat.h"
#include <cstdio>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "UART.h"
#include "AnalogIn.h"
#include "Timeout.h"
#include "Clock.h"
#include "thread.h"
#include "Timeout.h"
#include "lock.h"
#include "UartCobs.h"


#define LOG_LEVEL INFO_LVL

CanInterface main_can = CanInterface(CAN_TX, CAN_RX, CAN_STBY, 250000, CanNetwork::Main);
static SdCard* global_sd_card = nullptr;

static UART radio_uart(RADIO_TX, RADIO_RX, 9600);
static UartCobs radio(&radio_uart);

/**
 * @brief Forward a received CAN message over the radio link via COBS-framed UART
 *
 * Packet format:
 *   [0..1]  Message ID  (2 bytes, big-endian)
 *   [2..N]  Message data (1-8 bytes, copied from the CAN frame)
 *
 * Registered as the CanInterface "always" callback so every message
 * received on the bus is transparently forwarded to the radio.
 *
 * @param msg The raw CAN frame received by the CanInterface
 */
void radio_send(const SerializedCanMessage &msg)
{
    uint8_t buf[10];

    buf[0] = static_cast<uint8_t>(msg.id >> 8);
    buf[1] = static_cast<uint8_t>(msg.id & 0xFF);

    uint8_t data_len = (msg.len <= 8) ? msg.len : 8;
    for (uint8_t i = 0; i < data_len; i++) {
        buf[2 + i] = msg.data[i];
    }

    radio.write(buf, 2 + data_len);
}

void handle_all_messages(const SerializedCanMessage &msg) {
    // 1. send message over radio
    radio_send(msg);
    // 2. send message over LTE

    // SD card is handled automatically through the log system
}

void missed_heartbeat_callback() {
    log_fault("missed heartbeat callback func xxxx");
}

void app_main() {
    log_configure(LOG_LEVEL, LOG_TX, LOG_RX, 921600);
    log_info("Telemetry Board starting up...");

    // HeartbeatSafetySystem::setup(&main_can, missed_heartbeat_callback, Node::TelemetryBoard);
    
    static SPI sd_spi(SPI2_MOSI, SPI2_MISO, SPI2_SCK, 400000);
    static SdCard sd_card(&sd_spi);
    global_sd_card = &sd_card;
    sd_card.attach_to_log();
    sd_card.attach_to_log();
    
    main_can.register_always_callback(handle_all_messages);
    Clock::sleep_forever();
}

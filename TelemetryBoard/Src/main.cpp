#include "log.h"
#include "CanInterface.h"
#include "pindef.h"
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


#define LOG_LEVEL DEBUG_LVL

CanInterface main_can = CanInterface(CAN_TX, CAN_RX, CAN_STBY, 250000, CanNetwork::Main);

static UART radio_uart(RADIO_TX, RADIO_RX, 921600);
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
  // 2. send message over LTE
}

void radio_send_test()
{
    SerializedCanMessage test_msg = {};
    test_msg.id = 0x123;
    test_msg.len = 4;
    test_msg.data[0] = 0xDE;
    test_msg.data[1] = 0xAD;
    test_msg.data[2] = 0xBE;
    test_msg.data[3] = 0xEF;

    radio_send(test_msg);

    log_info("Sent test SerializedCanMessage over radio");
}

void app_main()
{
  log_configure(LOG_LEVEL, LOG_TX, LOG_RX, 921600);
  // log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
  /* USER CODE END Init */

  log_info("Telemetry Board starting up...");

  // main_can.register_always_callback(handle_all_messages);
  radio_send_test();

  log_info("Telemetry Board initialized");
  DigitalOut LED1(PB_0);

  while (1)
  {
    // log_debug("%s","HERE");
    HAL_Delay(1000);
    LED1.write(!LED1.read());
  }
}


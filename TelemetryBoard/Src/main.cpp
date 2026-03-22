#include "log.h"
#include "CanInterface.h"
#include "pindef.h"
/* USER CODE END Includes */

#define LOG_LEVEL DEBUG_LVL

struct TelemetryCanFrame
{
    uint16_t id;
    uint8_t len; // data length code
    uint8_t data[8]; // Raw payload bytes (pad unused bytes with 0)
    uint32_t timestamp_ms;  // when this frame was received (ms since boot)
};

// Queue of TelemetryCanFrame items waiting to be sent out over LTE.
static QueueHandle_t telemetrySendQueue = nullptr;

static DigitalOut lteDtrPin(LTE_DTR);
static UART lteUart(LTE_TX, LTE_RX, 115200);
static CanInterface mainCan(CAN_TX, CAN_RX, CAN_STBY, 250000, CanNetwork::Main);

static Thread lte_thread;

// Optional stats for debugging
static volatile uint32_t framesQueued = 0;
static volatile uint32_t framesDropped = 0;


static inline uint32_t nowMs() { return HAL_GetTick(); }

static void enqueueCanMsg(const SerializedCanMessage &msg)
{
  TelemetryCanFrame frame{};
  frame.id = msg.id;
  frame.len = msg.len;
  frame.timestamp_ms = nowMs();
  
  // Copy data; pad remainder with 0 for cleanliness
  for (int i = 0; i < 8; i++) {
    frame.data[i] = (i < frame.len) ? msg.data[i] : 0;
  }

  // Non-blocking push. If full, drop.
  if (xQueueSendToBack(telemetrySendQueue, &frame, 0) == pdTRUE) {
    framesQueued++;
  } else {
    framesDropped++;
  }

}

/*
  --------------------------------------------
  LTE SENDER THREAD (SLOW PATH)
  --------------------------------------------
  This thread is allowed to:
    - block waiting for queue items
    - block on UART writes

  Packet format (fixed length, easy to parse):
    START  : 0xA5
    ID     : uint16 (little-endian)
    LEN    : uint8  (0..8)
    DATA   : 8 bytes (always 8 sent)
    TIME   : uint32 ms (little-endian)
    END    : 0x5A

  Total = 1 + 2 + 1 + 8 + 4 + 1 = 17 bytes.
*/

// NOTE: Need to talk with Embedded about if there's an easier an easier way to do this.
static void handle_lte_transmission()
{
  const uint8_t START = 0xA5;
  const uint8_t END = 0x5A;

  TelemetryCanFrame frame{};

  while (true) {
      // Wait until a frame is available
      if (xQueueReceive(telemetrySendQueue, &frame, portMAX_DELAY) != pdTRUE) {
          continue;
      }

      uint8_t pkt[17];
      int idx = 0;

      pkt[idx++] = START;

      // ID (little endian)
      pkt[idx++] = (uint8_t)(frame.id & 0xFF);
      pkt[idx++] = (uint8_t)((frame.id >> 8) & 0xFF);

      // LEN (DLC bytes)
      pkt[idx++] = frame.len;

      // DATA (always 8 bytes in packet)
      for (int i = 0; i < 8; i++) {
          pkt[idx++] = frame.data[i];
      }

      // timestamp_ms (little endian)
      pkt[idx++] = (uint8_t)(frame.timestamp_ms & 0xFF);
      pkt[idx++] = (uint8_t)((frame.timestamp_ms >> 8) & 0xFF);
      pkt[idx++] = (uint8_t)((frame.timestamp_ms >> 16) & 0xFF);
      pkt[idx++] = (uint8_t)((frame.timestamp_ms >> 24) & 0xFF);

      pkt[idx++] = END;

      log_info("Sending pkt: id=0x%03X len=%u time=%lu data=%02X %02X %02X %02X %02X %02X %02X %02X",
         frame.id,
         frame.len,
         (unsigned long)frame.timestamp_ms,
         frame.data[0], frame.data[1], frame.data[2], frame.data[3],
         frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
      }

      // Send to LTE module
      // note: need to look into which mode for LTE to use. 
      // note: lteUart.write(pkt, idx) sends raw telemetry packets to the XBee over UART. A MicroPython script on the XBee is expected to read these bytes and publish them to AWS IoT over MQTT/TLS.
      lteUart.write(pkt, idx); NOTE: UNCOMMENT WHEN NOT TESTING
}


void app_main()
{
  // NOTE: pindef does not have Log TX/RX?
  log_configure(LOG_LEVEL, LOG_TX, LOG_RX, 921600);
  log_info("Telemetry Board starting up...");

  // Might need to do something with DRL?
  // like lteDtrPin.write(true);

  // Create queue (tune depth later)
  telemetrySendQueue = xQueueCreate(256, sizeof(TelemetryCanFrame));
  if (telemetrySendQueue == nullptr) {
      log_fault("Failed to create telemetrySendQueue (out of memory?)");
      return;
  }

  // Register callback for ALL CAN frames
  int rc = mainCan.register_always_callback(enqueueCanMsg);
  if (rc != 0) {
      log_fault("Failed to register CAN always callback: %d", rc);
      return;
  }

  // Start thread(s)
  lte_thread.start(handle_lte_transmission);
  // prob going to need an XBEE RF thread as well

  lteDtrPin.write(true);
  
  Clock clk;
  uint32_t lastLog = nowMs();

  while (1)
  {
    clk.sleep_since(500);
    uint32_t t = nowMs();
    if (t - lastLog >= 2000) {
      lastLog = t;
      log_info("Telemetry stats: queued=%lu dropped=%lu", (unsigned long)framesQueued, (unsigned long)framesDropped);
    }
  }
}
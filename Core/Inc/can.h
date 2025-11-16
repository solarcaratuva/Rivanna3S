#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "fdcan.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"
#ifdef __cplusplus
}
#endif

// -----------------------------------------------------------------------------
// Serialized CAN message (raw frame)
// -----------------------------------------------------------------------------
struct SerializedCanMessage {
    uint16_t id;       // 11-bit standard ID
    uint8_t  len;      // number of data bytes (0..8)
    uint8_t  data[8];  // payload
};

// -----------------------------------------------------------------------------
// Logical CAN message interface
// -----------------------------------------------------------------------------
struct CanMessage {
    virtual ~CanMessage() = default;

    // Convert logical struct -> raw CAN frame
    virtual SerializedCanMessage serialize() const = 0;

    // Convert raw CAN frame -> logical struct
    virtual void deserialize(const SerializedCanMessage& scm) = 0;

    // For debugging/logging
    virtual void log(int level) const = 0;

    // make sure to always initialize it with static uint16_t get_ID()
};

// -----------------------------------------------------------------------------
// Simple CAN driver wrapper
// -----------------------------------------------------------------------------

using Pin = uint32_t;

class CAN {
public:
    // Constructor: tx/rx kept for compatibility; baudrate is assumed to be set
    // in MX_FDCAN1_Init / FDCAN_Config.
    CAN(Pin tx, Pin rx, uint32_t baudrate);

    // Write a logical CanMessage
    // Returns 0 on success, negative on error.
    int write(CanMessage* msg);

    // Read a raw message (polling FIFO0).
    // Returns:
    //   1 if a message was read,
    //   0 if no message pending,
    //  <0 on error.
    int read(SerializedCanMessage* msg);

private:
    FDCAN_HandleTypeDef* m_hfdcan;
    SemaphoreHandle_t    m_txMutex;
    FDCAN_TxHeaderTypeDef m_txHeader;
    FDCAN_RxHeaderTypeDef m_rxHeader;

    uint32_t bytesToDlc(uint8_t len) const;
    uint8_t  dlcToBytes(uint32_t dlc) const;
};
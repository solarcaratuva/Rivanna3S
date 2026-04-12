#ifndef CAN_H
#define CAN_H

#include "stm32_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "lock.h"
#include "log.h"

/**
 * @brief Serialized CAN message structure representing a raw CAN frame
 * 
 * This structure contains the low-level representation of a CAN message
 * suitable for direct transmission over the CAN bus or received from it.
 */
struct SerializedCanMessage {
    uint16_t id;       ///< CAN identifier (11-bit standard ID, range: 0x000 - 0x7FF)
    uint8_t  len;      ///< Number of data bytes in the payload (0..64)
    uint8_t  data[64];  ///< Message payload data
};

/**
 * @brief Abstract interface for logical CAN messages
 * 
 * This interface defines the contract for higher-level CAN message types
 * that can be serialized to and deserialized from raw CAN frames. Derive
 * from this interface to create message-specific structures.
 * 
 * @note Derived classes must implement a static uint16_t get_message_ID() method
 *       to provide the CAN ID for the message type.
 */
struct CanMessage {
    virtual ~CanMessage() = default;

    /**
     * @brief Convert logical message structure to raw CAN frame
     * @param[out] message Pointer to SerializedCanMessage to populate
     */
    virtual void serialize(SerializedCanMessage *message) = 0;

    /**
     * @brief Convert raw CAN frame to logical message structure
     * @param[in] message Pointer to SerializedCanMessage to deserialize
     */
    virtual void deserialize(SerializedCanMessage *message) = 0;

    /**
     * @brief Log the message contents for debugging
     * @param level Logging level (e.g., LOG_INFO, LOG_WARN, LOG_ERROR)
     */
    virtual void log_msg(LogLevel level) const = 0;

    /**
     * @brief Get the message type ID of the instance. 
     * Using the static `MessageType::get_message_ID()` is preferred when the message type is know at compile time.
     */
    virtual uint16_t ID() const = 0;
};

/**
 * @brief CAN bus driver wrapper for STM32 FDCAN peripheral
 * 
 * This class provides a high-level interface to the STM32 FDCAN (Flexible Data-rate CAN)
 * peripheral operating in Classic CAN mode. It supports both raw frame transmission/reception
 * and higher-level message objects through the CanMessage interface.
 * 
 * Features:
 * - Thread-safe operation using FreeRTOS locks
 * - Standard CAN (11-bit identifiers)
 * - Classic CAN frame format (up to 8 bytes)
 * - Configurable baudrate
 * - Automatic pin mapping and peripheral initialization
 * 
 * @note This driver currently operates in Classic CAN mode only. CAN FD support
 *       requires modifications to support larger data lengths.
 * 
 * Example usage:
 * @code
 * CAN can(TX_PIN, RX_PIN, 500000);  // 500 kbps
 * 
 * SerializedCanMessage msg;
 * msg.id = 0x123;
 * msg.len = 4;
 * msg.data[0] = 0xAA;
 * msg.data[1] = 0xBB;
 * msg.data[2] = 0xCC;
 * msg.data[3] = 0xDD;
 * 
 * can.write(msg);
 * @endcode
 */
class CAN {
public:
    /**
     * @brief Construct and initialize the CAN peripheral
     * 
     * This constructor configures the FDCAN peripheral with the specified pins
     * and baudrate. It performs the following operations:
     * - Finds and claims a suitable FDCAN peripheral matching the pin configuration
     * - Configures GPIO pins with appropriate alternate functions
     * - Initializes the FDCAN peripheral in Classic CAN mode
     * - Starts the CAN peripheral for operation
     * 
     * @param tx Transmit pin (must be a valid FDCAN TX pin)
     * @param rx Receive pin (must be a valid FDCAN RX pin)
     * @param baudrate CAN bus baudrate in bits per second (common values: 125000, 250000, 500000, 1000000)
     * 
     * @warning If initialization fails (e.g., invalid pins or no available peripheral),
     *          the object will be marked as uninitialized and subsequent operations will fail.
     */
    CAN(Pin tx, Pin rx, uint32_t baudrate);

    /**
     * @brief Write a logical CAN message to the bus
     * 
     * Serializes a CanMessage object and transmits it on the CAN bus.
     * This is a convenience method that internally calls serialize() on the
     * message object before transmission.
     * 
     * @param[in] msg Pointer to the CanMessage object to transmit
     * @return int Status code:
     *         - 0: Success
     *         - 1: CAN peripheral not initialized
     *         - 2: HAL transmission error
     * 
     * @note This method is thread-safe
     */
    int write(CanMessage* msg);

    /**
     * @brief Write a raw CAN frame to the bus
     * 
     * Transmits a raw SerializedCanMessage directly to the CAN bus without
     * additional processing. The message is added to the transmit FIFO/queue.
     * 
     * @param[in] msg Pointer to the SerializedCanMessage to transmit
     * @return int Status code:
     *         - 0: Success
     *         - 1: CAN peripheral not initialized
     *         - 2: HAL transmission error (FIFO full or peripheral error)
     * 
     * @note This method is thread-safe
     */
    int write(const SerializedCanMessage *msg);

    /**
     * @brief Read a raw CAN message from the receive FIFO
     * 
     * Polls the FDCAN RX FIFO0 for incoming messages. This method will block
     * until a message is available.
     * 
     * @param[out] msg Pointer to SerializedCanMessage to populate with received data
     * @return int Status code:
     *         - 0: Message successfully received
     *         - 1: CAN peripheral not initialized
     *         - 2: HAL reception error
     * 
     * @note This method is thread-safe but BLOCKING. It will wait indefinitely
     *       until a message arrives.
     */
    int read(SerializedCanMessage* msg);

private:
    FDCAN_HandleTypeDef* hfdcan;        ///< Handle to the STM32 HAL FDCAN peripheral
    Lock                 instance_lock;  ///< Mutex for thread-safe access
    FDCAN_TxHeaderTypeDef txHeader;     ///< Transmit header (reused for efficiency)
    FDCAN_RxHeaderTypeDef rxHeader;     ///< Receive header (reused for efficiency)
    bool initialized;                    ///< Initialization status flag

    FDCAN_Peripheral* fdcan_periph;     ///< Pointer to the claimed FDCAN peripheral descriptor

    /**
     * @brief Convert byte count to FDCAN DLC (Data Length Code)
     * 
     * Converts a byte count (0-8) to the corresponding FDCAN DLC value
     * used in the hardware registers. Values greater than 8 are clamped.
     * 
     * @param len Number of bytes (0-8)
     * @return uint32_t FDCAN DLC constant (e.g., FDCAN_DLC_BYTES_4)
     */
    uint32_t bytesToDlc(uint8_t len) const;

    /**
     * @brief Convert FDCAN DLC (Data Length Code) to byte count
     * 
     * Converts an FDCAN DLC value from the hardware registers to the
     * actual number of data bytes it represents.
     * 
     * @param dlc FDCAN DLC constant from hardware register
     * @return uint8_t Number of bytes (0-8)
     */
    uint8_t  dlcToBytes(uint32_t dlc) const;

    /**
     * @brief Find and claim an FDCAN peripheral matching the pin configuration
     * 
     * Searches through available FDCAN peripherals to find one that supports
     * the specified TX and RX pins. If found and not already claimed, marks
     * the peripheral as claimed and returns it.
     * 
     * @param tx Desired transmit pin
     * @param rx Desired receive pin
     * @return FDCAN_Peripheral* Pointer to claimed peripheral, or nullptr if none available
     */
    FDCAN_Peripheral* findCANPin(Pin tx, Pin rx);
};

#endif // CAN_H

#ifndef CANINTERFACE_H
#define CANINTERFACE_H

#include <cstdint>

#include "can.h"        // CAN wrapper (CAN, CanMessage, SerializedCanMessage)
#include "thread.h"
#include "Clock.h"
#include "DigitalOut.h"

/**
 * @brief Callback function type for CAN message reception
 * 
 * User-defined callbacks receive a const reference to the received raw CAN frame.
 * The callback should process the message quickly to avoid blocking the receiver thread.
 */
using CanCallback = void (*)(const SerializedCanMessage &msg);

enum class CanNetwork {
    Main = 0,
    Motor = 1
};

static inline const char* CanNetworkToString(CanNetwork net) {
    switch (net) {
        case CanNetwork::Main: return "Main";
        case CanNetwork::Motor: return "Motor";
        default: return "Unknown";
    }
}

/**
 * @brief High-level CAN interface with automatic message dispatching
 * 
 * This class provides a callback-based interface for CAN communication. It manages:
 * - A background receiver thread that continuously polls for incoming messages
 * - Automatic dispatching of messages to registered ID-specific callbacks
 * - Optional "always" callback invoked for every received message
 * - Thread-safe message transmission
 * - Multiple independent CAN networks (Main, Motor, etc.) via the CanNetwork enum
 * 
 * The receiver thread runs continuously after construction and can be paused/resumed
 * as needed using stop_receiver_execution() and restart_receiver_execution().
 * 
 * Multiple instances of CanInterface can coexist simultaneously, each configured
 * for a different CAN network type. Each instance maintains its own receiver thread,
 * callback registrations, and message processing pipeline.
 * 
 * Example usage:
 * @code
 * void my_callback(const SerializedCanMessage& msg) {
 *     // Process message with ID 0x123
 * }
 * 
 * // Create separate instances for different CAN networks
 * CanInterface main_can(TX_PIN_1, RX_PIN_1, 500000, CanNetwork::Main);
 * CanInterface motor_can(TX_PIN_2, RX_PIN_2, 500000, CanNetwork::Motor);
 * 
 * main_can.register_callback(0x123, my_callback);
 * @endcode
 */
class CanInterface
{
public:
    /**
     * @brief Construct a new CanInterface object and start the receiver thread
     * 
     * This constructor initializes the underlying CAN peripheral with the given TX/RX pins
     * and baudrate, and immediately starts a background thread that continuously receives
     * CAN messages and dispatches them to registered callbacks.
     * 
     * The network parameter specifies which CAN network this interface belongs to (Main or Motor).
     * Multiple CanInterface instances can exist simultaneously, each serving a different network.
     * This allows independent message handling for separate CAN buses.
     * 
     * @param tx Transmit pin used by the CAN peripheral
     * @param rx Receive pin used by the CAN peripheral
     * @param standby Standby pin used by the CAN peripheral
     * @param baudrate CAN bus baudrate in bits per second (e.g., 500000 for 500 kbps)
     * @param network The CAN network type this interface belongs to (default: CanNetwork::Main)
     * 
     * @note The receiver thread starts immediately upon construction
     */
    CanInterface(Pin tx, Pin rx, Pin standby, uint32_t baudrate, CanNetwork network = CanNetwork::Main);

    /**
     * @brief Transmit a CAN message on the bus
     * 
     * This function forwards the provided CAN message to the underlying CAN driver
     * for transmission. The message is serialized and sent asynchronously.
     * 
     * @param[in] msg Pointer to a valid CanMessage structure to be sent. Must not be nullptr.
     * @return int Status code:
     *         - 0: Success
     *         - 1: CAN peripheral not initialized
     *         - 2: HAL transmission error
     *         - -1: Invalid argument (msg is nullptr)
     * 
     * @note This method is thread-safe
     */
    int write(CanMessage *msg);

    /**
     * @brief Register a callback for a specific CAN message ID
     * 
     * When a CAN frame with the given @p msg_id is received, the corresponding
     * callback will be invoked with the deserialized message as its argument.
     * The callback is executed in the context of the receiver thread.
     * 
     * @warning Only one callback per CAN ID is supported. Registering multiple callbacks
     *          for the same ID will result in only the first registered callback being invoked.
     * @warning Maximum of 16 callbacks can be registered (configurable via max_callbacks).
     * 
     * @param msg_id The CAN identifier (11-bit standard ID: 0x000 - 0x7FF) to associate with this callback
     * @param callback Function pointer of type CanCallback to be invoked when a
     *                 message with the given ID is received. Must not be nullptr.
     * 
     * @return int Status code:
     *         - 0: Success
     *         - -1: callback is nullptr
     *         - -2: Internal callback map is full (maximum callbacks already registered)
     */
    int register_callback(uint16_t msg_id, CanCallback callback);

    /**
     * @brief Register a callback that is invoked for every received CAN message
     * 
     * The specified callback is called for all successfully received messages,
     * regardless of their CAN ID. If an ID-specific callback is also registered
     * for a given message, the ID-specific callback is executed first, followed
     * by this "always" callback.
     * 
     * Only a single "always" callback can be active at any time; calling this
     * function again replaces any previously registered callback.
     * 
     * This is useful for logging, diagnostics, or implementing custom message routing.
     * 
     * 
     * @param callback Function pointer of type CanCallback to be invoked for
     *                 every received message. Must not be nullptr.
     * @return int Status code:
     *         - 0: Success
     *         - -1: callback is nullptr
     */
    int register_always_callback(CanCallback callback);

    /**
     * @brief Pause the receiver thread's message processing
     * 
     * After calling this function, the background receiver thread will stop processing
     * incoming messages until restart_receiver_execution() is called. The thread itself
     * continues running but enters a sleep loop and will not invoke any callbacks.
     * 
     * This is useful when you need to temporarily disable CAN message processing without
     * destroying the CanInterface object.
     * 
     * @note Messages that arrive while paused will remain in the CAN hardware FIFO
     *       and will be processed when the receiver is restarted (FIFO capacity permitting)
     */
    void stop_receiver_execution();

    /**
     * @brief Resume the receiver thread's message processing
     * 
     * This function re-enables processing of incoming CAN messages by the background
     * receiver thread after it has been paused with stop_receiver_execution().
     * 
     * @note Any messages that accumulated in the hardware FIFO while paused will be
     *       processed once reception is resumed
     */
    void restart_receiver_execution();

private:
    CAN     my_can;                              ///< Underlying CAN peripheral driver
    DigitalOut standby_pin;                      ///< Standby pin for controlling CAN transceiver power state
    bool    receiverRunning;                      ///< Flag to control receiver thread execution
    Thread  interface_thread;                     ///< Background thread for message reception
    Lock    callback_lock;                        ///< Mutex protecting callback arrays and registration
    CanNetwork network;                        ///< CAN network type (Main or Motor)

    int num_callbacks = 0;                        ///< Current number of registered ID-specific callbacks

    static constexpr int max_callbacks = 16;      ///< Maximum number of ID-specific callbacks

    uint16_t      CANid_arr[max_callbacks];      ///< Array of registered CAN IDs
    CanCallback callback_arr[max_callbacks];     ///< Array of callbacks corresponding to CAN IDs
    CanCallback alwayscallback;                  ///< Callback invoked for every received message

    /**
     * @brief Background receiver loop entry point (runs in dedicated thread)
     * 
     * This function continuously polls the CAN interface for new messages, dispatching
     * them to the appropriate per-ID callback and the "always" callback (if registered).
     * When reception is paused via stop_receiver_execution(), the loop sleeps briefly
     * instead of processing messages.
     * 
     * @note This function is intended to run indefinitely in a dedicated thread and
     *       should not be called directly by user code.
     */
    void receiver_thread();

    /**
     * @brief Receive a single CAN message and dispatch to registered callbacks
     * 
     * Attempts to read a single CAN message from the underlying driver. If a message
     * is successfully received, the function:
     *   1. Looks up any registered per-ID callback and invokes it (if present)
     *   2. Invokes the "always" callback, if registered
     * 
     * This function is normally called repeatedly from receiver_thread().
     * 
     * @note Callbacks are executed synchronously in the receiver thread context,
     *       so they should execute quickly to avoid blocking message processing.
     */
    void receive();

    /**
     * @brief Convert a byte array to a hexadecimal string representation
     * 
     * This utility function converts raw binary data into a human-readable hexadecimal
     * string format. Each byte is represented as two hexadecimal characters (e.g., 0xAB -> "AB").
     * 
     * This is useful for logging, debugging, and displaying CAN message payloads in a readable format.
     * 
     * @param[in] data Pointer to the byte array to be converted. Must not be nullptr.
     * @param[in] len Number of bytes in the data array to convert
     * @param[out] out_str Pointer to the output character buffer where the hexadecimal string
     *                     will be written. Must not be nullptr and must be large enough to hold
     *                     the result (minimum size: 2*len + 1 bytes for null terminator).
     * @param[in] out_str_size Size of the output buffer in bytes
     * 
     * @note The output string will be null-terminated
     */
    void bytes_to_hex(const uint8_t* data, uint8_t len, char* out_str, size_t out_str_size);

};
#endif
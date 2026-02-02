#ifndef CANINTERFACE_H
#define CANINTERFACE_H

#include <cstdint>

#include "can.h"        // CAN wrapper (CAN, CanMessage, SerializedCanMessage)
#include "thread.h"
#include "Clock.h"

/**
 * @brief Callback function type for CAN message reception
 * 
 * User-defined callbacks receive a const reference to the received raw CAN frame.
 * The callback should process the message quickly to avoid blocking the receiver thread.
 */
using CanCallback = void (*)(const SerializedCanMessage &msg);

/**
 * @brief High-level CAN interface with automatic message dispatching
 * 
 * This class provides a callback-based interface for CAN communication. It manages:
 * - A background receiver thread that continuously polls for incoming messages
 * - Automatic dispatching of messages to registered ID-specific callbacks
 * - Optional "always" callback invoked for every received message
 * - Thread-safe message transmission
 * 
 * The receiver thread runs continuously after construction and can be paused/resumed
 * as needed using stop_receiver_execution() and restart_receiver_execution().
 * 
 * @note Callback registration is not thread-safe and should be done during initialization
 *       before the receiver thread begins processing messages.
 * 
 * Example usage:
 * @code
 * void my_callback(const SerializedCanMessage& msg) {
 *     // Process message with ID 0x123
 * }
 * 
 * CanInterface can_if(TX_PIN, RX_PIN, 500000);
 * can_if.register_callback(0x123, my_callback);
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
     * @param tx Transmit pin used by the CAN peripheral
     * @param rx Receive pin used by the CAN peripheral
     * @param baudrate CAN bus baudrate in bits per second (e.g., 500000 for 500 kbps)
     * 
     * @note The receiver thread starts immediately upon construction
     */
    CanInterface(Pin tx, Pin rx, uint32_t baudrate);

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
     * @warning This function is not thread-safe and should only be called from a single
     *          context (e.g., during initialization) before messages start being processed.
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
     * @warning This function is not thread-safe and must only be called from a single
     *          context (e.g., during initialization).
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
    bool    receiverRunning;                      ///< Flag to control receiver thread execution
    Thread  interface_thread;                     ///< Background thread for message reception

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

};
#endif
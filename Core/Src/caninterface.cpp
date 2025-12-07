#include "CanInterface.h"

// global pointer used by the lambda
static CanInterface* g_canInterfaceInstance = nullptr;

/**
 * @brief Construct a new CanInterface object and start the receiver thread.
 *
 * This constructor initializes the underlying CAN peripheral with the given TX/RX pins
 * and baudrate, and starts a background thread that continuously receives CAN messages
 * and dispatches them to registered callbacks. It 
 *
 * @param tx       Transmit pin used by the CAN peripheral.
 * @param rx       Receive pin used by the CAN peripheral.
 * @param baudrate CAN bus baudrate in bits per second.
 */
CanInterface::CanInterface(Pin tx, Pin rx, uint32_t baudrate)
    : my_can(tx, rx, baudrate),
      receiverRunning(true),
      interface_thread(),
      alwayscallback(nullptr)
{
    // Remember this instance so the thread can call back into it
    g_canInterfaceInstance = this;

    // Start a FreeRTOS task using our Thread wrapper.
    interface_thread.start(+[]() { // Cannot use a non-static function with thread.start(), so we wrap it in Lambda
        g_canInterfaceInstance->receiver_thread();
    });
}

/**
 * @brief Transmit a CAN message on the bus.
 *
 * This function forwards the provided CAN message to the underlying CAN driver
 * for transmission.
 *
 * @param msg Pointer to a valid CanMessage structure to be sent. Must not be nullptr.
 * @return int CAN Driver-specific status code. Returns -1 if @p msg is nullptr.
 */
int CanInterface::write(CanMessage *msg)
{
    if (!msg) {
        return -1; // invalid argument
    }

    return my_can.write(msg);
}

/**
 * @brief Register a callback for a specific CAN message ID.
 *
 * When a CAN frame with the given @p msg_id is received, the corresponding
 * callback will be invoked with the deserialized message as its argument.
 *
 * The number of callbacks that can be registered is limit to a maximum amount.
 * This amount can be modified in the .h file for the CAN interface.
 * 
 * Is not designed to work with registering multiple callbacks for a single CAN ID.
 * If this is done, then only the first callback that was registered will be mapped to the CAN ID. 
 * 
 * This function is not thread-safe and should only be called from a single
 * context (e.g. the main thread).
 *
 * @param msg_id   The CAN identifier to associate with this callback.
 * @param callback Function pointer of type CanCallback to be invoked when a
 *                 message with the given ID is received. Must not be nullptr.
 *
 * @return int 0   on success.  
 *             -1  if @p callback is nullptr.  
 *             -2  if the internal callback map is full (no more callbacks can be registered).
 */
int CanInterface::register_callback(uint16_t msg_id, CanCallback callback)
{
    if (!callback) {
        return -1;
    }

    if (num_callbacks >= max_callbacks) { // too many callbacks
        return -2;
    }

    CANid_arr[num_callbacks] = msg_id;
    callback_arr[num_callbacks] = callback;

    num_callbacks++;

    return 0;
}

/**
 * @brief Register a callback that is invoked for every received CAN message.
 *
 * The specified callback is called for all successfully received messages,
 * regardless of their CAN ID. If an ID-specific callback is also registered
 * for a given message, that callback is executed first, followed by this
 * "always" callback.
 *
 * Only a single "always" callback can be active at any time; calling this
 * function again replaces any previously registered one.
 *
 * This function is not thread-safe and must only be called from a single
 * context (e.g. the main thread) while no concurrent modifications are made.
 *
 * @param callback Function pointer of type CanCallback to be invoked for
 *                 every received message. Must not be nullptr.
 * @return int 0 on success, or -1 if @p callback is nullptr.
 */
int CanInterface::register_always_callback(CanCallback callback)
{
    if (!callback) {
        return -1;
    }

    alwayscallback = callback;

    return 0;
}

/**
 * @brief Pause the receiver thread’s processing loop.
 *
 * After calling this function, the background receiver thread will stop processing
 * incoming messages until @ref restart_receiver_execution() is called. The thread
 * itself continues running but will not process CAN messages.
 */
void CanInterface::stop_receiver_execution()
{
    receiverRunning = false;
}

/**
 * @brief Resume the receiver thread’s processing loop.
 *
 * This function re-enables processing of incoming CAN messages by the background
 * receiver thread after it has been paused with @ref stop_receiver_execution().
 */
void CanInterface::restart_receiver_execution()
{
    receiverRunning = true;
}

/**
 * @brief Background receiver loop entry point.
 *
 * This function is intended to run in a dedicated thread. It continuously polls
 * the CAN interface for new messages, dispatching them to the appropriate
 * per-ID callback and the "always" callback (if registered). When reception
 * is paused via @ref stop_receiver_execution(), the loop sleeps briefly
 * instead of processing messages.
 */
void CanInterface::receiver_thread()
{
    while (true) {
        if (!receiverRunning) {
            Clock::sleep_for(10); // wait for 10 ms
            continue;
        }

        // Try to receive one message and dispatch callbacks
        receive();
    }
}

/**
 * @brief Receive a single CAN message and call callbacks.
 *
 * Attempts to read a single CAN message from the underlying driver. If a message
 * is successfully received, the function:
 *   - Looks up any registered per-ID callback and invokes it (if present).
 *   - Invokes the "always" callback, if registered.
 *
 * This function is normally called from @ref receiver_thread().
 */
void CanInterface::receive()
{
    SerializedCanMessage msg{};

    int ret = my_can.read(&msg);
    if (ret != 1) {
        return;
    }

    // Find index of matching CAN ID
    int idx = -1;
    for (int i = 0; i < num_callbacks; ++i) {
        if (CANid_arr[i] == msg.id) {
            idx = i;
            break;
        }
    }

    // Call the callback for the can ID
    if (idx != -1) {                 // only if we found one
        CanCallback cb = callback_arr[idx];
        if (cb) {
            cb(msg);
        }
    }

    // Call the "always" callback
    if (alwayscallback) {
        alwayscallback(msg);
    }
}

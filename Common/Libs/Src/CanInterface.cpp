#include "CanInterface.h"
#include "can.h"

// global pointer used by the lambda
static CanInterface* g_canInterfaceInstance = nullptr;
// TODO: change this so multiple CanInterface instances can exist at the same time (Main CAN + Motor CAN)

// TODO: concurrncy issues with register_callback if called while receiver thread is running?

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

int CanInterface::write(CanMessage *msg)
{
    if (!msg) {
        return -1; // invalid argument
    }

    return my_can.write(msg);
}

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

int CanInterface::register_always_callback(CanCallback callback)
{
    if (!callback) {
        return -1;
    }

    alwayscallback = callback;

    return 0;
}


void CanInterface::stop_receiver_execution()
{
    receiverRunning = false;
}


void CanInterface::restart_receiver_execution()
{
    receiverRunning = true;
}

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

void CanInterface::receive()
{
    SerializedCanMessage msg{};

    int ret = my_can.read(&msg);
    if (ret != 0) {
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
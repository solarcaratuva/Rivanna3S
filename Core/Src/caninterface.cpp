#include "CanInterface.h"

// Global (or static) pointer used by the thread trampoline
static CanInterface* g_canInterfaceInstance = nullptr;

CanInterface::CanInterface(Pin tx, Pin rx, uint32_t baudrate)
    : my_can(tx, rx, baudrate),
      receiverRunning(true),
      interface_thread()
{
    // Remember this instance so the thread can call back into it
    g_canInterfaceInstance = this;

    // Start a FreeRTOS task using our Thread wrapper.
    // Thread::start expects a `void (*)()`, so we use a captureless lambda
    // that calls `thread()` on the stored instance.
    interface_thread.start(+[]() {
        if (g_canInterfaceInstance) {
            g_canInterfaceInstance->receiver_thread();
        }
    });
}

// ---------------- TX: write() ----------------

int CanInterface::write(CanMessage *msg)
{
    if (!msg) {
        return -1; // invalid argument
    }

    // Just delegate to CAN wrapper
    return my_can.write(msg);
}

// ---------------- Callback registration ----------------
// should not call from multiple threads (just in main)
int CanInterface::register_callback(uint16_t msg_id, CanCallback callback)
{
    if (!callback) {
        return -1;
    }

    CANid_arr[num_callbacks] = msg_id;
    callback_arr[num_callbacks] = callback;

    num_callbacks++;

    return 0; // error int?
}

int CanInterface::register_always_callback(CanCallback callback)
{
    if (!callback) {
        return -1;
    }

    alwayscallback_arr[num_alwayscallbacks] = callback;

    num_alwayscallbacks++;

    return 0;
}

// ---------------- Receiver control ----------------

void CanInterface::stop_receiver_execution()
{
    receiverRunning = false;
}

void CanInterface::restart_receiver_execution()
{
    receiverRunning = true;
}

// ---------------- Receiver thread loop ----------------

void CanInterface::receiver_thread()
{
    for (;;) {
        if (!receiverRunning) {
            // Receiver paused; sleep a bit
            vTaskDelay(pdMS_TO_TICKS(10));
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
    if (ret != 1) {
        // no message or error; handle CANmessage read error here
        return;
    }

    // Find index of matching CAN ID
    int idx = -1;
    for (int i = 0; i < num_callbacks; ++i) {
        if (CANid_arr[i] == msg.id) {
            idx = i;
            break; // found it, stop searching
        }
    }

    // Call the per-ID callback at callback_arr[idx]
    if (idx != -1) {                 // only if we found one
        CanCallback cb = callback_arr[idx];
        if (cb) {
            cb(msg);
        }
    }

    // Call all "always" callbacks
    for (int i = 0; i < num_alwayscallbacks; ++i) {
        CanCallback cb = alwayscallback_arr[i];
        if (cb) {
            cb(msg);
        }
    }
}

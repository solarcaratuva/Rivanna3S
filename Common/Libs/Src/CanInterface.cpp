#include "CanInterface.h"
#include "can.h"

// global pointer used by the lambda
static CanInterface* main_can_interface_instance = nullptr;
static CanInterface* motor_can_interface_instance = nullptr;


CanInterface::CanInterface(Pin tx, Pin rx, uint32_t baudrate, CanNetwork network)
    : my_can(tx, rx, baudrate),
      receiverRunning(true),
      interface_thread(),
      alwayscallback(nullptr),
      network(network)
{
    // Remember this instance so the thread can call back into it
    if (network == CanNetwork::Main) {
        main_can_interface_instance = this;
        interface_thread.start(+[]() {
            main_can_interface_instance->receiver_thread();
        });
    } else if (network == CanNetwork::Motor) {
        motor_can_interface_instance = this;
        interface_thread.start(+[]() {
            motor_can_interface_instance->receiver_thread();
        });
    } else {
        log_warn("CanInterface: Unknown network type, receiver thread not started");
    }

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

    callback_lock.lock();

    if (num_callbacks >= max_callbacks) { // too many callbacks
        callback_lock.unlock();
        return -2;
    }

    CANid_arr[num_callbacks] = msg_id;
    callback_arr[num_callbacks] = callback;

    num_callbacks++;

    callback_lock.unlock();
    return 0;
}

int CanInterface::register_always_callback(CanCallback callback)
{
    if (!callback) {
        return -1;
    }

    callback_lock.lock();
    alwayscallback = callback;
    callback_lock.unlock();

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

    callback_lock.lock();

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

    callback_lock.unlock();
}
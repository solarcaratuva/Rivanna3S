#ifndef CANINTERFACE_H
#define CANINTERFACE_H

#include <cstdint> // 

#include "can.h"        // Your CAN wrapper (CAN, CanMessage, SerializedCanMessage)
#include "thread.h"
#include "Clock.h"

// Callback type: user gets a const reference to the received serialized CAN frame
using CanCallback = void (*)(const SerializedCanMessage &msg);

class CanInterface
{
public:
    CanInterface(Pin tx, Pin rx, uint32_t baudrate);

    // Send one logical CAN message using the underlying CAN class
    int write(CanMessage *msg);

    // Register callback for a specific CAN ID
    int register_callback(uint16_t msg_id, CanCallback callback);

    // Register a callback that is called for every received message
    int register_always_callback(CanCallback callback);

    // Pause the receiver thread (no callbacks are executed)
    void stop_receiver_execution();

    // Resume the receiver thread
    void restart_receiver_execution();

private:
    CAN     my_can;
    bool    receiverRunning;
    Thread  interface_thread;

    int num_callbacks = 0;

    static constexpr int max_callbacks = 16;

    uint16_t      CANid_arr[max_callbacks];
    CanCallback callback_arr[max_callbacks];
    CanCallback alwayscallback;
    // Receive one frame and dispatch callbacks
    void receiver_thread();
    void receive();

};
#endif
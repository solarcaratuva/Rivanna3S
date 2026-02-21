#include "CanInterface.h"
#include "can.h"
#include "log.h"
#include "thread.h"
#include "lock.h"

// global pointer used by the lambda
static CanInterface* main_can_interface_instance = nullptr;
static CanInterface* motor_can_interface_instance = nullptr;


CanInterface::CanInterface(Pin tx, Pin rx, Pin standby, uint32_t baudrate, CanNetwork network)
    : my_can(tx, rx, baudrate),
      standby_pin(standby),
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

    standby_pin.write(false); // ensure standby is low (transceiver enabled)

}

int CanInterface::write(CanMessage *msg)
{
    if (!msg) {
        return -1; // invalid argument
    }

    int status = my_can.write(msg);
    if (status == 0) {
        uint8_t data_hex[17]; // 16 bytes + null terminator
        // bytes_to_hex(msg->data, msg->len, reinterpret_cast<char*>(data_hex), sizeof(data_hex));
        // log_debug("CanInterface: Sent CAN message with ID %d Length %d Data 0x%s", msg->id, msg->len, data_hex);
    }
    return status;
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
    else {
        uint8_t data_hex[17]; // 16 bytes + null terminator
        bytes_to_hex(msg.data, msg.len, reinterpret_cast<char*>(data_hex), sizeof(data_hex));
        log_debug("CanInterface: Received CAN message with ID %d Length %d Data 0x%s", msg.id, msg.len, data_hex);
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


void CanInterface::bytes_to_hex(const uint8_t* data, uint8_t len, char* out_str, size_t out_str_size)
{
    const char hex_chars[] = "0123456789ABCDEF";

    if (out_str_size < (len * 2 + 1)) { // Check if there is enough space in output string
        if (out_str_size > 0) {
            out_str[0] = '\0';
        }
        return;
    }

    for (uint8_t i = 0; i < len; i++) {
        out_str[i * 2]     = hex_chars[(data[i] >> 4) & 0x0F];
        out_str[i * 2 + 1] = hex_chars[data[i] & 0x0F];
    }
    out_str[len * 2] = '\0'; // Null-terminate the string
}
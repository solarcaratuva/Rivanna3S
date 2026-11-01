#include "CanInterface.h"
#include "can.h"
#include "log.h"
#include "thread.h"
#include "lock.h"

// global pointer used by the lambda
static CanInterface* main_can_interface_instance = nullptr;
static CanInterface* motor_can_interface_instance = nullptr;
static constexpr uint32_t CAN_QUEUE_DROP_WARN_INTERVAL_MS = 250;
static constexpr int CAN_RECEIVE_BURST_LIMIT = 128;


CanInterface::CanInterface(Pin tx, Pin rx, Pin standby, uint32_t baudrate, CanNetwork network)
    : my_can(tx, rx, baudrate),
      standby_pin(standby),
      receiverRunning(true),
      receiver_thread_handle(),
      handler_thread_handle(),
      receive_queue(receive_queue_depth, sizeof(SerializedCanMessage)),
      network(network),
      alwayscallback(nullptr)
{
    // Remember this instance so the thread can call back into it
    if (network == CanNetwork::Main) {
        main_can_interface_instance = this;
        receiver_thread_handle.start(+[]() {
            main_can_interface_instance->receiver_thread();
        });
        handler_thread_handle.start(+[]() {
            main_can_interface_instance->handle_thread();
        });
    } else if (network == CanNetwork::Motor) {
        motor_can_interface_instance = this;
        receiver_thread_handle.start(+[]() {
            motor_can_interface_instance->receiver_thread();
        });
        handler_thread_handle.start(+[]() {
            motor_can_interface_instance->handle_thread();
        });
    } else {
        log_warn("CanInterface: Unknown network type, receiver/handler threads not started");
    }

    standby_pin.write(false); // ensure standby is low (transceiver enabled)

}

int CanInterface::write(SerializedCanMessage *msg) {
    if (!msg) {
        return -1; // invalid argument
    }

    int status = my_can.write(msg);
    if (status == 0) {
        char data_hex[17]; // 16 bytes + null terminator
        bytes_to_hex(msg->data, msg->len, data_hex, sizeof(data_hex));
        log_debug("CanInterface: Sent CAN message with ID %d Length %d Data 0x%s", msg->id, msg->len, data_hex);
    }
    return status;
}

int CanInterface::write(CanMessage *msg) {
    if (!msg) {
        return -1; // invalid argument
    }

    SerializedCanMessage scm{};
    msg->serialize(&scm);
    return write(&scm);
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
    auto enqueue_message = [this](const SerializedCanMessage &queued_msg) {
        SerializedCanMessage msg_copy = queued_msg;
        int queue_ret = receive_queue.append_to_back(&msg_copy, 0);
        if (queue_ret == -2) {
            uint32_t now_ms = Clock::get_current_time();
            if ((now_ms - last_queue_drop_warn_time_ms) >= CAN_QUEUE_DROP_WARN_INTERVAL_MS) {
                last_queue_drop_warn_time_ms = now_ms;
                log_warn("CanInterface: Software RX queue full on %s CAN; dropping frames", CanNetworkToString(network));
            }
        }
    };

    int ret = my_can.read(&msg);
    if (ret != 0) {
        return;
    }

    enqueue_message(msg);

    //try to read burst onto the queue
    for (int i = 0; i < CAN_RECEIVE_BURST_LIMIT; ++i) {
        ret = my_can.try_read(&msg);
        if (ret == 3) {
            break;
        }

        if (ret != 0) {
            break;
        }

        enqueue_message(msg);
    }
}

void CanInterface::handle_thread()
{
    while (true) {
        if (!receiverRunning) {
            Clock::sleep_for(10);
            continue;
        }

        handle();
    }
}

void CanInterface::handle()
{
    SerializedCanMessage msg{};
    if (receive_queue.get(&msg) != 0) {
        return;
    }

    char data_hex[17]; // 16 bytes + null terminator
    bytes_to_hex(msg.data, msg.len, data_hex, sizeof(data_hex));
    log_debug("CanInterface: Received CAN message with ID %d Length %d Data 0x%s", msg.id, msg.len, data_hex);

    CanCallback cb = nullptr;
    CanCallback always_cb = nullptr;

    callback_lock.lock();

    for (int i = 0; i < num_callbacks; ++i) {
        if (CANid_arr[i] == msg.id) {
            cb = callback_arr[i];
            break;
        }
    }

    always_cb = alwayscallback;
    callback_lock.unlock();

    if (cb) {
        cb(msg);
    }

    if (always_cb) {
        always_cb(msg);
    }
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

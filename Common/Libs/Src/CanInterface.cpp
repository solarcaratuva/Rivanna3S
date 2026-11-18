#include "CanInterface.h"
#include "can.h"
#include "log.h"
#include "thread.h"
#include "lock.h"

// global pointer used by the lambda
static CanInterface* main_can_interface_instance = nullptr;
static CanInterface* motor_can_interface_instance = nullptr;
static constexpr uint32_t CAN_RX_LOST_WARN_INTERVAL_MS = 250;

CanInterface::CanInterface(Pin tx, Pin rx, Pin standby, uint32_t baudrate, CanNetwork network)
    : my_can(tx, rx, baudrate),
      standby_pin(standby),
      receiverRunning(true),
      receiver_thread_handle(),
      network(network),
      alwayscallback(nullptr)
{
    // Remember this instance so the thread can call back into it
    if (network == CanNetwork::Main) {
        main_can_interface_instance = this;
        receiver_thread_handle.start(+[]() {
            main_can_interface_instance->receiver_thread();
        });
    } else if (network == CanNetwork::Motor) {
        motor_can_interface_instance = this;
        receiver_thread_handle.start(+[]() {
            motor_can_interface_instance->receiver_thread();
        });
    } else {
        log_warn("CanInterface: Unknown network type, receiver thread not started");
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
    if (my_can.read(&msg) != 0) {
        return;
    }

    handle(msg);
    report_lost_frames();
}

void CanInterface::handle(const SerializedCanMessage &msg)
{
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

void CanInterface::report_lost_frames()
{
    /* Frames the peripheral dropped because RX FIFO0 was full when they
       arrived, counted in the interrupt. A full FIFO is not loss; this is. */
    const uint32_t lost = my_can.get_rx_lost_count();
    if (lost == last_reported_rx_lost_count) {
        return;
    }

    const uint32_t now = Clock::get_current_time();
    if (now - last_rx_lost_warn_time_ms < CAN_RX_LOST_WARN_INTERVAL_MS) {
        return;
    }

    last_rx_lost_warn_time_ms = now;
    last_reported_rx_lost_count = lost;
    log_warn("CanInterface: RX FIFO0 lost %lu frames on %s CAN",
             static_cast<unsigned long>(lost), CanNetworkToString(network));
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

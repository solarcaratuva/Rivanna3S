
#include "MessageQueue.h"
#include "FreeRTOS.h"
#include "message_buffer.h"

// Constructor
MessageQueue::MessageQueue(uint16_t buffer_length)
    : buffer_length_(buffer_length) {

    // Create FreeRTOS message buffer with specified byte capacity
    message_buffer_handle = xMessageBufferCreate(buffer_length);

    if (message_buffer_handle != NULL) {
        initialized = true;
    } else {
        initialized = false;
    }
}

// Destructor
MessageQueue::~MessageQueue() {
    if (message_buffer_handle != NULL) {
        vMessageBufferDelete(message_buffer_handle);
    }
}

int MessageQueue::send(uint8_t *tx_buffer, uint16_t len) {
    if (!initialized || tx_buffer == nullptr) {
        return -1;
    }

    // Try to send without blocking (timeout = 0)
    // If buffer is full, this will return 0 bytes sent
    size_t bytes_sent = xMessageBufferSend(message_buffer_handle, tx_buffer, len, 0);

    if (bytes_sent == len) {
        return 0;  // Success - all bytes sent
    } else {
        return -1;  // Buffer full or error
    }
}

int MessageQueue::receive(uint8_t *rx_buffer, uint16_t len) {
    if (!initialized || rx_buffer == nullptr) {
        return -1;
    }

    // Block indefinitely until a message is available
    size_t bytes_received = xMessageBufferReceive(message_buffer_handle, rx_buffer, len, portMAX_DELAY);

    return static_cast<int>(bytes_received);
}

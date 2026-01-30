/**
 * @file MessageQueue.h
 * @brief C++ wrapper class for FreeRTOS message buffer
 *
 * This class provides a high-level object-oriented interface for FreeRTOS
 * message buffers, simplifying inter-task communication with variable-length
 * messages.
 *
 * The send operation is non-blocking and will skip the message if the buffer
 * is full. The receive operation blocks until a message is available.
 *
 * Example usage:
 * @code
 * #include "MessageQueue.h"
 *
 * MessageQueue queue(512);  // Create message buffer with 512 bytes capacity
 *
 * uint8_t tx_data[] = {0x01, 0x02, 0x03};
 * queue.send(tx_data, sizeof(tx_data));
 *
 * uint8_t rx_buffer[10];
 * int bytes_received = queue.receive(rx_buffer, sizeof(rx_buffer));
 * @endcode
 *
 * @note This class uses FreeRTOS message buffer API internally
 * @see FreeRTOS.h
 * @see message_buffer.h
 */

#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include "FreeRTOS.h"
#include "message_buffer.h"
#include <cstdint>

/**
 * @class MessageQueue
 * @brief Provides FreeRTOS message buffer wrapper for inter-task communication
 */
class MessageQueue {
public:
    /**
     * @brief Indicates whether the message buffer was successfully initialized
     */
    bool initialized = false;

    /**
     * @brief Constructs a MessageQueue object and initializes the FreeRTOS message buffer
     * @param buffer_length Total size of the message buffer in bytes
     */
    explicit MessageQueue(uint16_t buffer_length);

    /**
     * @brief Sends a message to the buffer (non-blocking)
     * @param tx_buffer Pointer to the data to send
     * @param len Length of the data to send
     * @return 0 on success, -1 if buffer is full or not initialized
     *
     * If the buffer is full, the message will be skipped and the function
     * returns immediately with -1.
     */
    int send(uint8_t *tx_buffer, uint16_t len);

    /**
     * @brief Receives a message from the buffer (blocking)
     * @param rx_buffer Pointer to buffer where received data will be stored
     * @param len Maximum length of data to receive
     * @return Number of bytes received, or -1 if not initialized
     *
     * This function blocks until a message is available in the buffer.
     */
    int receive(uint8_t *rx_buffer, uint16_t len);

    /**
     * @brief Destructor - deletes the FreeRTOS message buffer
     */
    ~MessageQueue();

private:
    MessageBufferHandle_t message_buffer_handle;  /**< FreeRTOS message buffer handle */
    uint16_t buffer_length_;                      /**< Buffer capacity in bytes */
};

#endif // MESSAGEQUEUE_H

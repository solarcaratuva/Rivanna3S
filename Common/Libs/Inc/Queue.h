#ifndef CUSTOMQUEUE_H
#define CUSTOMQUEUE_H

#include "FreeRTOS.h"
#include "queue.h"

/**
 * @brief C++ wrapper class for FreeRTOS queues
 * 
 * This class provides a simple C++ interface for creating and managing FreeRTOS queues.
 */
class Queue {
public:
    /**
     * @brief Construct a new Queue object
     * 
     * Creates a queue with the specified length and item size.
     * 
     * @param length The maximum number of items the queue can hold
     * @param datatype_size The size of each item in bytes
     * 
     * @return A new Queue object with an initialized FreeRTOS queue handle
     */
    Queue(uint16_t length, uint16_t datatype_size);


    /**
     * @brief Append an item to the back of the queue
     * 
     * @param item Pointer to the item to be added to the queue
     * @param wait_time Maximum time to wait for space to become available (in ticks) (Default is portMAX_DELAY (wait indefinitely))
     * 
     * @return 0 on success, -1 if queue is not initialized, -2 if failed to add item
     */
    int append_to_back(void* item, uint32_t wait_time = portMAX_DELAY);


    /**
     * @brief Append an item to the front of the queue
     * 
     * @param item Pointer to the item to be added to the queue
     * @param wait_time Maximum time to wait for space to become available (in ticks) (Default is portMAX_DELAY (wait indefinitely))
     * 
     * @return 0 on success, -1 if queue is not initialized, -2 if failed to add item
     */
    int append_to_front(void* item, uint32_t wait_time = portMAX_DELAY);


    /**
     * @brief Overwrite the item in the queue (only for queues of length 1)
     * 
     * @param item Pointer to the item to be added to the queue
     * 
     * @return 0 on success, -1 if queue is not initialized, -2 if failed to overwrite item
     */
    int overwrite(void* item);


    /**
     * @brief Get an item from the queue
     * 
     * @param item Pointer to the buffer where the received item will be stored
     * @param wait_time Maximum time to wait for an item to become available (in ticks) (Default is portMAX_DELAY (wait indefinitely))
     * 
     * @return 0 on success, -1 if queue is not initialized, -2 if failed to receive item
     */
    int get(void* item, uint32_t wait_time = portMAX_DELAY);


    /**
     * @brief Peek at the item at the front of the queue without removing it
     * 
     * @param item Pointer to the buffer where the peeked item will be stored
     * @param wait_time Maximum time to wait for an item to become available (in ticks) (Default is portMAX_DELAY (wait indefinitely))
     * 
     * @return 0 on success, -1 if queue is not initialized, -2 if failed to peek item
     */
    int peek(void* item, uint32_t wait_time = portMAX_DELAY);
private:
    QueueHandle_t queue_handle;
    uint16_t length;
    uint16_t datatype_size;
};

#endif
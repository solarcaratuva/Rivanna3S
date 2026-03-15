#include "Queue.h"

Queue::Queue(uint16_t length, uint16_t datatype_size){
    this->length = length;
    this->datatype_size = datatype_size;

    queue_handle = xQueueCreate(length, datatype_size);
}

int Queue::append_to_back(void* item, uint32_t wait_time){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    if (xQueueSendToBack(queue_handle, item, wait_time) != pdPASS){
        return -2; // Failed to send item
    }

    return 0; // Success
}

int Queue::append_to_front(void* item, uint32_t wait_time){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    if (xQueueSendToFront(queue_handle, item, wait_time) != pdPASS){
        return -2; // Failed to send item
    }

    return 0; // Success
}

int Queue::overwrite(void* item){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    if (xQueueOverwrite(queue_handle, item) != pdPASS){
        return -2; // Failed to overwrite item
    }

    return 0; // Success
}

int Queue::get(void* item, uint32_t wait_time){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    if (xQueueReceive(queue_handle, item, wait_time) != pdPASS){
        return -2; // Failed to receive item
    }

    return 0; // Success
}

int Queue::peek(void* item, uint32_t wait_time){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    if (xQueuePeek(queue_handle, item, wait_time) != pdPASS){
        return -2; // Failed to peek item
    }

    return 0; // Success
}
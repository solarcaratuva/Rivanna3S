#include "FiniteQueue.h"

FiniteQueue::FiniteQueue(uint16_t length, uint16_t datatype_size){
    this->length = length;
    this->datatype_size = datatype_size;
    item_count = 0;

    queue_handle = xQueueCreate(length, datatype_size);
}

int FiniteQueue::append_to_back(void* item, uint32_t wait_time){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    // Convert wait_time from milliseconds to ticks if it's not portMAX_DELAY
    if (wait_time != portMAX_DELAY) {
        wait_time = pdMS_TO_TICKS(wait_time);
    }

    if (xQueueSendToBack(queue_handle, item, wait_time) != pdPASS){
        return -2; // Failed to send item
    }

    if (item_count < length) {
        item_count++;
    }

    return 0; // Success
}

int FiniteQueue::append_to_front(void* item, uint32_t wait_time){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    // Convert wait_time from milliseconds to ticks if it's not portMAX_DELAY
    if (wait_time != portMAX_DELAY) {
        wait_time = pdMS_TO_TICKS(wait_time);
    }

    if (xQueueSendToFront(queue_handle, item, wait_time) != pdPASS){
        return -2; // Failed to send item
    }

    if (item_count < length) {
        item_count++;
    }

    return 0; // Success
}

int FiniteQueue::overwrite(void* item){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    if (xQueueOverwrite(queue_handle, item) != pdPASS){
        return -2; // Failed to overwrite item
    }

    item_count = 1;

    return 0; // Success
}

int FiniteQueue::get(void* item, uint32_t wait_time){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    // Convert wait_time from milliseconds to ticks if it's not portMAX_DELAY
    if (wait_time != portMAX_DELAY) {
        wait_time = pdMS_TO_TICKS(wait_time);
    }

    if (xQueueReceive(queue_handle, item, wait_time) != pdPASS){
        return -2; // Failed to receive item
    }

    if (item_count > 0) {
        item_count--;
    }

    return 0; // Success
}

int FiniteQueue::peek(void* item, uint32_t wait_time){
    if (queue_handle == nullptr) {
        return -1; // Queue not initialized
    }

    // Convert wait_time from milliseconds to ticks if it's not portMAX_DELAY
    if (wait_time != portMAX_DELAY) {
        wait_time = pdMS_TO_TICKS(wait_time);
    }

    if (xQueuePeek(queue_handle, item, wait_time) != pdPASS){
        return -2; // Failed to peek item
    }

    return 0; // Success
}

uint16_t FiniteQueue::size() const{
    return item_count;
}
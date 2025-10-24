#include "lock.h"

Lock::Lock()
    : handle_(xSemaphoreCreateMutex())
{
    configASSERT(handle_ != nullptr); // ensure it was created
}

Lock::~Lock() {
    if (handle_) {
        vSemaphoreDelete(handle_);
    }
}

bool Lock::lock() {
    return xSemaphoreTake(handle_, portMAX_DELAY) == pdTRUE;
}

bool Lock::try_lock() {
    return xSemaphoreTake(handle_, 0) == pdTRUE;
}

bool Lock::unlock() {
    return xSemaphoreGive(handle_) == pdTRUE;
}

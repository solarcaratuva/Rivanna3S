#ifndef LOCK_H
#define LOCK_H

#include "FreeRTOS.h"
#include "semphr.h"

class Lock {
public:
    Lock();
    ~Lock();

    // Block indefinitely
    bool lock();
    // Non-blocking
    bool try_lock();
    // Release
    bool unlock();

private:
    SemaphoreHandle_t handle_;
};

#endif // LOCK_H
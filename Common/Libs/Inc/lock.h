#ifndef LOCK_H
#define LOCK_H

#include "FreeRTOS.h"
#include "semphr.h"

/**
 * @brief Mutex wrapper class for thread-safe synchronization using FreeRTOS semaphores
 * 
 * Provides RAII-style mutex operations for protecting shared resources in multi-threaded
 * environments. Built on top of FreeRTOS binary semaphores.
 */
class Lock {
public:
    /**
     * @brief Construct a new Lock object and create the underlying semaphore
     */
    Lock();
    
    /**
     * @brief Destroy the Lock object and delete the underlying semaphore
     */
    ~Lock();

    /**
     * @brief Acquire the lock, blocking indefinitely until available
     * @return true if lock was successfully acquired
     * @return false if lock acquisition failed
     */
    bool lock();
    
    /**
     * @brief Attempt to acquire the lock without blocking
     * @return true if lock was immediately acquired
     * @return false if lock was not available
     */
    bool try_lock();
    
    /**
     * @brief Release the lock, allowing other threads to acquire it
     * @return true if lock was successfully released
     * @return false if lock release failed
     */
    bool unlock();

private:
    SemaphoreHandle_t handle_;
};

#endif // LOCK_H

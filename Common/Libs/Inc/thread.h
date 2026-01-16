#ifndef THREAD_H
#define THREAD_H
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief Thread wrapper class for creating and managing FreeRTOS tasks
 * 
 * Simplifies task creation by wrapping FreeRTOS task APIs. Each Thread object
 * manages one FreeRTOS task with default priority and stack size.
 */
class Thread {
public:
    /**
     * @brief Construct a new Thread object
     * 
     * Initializes the thread handle. The task is not created until start() is called.
     */
    Thread();
    
    /**
     * @brief Create and start a FreeRTOS task
     * @param fn Function pointer to the task entry point (void function with no parameters)
     * @return BaseType_t pdPASS if task was created successfully, errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY otherwise
     * 
     * Creates a FreeRTOS task that will begin execution immediately.
     * The function should contain an infinite loop for continuous task operation.
     * 
     * @note Default task priority and stack size are used (defined in implementation)
     */
    BaseType_t start(void(*fn)());

private:
    TaskHandle_t handle_;
};
#endif

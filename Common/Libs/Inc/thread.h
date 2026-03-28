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
    BaseType_t start(void (*fn)());


    /**
     * @brief Get the handle of the currently executing FreeRTOS task
     * @return TaskHandle_t Handle of the calling task
     * 
     * Returns the FreeRTOS task handle associated with the task that
     * invokes this function. This is a thin wrapper around the
     * FreeRTOS API function xTaskGetCurrentTaskHandle().
     * 
     * This function is static and does not require a Thread instance.
     * It can be called from any task context to obtain its own handle,
     * which can then be used with other FreeRTOS APIs (e.g., task
     * notifications, suspension, or deletion).
     * 
     * @note Must be called from a task context. Behavior is undefined
     *       if called before the scheduler starts or from an ISR.
     */
    static TaskHandle_t get_task_handle();

private:
    TaskHandle_t handle_;
};
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "thread.h"

#define STACK_SIZE 1000
#define PRIORITY 1

Thread::Thread() 
    : handle_(NULL)
    {}

BaseType_t Thread::start(std::function<void()> fn) {
    callback_ = fn;
    
    return xTaskCreate(
        Thread::task_wrapper,
        "HELPER", 
        STACK_SIZE,
        this,  // Pass this instance as parameter
        PRIORITY, 
        &handle_);
}

void Thread::task_wrapper(void* pvParameters) {
    Thread* instance = static_cast<Thread*>(pvParameters);
    if(instance && instance->callback_) {
        // Run the function in a loop for continuous execution
        while(1) {
            instance->callback_();
        }
    }
    // This should never be reached, but just in case
    vTaskDelete(NULL);
}

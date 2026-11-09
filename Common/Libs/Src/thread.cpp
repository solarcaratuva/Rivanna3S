#include "FreeRTOS.h"
#include "task.h"
#include "thread.h"

Thread::Thread() 
    : handle_(NULL)
    {}

BaseType_t Thread::start(void (*fn)(), uint16_t stack_words, UBaseType_t priority) {
        return xTaskCreate(
            +[](void* pv) {
                auto fn = reinterpret_cast<void(*)()>(pv);
                // Run the function in a loop for continuous execution
                while(1) {
                    fn();
                    taskYIELD();
                }
                // This should never be reached, but just in case
                vTaskDelete(NULL);
            },
            "HELPER", stack_words,
            reinterpret_cast<void*>(fn), // Pass the function pointer as parameter
            priority, &handle_);
}

TaskHandle_t Thread::get_task_handle(){
    return xTaskGetCurrentTaskHandle();
}

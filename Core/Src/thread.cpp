#include "FreeRTOS.h"
#include "task.h"
#include "thread.h"

#define STACK_SIZE 1000
#define PRIORITY 1

Thread::Thread() {
    handle_ = NULL;
}

BaseType_t Thread::start(void (*fn)()) {
        return xTaskCreate(
            +[](void* pv) {
                auto fn = reinterpret_cast<void(*)()>(pv);
                // Run the function in a loop for continuous execution
                while(1) {
                    fn();
                }
                // This should never be reached, but just in case
                vTaskDelete(NULL);
            },
            "HELPER", STACK_SIZE,
            reinterpret_cast<void*>(fn), // Pass the function pointer as parameter
            PRIORITY, &handle_);
    }

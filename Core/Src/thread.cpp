#include "FreeRTOS.h"
#include "task.h"
#include "thread.h"

#define STACK_SIZE 1000
#define PRIORITY 1
class Thread {
  Thread() { semaphoreHandler = xSemaphoreCreateBinary(); }

public:
//   int start(void *functionName) {
//     BaseType_t xReturned;
//     TaskHandle_t xHandle = NULL;
// 
//     /* Create the task, storing the handle. */
//     xReturned =
//         xTaskCreate(+[](void* functionName) {}, /* Function that implements the task. */
//                     "HELPER",      /* Text name for the task. */
//                     STACK_SIZE,    /* Stack size in words, not bytes. */
//                     NULL, /* Parameter passed into the task. */
//                     PRIORITY,  /* Priority at which the task is created. */
//                     &xHandle); /* Used to pass out the created task's handle. */
//   }
//   int join() { xSemaphoreTake(semaphoreHandler, 100); }
    BaseType_t start(void (*fn)()) {
        return xTaskCreate(
            +[](void* pv) {
                auto fn = reinterpret_cast<void(*)()>(pv);
                fn();
                // xSemaphoreGive(done_) ... (needs access; store `this` in pv instead)
                vTaskDelete(nullptr);
            },
            "HELPER", STACK_SIZE,
            NULL, 
            PRIORITY, &handle_);
    }
// private:
//   SemaphoreHandle_t semaphoreHandler;
//   int startHelper(void *functionName) {
//     xSemaphoreTake(semaphoreHandler, 100);
//     functionName();
//     xSemaphoreGive(semaphoreHandler);
//   }
// };
// extern "C" void my_c_function(void *functionName) {
//   MyClass::startHelper(functionName);
};

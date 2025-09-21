#include ".h" // FIND .H FILE!!!

#define STACK_SIZE 1000
#define PRIORITY 1
class Thread {
  Thread() { semaphoreHandler = xSemaphoreCreateBinary(); }

public:
  int start(void *functionName) {
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    /* Create the task, storing the handle. */
    xReturned =
        xTaskCreate(my_c_function, /* Function that implements the task. */
                    "HELPER",      /* Text name for the task. */
                    STACK_SIZE,    /* Stack size in words, not bytes. */
                    (void *)functionName, /* Parameter passed into the task. */
                    PRIORITY,  /* Priority at which the task is created. */
                    &xHandle); /* Used to pass out the created task's handle. */
  }
  int join() { xSemaphoreTake(semaphoreHandler, 100); }

private:
  SemaphoreHandle_t semaphoreHandler;
  int startHelper(void *functionName) {
    xSemaphoreTake(semaphoreHandler, 100);
    functionName();
    xSemaphoreGive(semaphoreHandler);
  }
};
extern "C" void my_c_function(void *functionName) {
  MyClass::startHelper(functionName);
}

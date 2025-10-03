#include "FreeRTOS.h"
#include "task.h"

#define STACK_SIZE 1000
#define PRIORITY 1

class Thread {
public:
    Thread();
    BaseType_t start(void(*fn)());

private:
    TaskHandle_t handle_;
};

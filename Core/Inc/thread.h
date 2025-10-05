#include "FreeRTOS.h"
#include "task.h"

#define STACK_SIZE 1000
#define PRIORITY 1

class Thread {
public:
    Thread();
    /** 
     * Function that starts a task in a thread
     * Tasks started with this function will not start until vTaskStartScheduler() is ran in main
     * Usage: BaseType_t <result_name> = <thread object>.start(<function name>)
     * Takes in a function that has a single void* parameter, passing in a function with no parameters may work but is undefined behavior 
     */
    BaseType_t start(void(*fn)());

private:
    TaskHandle_t handle_;
};

#ifndef THREAD_H
#define THREAD_H
#include "FreeRTOS.h"
#include "task.h"

class Thread {
public:
    Thread();
    /** 
     * Function that starts a task in a thread
     * Tasks started with this function will not start until vTaskStartScheduler() is ran in main
     * Usage: BaseType_t <result_name> = <thread object>.start(<function name>)
     * Takes in a function that has returns nothing (void function) and has no parameters
     */
    BaseType_t start(void(*fn)());

private:
    TaskHandle_t handle_;
};
#endif

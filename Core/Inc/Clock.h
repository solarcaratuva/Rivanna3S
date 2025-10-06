#ifndef TIME_H
#define TIME_H


#include "FreeRTOS.h"
#include "task.h"

class Clock {

public:
	Clock(); // Constructor sets lastSleepSinceCall
    /*
     * Function that sleeps for time_ms since the last time_ms
     */
    void sleep_since(uint32_t time_ms);

    /*
     * Function that sleeps for time_ms
     */ 
    static void sleep_for(uint32_t time_ms);

    /* 
     * Function that returns the current time
     */
    static uint32_t get_current_time();
private:
    TickType_t lastSleepSinceCall;
};


#endif // TIME_H

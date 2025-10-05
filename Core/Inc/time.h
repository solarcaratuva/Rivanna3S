#ifndef TIME_H
#define TIME_H

#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"

class Time() {
public:
    Time();
    void sleep_since(uint32_t time_ms);
    static void sleep_for(uint32_t time_ms);
    static uint32_t get_current_time();
private:
    TickType_t xLastSleepSinceCall;
}
#endif

#include "time.h"

Time::Time() {
    xLastSleepSinceCall = xTaskGetTickCount;
}
Time::void sleep_since(uint32_t time_ms) {
    TickType_t tickCount = time_ms / portTICK_PERIOD_MS; // this converts the ms into ticks
    vTaskDelayUntil(&xLastSleepSinceCall, tickCount); // xLastSleepSinceCall is updated here
    return;
}

Time::void sleep_for(uint32_t time_ms) {
    const TickType_t xDelay = time_ms / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );
    return;
}

Time::uint32_t get_current_time() {
    TickType_t tickCount = xTaskGetTickCount();
    return static_cast<uint32_t>(tickCount * portTICK_PERIOD_MS); 
}

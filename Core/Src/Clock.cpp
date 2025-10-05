#include "Clock.h"

Clock::Clock() {
    xLastSleepSinceCall = xTaskGetTickCount();
}

void Clock::sleep_since(uint32_t time_ms) {
    TickType_t tickCount = time_ms / portTICK_PERIOD_MS; // this converts the ms into ticks
    vTaskDelayUntil(&xLastSleepSinceCall, tickCount); // xLastSleepSinceCall is updated here
    return;
}

void Clock::sleep_for(uint32_t time_ms) {
    const TickType_t xDelay = time_ms / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );
    return;
}

uint32_t Clock::get_current_time() {
    TickType_t tickCount = xTaskGetTickCount();
    return static_cast<uint32_t>(tickCount * portTICK_PERIOD_MS); 
}

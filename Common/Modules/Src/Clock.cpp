#include "Clock.h"

Clock::Clock() {
    lastSleepSinceCall = xTaskGetTickCount();
}

void Clock::sleep_since(uint32_t time_ms) {
    TickType_t tick_count = time_ms / portTICK_PERIOD_MS; // this converts the ms into ticks
    vTaskDelayUntil(&lastSleepSinceCall, tick_count); // xLastSleepSinceCall is updated here
    return;
}

void Clock::sleep_for(uint32_t time_ms) {
    const TickType_t delay = time_ms / portTICK_PERIOD_MS;
    vTaskDelay(delay);
    return;
}

uint32_t Clock::get_current_time() {
    TickType_t tick_count = xTaskGetTickCount();
    return static_cast<uint32_t>(tick_count * portTICK_PERIOD_MS); 
}

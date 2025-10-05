#include "time.h"

Time::sleep_since(uint32_t time_ms) {
    // TODO   
}

Time::void sleep_for(uint32_t time_ms) {
    const TickType_t xDelay = time_ms / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );
    return;
}

Time::uint32_t get_current_time() {
    // TODO
    return portTick_Period_MS;
}

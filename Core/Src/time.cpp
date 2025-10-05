
class Time() {
public:
    void sleep_since(uint32_t time_ms);
    static void sleep_for(uint32_t time_ms);
    static uint32_t get_current_time();
private:
    TickType_t xLastWakeTime;
}


Time::sleep_since(uint32_t time_ms) {
    
}

Time::void sleep_for(uint32_t time_ms) {
    const TickType_t xDelay = time_ms / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );
    return;
}

Time::uint32_t get_current_time() {
    return portTick_Period_MS;
}

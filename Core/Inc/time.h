#ifndef TIME_H
#define TIME_H

class Time() {
public:
    void sleep_since(uint32_t time_ms);
    static void sleep_for(uint32_t time_ms);
    static uint32_t get_current_time();
private:

}
#endif

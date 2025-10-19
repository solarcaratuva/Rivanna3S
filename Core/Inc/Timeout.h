#ifndef TIMEOUT_H
#define TIMEOUT_H

#include "FreeRTOS.h"
#include "timers.h"
#include <functional>


class Timeout {
public:
    Timeout();

    /**
     * @brief Attach a callback to be called after a specified timeout.
     * 
     * If a timer is already running, it will be deleted and replaced.
     * 
     * @param cb        Function to call on timeout.
     * @param time_ms   Timeout period in milliseconds.
     * @return          0 on success, -1 on create failure, -2 on start failure.
    */
    int attach(std::function<void()> cb, uint32_t time_ms);

    /** 
     * @brief Refresh the timer countdown using the same timeout period
     *  
     * @return  0 on success, -1 if no timer exists, -2 on reset failure.
    */
    int refresh();

    /**
     * @brief Stop the timer before it expires;
     * 
     * @return  0 on success, -1 if no timer exists, -2 on stop failure. 
    */
    int stop();

private:
    TimerHandle_t timer_handle;         // FreeRTOS timer handle
    std::function<void()> callback;     // Function to call on timeout
    uint32_t timeout_ms;                // Timeout duration (ms)

    static void timer_callback(TimerHandle_t xTimer);
};

#endif
#ifndef TIME_H
#define TIME_H


#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief Time management class for delays and timing operations using FreeRTOS
 * 
 * Provides both instance-based periodic timing and static utility functions for
 * delays and time measurement. Uses FreeRTOS tick counts for timing.
 */
class Clock {

public:
    /**
     * @brief Construct a new Clock object and initialize timing reference
     * 
     * Sets the initial reference point for sleep_since() calls to the current tick count.
     */
	Clock();
    
    /**
     * @brief Sleep until a specified time has elapsed since the last call
     * @param time_ms Target period in milliseconds
     * 
     * Delays execution to maintain a consistent period between successive calls.
     * Useful for periodic tasks that need to run at fixed intervals regardless of
     * processing time. Updates the internal reference point after each call.
     */
    void sleep_since(uint32_t time_ms);

    /**
     * @brief Sleep for a specified duration
     * @param time_ms Duration to sleep in milliseconds
     * 
     * Static function that blocks the calling task for the specified duration.
     * Uses vTaskDelay() internally.
     */ 
    static void sleep_for(uint32_t time_ms);

    /**
     * @brief Get the current system time
     * @return uint32_t Current time in milliseconds since system start
     * 
     * Returns the FreeRTOS tick count converted to milliseconds. May wrap around
     * after approximately 49 days of continuous operation.
     */
    static uint32_t get_current_time();
private:
    TickType_t lastSleepSinceCall;
};


#endif // TIME_H

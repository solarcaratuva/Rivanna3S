#include "Timeout.h"

Timeout::Timeout():timer_handle(nullptr), timeout_ms(0) {}


int Timeout::attach(std::function<void()> cb, uint32_t time_ms){
    // Delete existing timer if one exists
    if(timer_handle != nullptr){
        xTimerDelete(timer_handle, 0);
        timer_handle = nullptr;
    }

    callback = cb;
    timeout_ms = time_ms;

    // Create a one-shot software timer
    timer_handle = xTimerCreate(
        "TimeoutTimer",
        pdMS_TO_TICKS(time_ms),
        pdFalse,        // one-shot timer
        this,           // timer ID = this instance
        Timeout::timer_callback
    );

    if(timer_handle == nullptr){
        return -1; // Failed to create timer
    }

    if(xTimerStart(timer_handle, 0) != pdPASS){
        return -2; // Failed to start timer
    }

    return 0; // Success
}


int Timeout::refresh(){
    if(timer_handle == nullptr){
        return -1;
    }

    if(xTimerReset(timer_handle, 0) != pdPASS){
        return -2;
    }

    return 0;
}


int Timeout::stop(){
    if(timer_handle == nullptr){
        return -1;
    }

    if(xTimerStop(timer_handle, 0) != pdPASS){
        return -2;
    }

    return 0;
}


void Timeout::timer_callback(TimerHandle_t xTimer){
    Timeout* instance = static_cast<Timeout*>(pvTimerGetTimerID(xTimer));
    if(instance && instance->callback){
        instance->callback();
    }
}
/*
 * DigitalOut.cpp
 *
 *  Created on: Sep 21, 2025
 *      Author: rrx7xw
 */
#include "DigitalOut.h"
#include "peripheralmap.h"
#include "stm32h7xx_hal.h"
#include "log.h"

// -------- Constructors --------

// Default: active-high, no pull
DigitalOut::DigitalOut(Pin pin)
    : pin_(pin), active_high_(true), initialized_(false)
{
    if (pin_.block == NULL) {
        log_warn("DigitalOut init failed: pin is NC");
        return;
    }

    gpio_clock_enable(pin_.block);
    configure_pin();
    initialized_ = true;
}

// Full constructor
DigitalOut::DigitalOut(Pin pin, bool active_high)
    : pin_(pin), active_high_(active_high), initialized_(false)
{
    if (pin_.block == NULL) {
        log_warn("DigitalOut init failed: pin is NC");
        return;
    }

    gpio_clock_enable(pin_.block);
    configure_pin();
    initialized_ = true;
}

// -------- Public Methods --------

// Write logical value to the pin
void DigitalOut::write(bool val) {
    if (!initialized_) {
        log_warn("DigitalOut write failed: DigitalOut not initialized");
        return;
    }

    GPIO_PinState state;

    // If active-high, val=true → SET, false → RESET
    // If active-low, val=true → RESET, false → SET
    if (active_high_) {
        state = val ? GPIO_PIN_SET : GPIO_PIN_RESET;
    } else {
        state = val ? GPIO_PIN_RESET : GPIO_PIN_SET;
    }

    HAL_GPIO_WritePin(pin_.block, pin_.block_mask, state);
}

// Read logical value from the pin
bool DigitalOut::read() {
    if (!initialized_) {
        log_warn("DigitalOut read failed: DigitalOut not initialized");
        return false;
    }

    GPIO_PinState state = HAL_GPIO_ReadPin(pin_.block, pin_.block_mask);

    // Return logical interpretation based on active_high_
    return active_high_ ? (state == GPIO_PIN_SET)
                        : (state == GPIO_PIN_RESET);
}



// -------- Private Helper --------

void DigitalOut::configure_pin() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin_.block_mask;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // push-pull
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // adjust if needed

    // Set pull configuration
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(pin_.block, &GPIO_InitStruct);
}





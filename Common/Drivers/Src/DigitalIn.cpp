/*
 * DigitalIn.cpp
 *
 *  Created on: Sep 21, 2025
 *      Author: rrx7xw
 */

#include "DigitalIn.h"
#include "stm32h7xx_hal.h"
#include "peripheralmap.h"

// -------- Constructors --------

// Default: active-high, no pull
DigitalIn::DigitalIn(Pin pin)
    : pin_(pin), active_high_(true), pull_(Pull::None)
{
    gpio_clock_enable(pin_.block);
    configure_pin();
}

// Full constructor
DigitalIn::DigitalIn(Pin pin, bool active_high, Pull pull)
    : pin_(pin), active_high_(active_high), pull_(pull)
{
    gpio_clock_enable(pin_.block);
    configure_pin();
}

// -------- Public Methods --------

bool DigitalIn::read() {
    GPIO_PinState state = HAL_GPIO_ReadPin(pin_.block, pin_.block_mask);

    // If active_high_ is true → logic 1 = pin high
    // If active_high_ is false → logic 1 = pin low
    return active_high_ ? (state == GPIO_PIN_SET)
                        : (state == GPIO_PIN_RESET);
}

// -------- Private Helper --------

void DigitalIn::configure_pin() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin_.block_mask;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Configure pin as input

    // Set pull configuration
    switch (pull_) {
        case Pull::Up:
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            break;
        case Pull::Down:
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
            break;
        case Pull::None:
        default:
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            break;
    }

    HAL_GPIO_Init(pin_.block, &GPIO_InitStruct);
}




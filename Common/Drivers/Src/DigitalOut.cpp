/*
 * DigitalOut.cpp
 *
 *  Created on: Sep 21, 2025
 *      Author: rrx7xw
 */
#include "../Inc/DigitalOut.h"

#include "stm32h7xx_hal.h"

// -------- Constructors --------

// Default: active-high, no pull
DigitalOut::DigitalOut(Pin pin)
    : pin_(pin), active_high_(true)
{
    configure_pin();
}

// Full constructor
DigitalOut::DigitalOut(Pin pin, bool active_high)
    : pin_(pin), active_high_(active_high)
{
    configure_pin();
}

// -------- Public Methods --------

// Write logical value to the pin
void DigitalOut::write(bool val) {
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
    GPIO_PinState state = HAL_GPIO_ReadPin(pin_.block, pin_.block_mask);

    // Return logical interpretation based on active_high_
    return active_high_ ? (state == GPIO_PIN_SET)
                        : (state == GPIO_PIN_RESET);
}



// -------- Private Helper --------

void DigitalOut::configure_pin() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO clock (basic version; expand for all ports)
    if (pin_.block == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (pin_.block == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (pin_.block == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (pin_.block == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (pin_.block == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (pin_.block == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (pin_.block == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
    else if (pin_.block == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
    else if (pin_.block == GPIOI) __HAL_RCC_GPIOI_CLK_ENABLE();
    // Add GPIOJ/K if your STM32 variant has them

    // Configure pin as output
    GPIO_InitStruct.Pin = pin_.block_mask;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // push-pull
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // adjust if needed

    // Set pull configuration
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(pin_.block, &GPIO_InitStruct);
}





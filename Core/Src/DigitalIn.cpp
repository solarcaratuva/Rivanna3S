/*
 * DigitalIn.cpp
 *
 *  Created on: Sep 21, 2025
 *      Author: rrx7xw
 */

#include "../Inc/DigitalIn.h"

#include "stm32h7xx_hal.h"

// -------- Constructors --------

// Default: active-high, no pull
DigitalIn::DigitalIn(Pin pin)
    : pin_(pin), active_high_(true), pull_(Pull::None)
{
    configure_pin();
}

// Full constructor
DigitalIn::DigitalIn(Pin pin, bool active_high, Pull pull)
    : pin_(pin), active_high_(active_high), pull_(pull)
{
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

    // Enable GPIO clock (basic version, can be expanded to support all ports)
    if (pin_.block == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (pin_.block == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (pin_.block == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (pin_.block == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (pin_.block == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (pin_.block == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (pin_.block == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
    else if (pin_.block == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
    else if (pin_.block == GPIOI) __HAL_RCC_GPIOI_CLK_ENABLE();
    // (extend for J, K if your MCU has them)

    // Configure pin as input
    GPIO_InitStruct.Pin = pin_.block_mask;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

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




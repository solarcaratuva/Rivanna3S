/*
 * DigitalIn.h
 *
 *  Created on: Sep 22, 2025
 *      Author: rrx7xw
 */

#ifndef INC_DIGITALIN_H_
#define INC_DIGITALIN_H_


#include "pinmap.h"
#include "stm32h7xx_hal.h"

/**
 * @brief Pull resistor configuration for digital input pins
 */
enum class Pull {
    None = GPIO_NOPULL,   ///< No pull resistor
    Up   = GPIO_PULLUP,   ///< Pull-up resistor enabled
    Down = GPIO_PULLDOWN  ///< Pull-down resistor enabled
};

/**
 * @brief Digital input pin interface with configurable polarity and pull resistors
 * 
 * Provides a high-level interface for reading digital input pins.
 * Supports active-high/active-low logic and internal pull resistors.
 */
class DigitalIn {
public:
    /**
     * @brief Construct a DigitalIn with default settings
     * @param pin The pin to configure as digital input
     * 
     * Defaults: active-high logic, no pull resistor
     */
    explicit DigitalIn(Pin pin);

    /**
     * @brief Construct a DigitalIn with full configuration
     * @param pin The pin to configure as digital input
     * @param active_high If true, high voltage = logical true; if false, low voltage = logical true
     * @param pull Pull resistor configuration (None, Up, or Down)
     */
    DigitalIn(Pin pin, bool active_high, Pull pull);

    /**
     * @brief Read the logical state of the input pin
     * @return true if pin is logically high (accounting for active_high setting)
     * @return false if pin is logically low (accounting for active_high setting)
     */
    bool read();

private:
    Pin pin_;
    bool active_high_;
    Pull pull_;

    void configure_pin(); // internal helper to configure GPIO
};




#endif /* INC_DIGITALIN_H_ */

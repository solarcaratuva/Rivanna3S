/*
 * DigitalOut.h
 *
 *  Created on: Sep 22, 2025
 *      Author: rrx7xw
 */

#ifndef INC_DIGITALOUT_H_
#define INC_DIGITALOUT_H_

#include "pinmap.h"
#include "stm32h7xx_hal.h"

/**
 * @brief Digital output pin interface with configurable polarity
 * 
 * Provides a high-level interface for controlling digital output pins.
 * Supports active-high/active-low logic for flexible hardware interfacing.
 */
class DigitalOut {
public:
    /**
     * @brief Construct a DigitalOut with default active-high logic
     * @param pin The pin to configure as digital output
     */
    explicit DigitalOut(Pin pin);

    /**
     * @brief Construct a DigitalOut with specified polarity
     * @param pin The pin to configure as digital output
     * @param active_high If true, logical high outputs high voltage; if false, logical high outputs low voltage
     */
    DigitalOut(Pin pin, bool active_high);

    /**
     * @brief Read back the current logical state of the output pin
     * @return true if pin is logically high (accounting for active_high setting)
     * @return false if pin is logically low (accounting for active_high setting)
     */
    bool read();

    /**
     * @brief Set the logical state of the output pin
     * @param val Logical value to write (true = logical high, false = logical low)
     * 
     * The actual voltage output depends on the active_high setting.
     */
    void write(bool val);

private:
    Pin pin_;
    bool active_high_;

    void configure_pin(); // internal helper to configure GPIO
};




#endif /* INC_DIGITALOUT_H_ */

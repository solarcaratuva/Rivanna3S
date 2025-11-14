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

enum class Pull {
    None = GPIO_NOPULL,
    Up   = GPIO_PULLUP,
    Down = GPIO_PULLDOWN
};

class DigitalIn {
public:
    // Construct with just a Pin (default: active-high, no pull)
    explicit DigitalIn(Pin pin);

    // Construct with Pin, active_high flag, and pull config
    DigitalIn(Pin pin, bool active_high, Pull pull);

    // Read the logical value of the pin
    bool read();

private:
    Pin pin_;
    bool active_high_;
    Pull pull_;

    void configure_pin(); // internal helper to configure GPIO
};




#endif /* INC_DIGITALIN_H_ */

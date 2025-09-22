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

class DigitalOut {
public:
    // Construct with just a Pin (default: active-high, no pull)
    explicit DigitalOut(Pin pin);

    // Construct with Pin, active_high flag, and pull config
    DigitalOut(Pin pin, bool active_high);

    // Read the logical value of the pin
    bool read();

    // Write a logical value to the pin
    void write(bool val);

private:
    Pin pin_;
    bool active_high_;

    void configure_pin(); // internal helper to configure GPIO
};




#endif /* INC_DIGITALOUT_H_ */

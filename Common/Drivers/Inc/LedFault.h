#ifndef LED_FAULT
#define LED_FAULT
#include <cstdint>

#include "stm32h743xx.h"

namespace LedFault {
    // Fault Blinking:
    // Bits 0 through 3 indicate how many blinks the "slow" LED should do (i.e. the LED we will blink slow)
    // Bits 4 through 7 will indicate how many blinks the "fast" LED should do
    enum Faults {
        RTOSAssert     = 0b00010001, // blink slow led once then fast led once
        // TODO: ummm... add the actual errors
    };

    void fault_signal_blink(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t pattern);
};

#endif
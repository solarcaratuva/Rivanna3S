#ifndef LED_FAULT
#define LED_FAULT
#include <stdint.h>
#include "stm32_hal.h"

// Fault Blinking:
// Bits 0 through 3 indicate how many blinks the "slow" LED should do (i.e. the LED we will blink slow)
// Bits 4 through 7 will indicate how many blinks the "fast" LED should do
enum LedFault_Faults {
    RTOSAssert     = 0b00010001, // blink slow led once then fast led once
    // TODO: ummm... add the actual errors
};

#define SLOW_BLINK_TIME 500 // ms
#define FAST_BLINK_TIME 200 // ms

void LedFault_fault_signal_blink(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t pattern);
#endif
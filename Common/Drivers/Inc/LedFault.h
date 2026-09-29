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

static void LedFault_fault_signal_blink(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t pattern) {
    const uint8_t slow_blinks = pattern & 0x0FU;
    const uint8_t fast_blinks = (pattern >> 4U) & 0x0FU;

    for (;;) {
        for (int b = 0; b < slow_blinks; b++) {
            HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
            HAL_Delay(SLOW_BLINK_TIME);
            HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
            HAL_Delay(SLOW_BLINK_TIME);
        }

        for (int b = 0; b < fast_blinks; b++) {
            HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
            HAL_Delay(FAST_BLINK_TIME);
            HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
            HAL_Delay(FAST_BLINK_TIME);
        }
    }
}
#endif
//
// Created by Charlotte on 9/29/26.
//
#include "LedFault.h"
void LedFault_fault_signal_blink(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t pattern){
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
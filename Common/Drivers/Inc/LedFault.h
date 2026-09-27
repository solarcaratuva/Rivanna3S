#ifndef LED_FAULT
#define LED_FAULT
#include <cstdint>

#include "stm32h743xx.h"

namespace LedFault {
    enum Faults {
        HardFault = 1,
        StackOverflow = 2,
        // TODO: ummm... add the actual errors
    };

    void fault_signal_blink(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t pattern);
};

#endif
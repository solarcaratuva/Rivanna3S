#ifndef STM32_HAL_H
#define STM32_HAL_H

// Select the correct HAL header based on the active device define.
#if defined(STM32H743xx)
  #include "stm32h7xx_hal.h"
#elif defined(STM32G474xx)
  #include "stm32g4xx_hal.h"
#elif defined(STM32U5A9xx)
  #include "stm32u5xx_hal.h"
#else
  #error "Unknown STM32 device: define STM32H743xx, STM32G474xx, or STM32U5A9xx."
#endif

#endif /* STM32_HAL_H */

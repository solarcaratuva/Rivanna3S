#include <LedFault.h>

namespace {
constexpr uint32_t SLOW_BLINK_MS = 500;
constexpr uint32_t FAST_BLINK_MS = 150;
constexpr uint32_t GROUP_GAP_MS = 500;
constexpr uint32_t PATTERN_GAP_MS = 1000;

void enable_gpio_clock(GPIO_TypeDef *GPIOx) {
#if defined(GPIOA)
    if (GPIOx == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); return; }
#endif
#if defined(GPIOB)
    if (GPIOx == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); return; }
#endif
#if defined(GPIOC)
    if (GPIOx == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); return; }
#endif
#if defined(GPIOD)
    if (GPIOx == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); return; }
#endif
#if defined(GPIOE)
    if (GPIOx == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); return; }
#endif
#if defined(GPIOF)
    if (GPIOx == GPIOF) { __HAL_RCC_GPIOF_CLK_ENABLE(); return; }
#endif
#if defined(GPIOG)
    if (GPIOx == GPIOG) { __HAL_RCC_GPIOG_CLK_ENABLE(); return; }
#endif
#if defined(GPIOH)
    if (GPIOx == GPIOH) { __HAL_RCC_GPIOH_CLK_ENABLE(); return; }
#endif
#if defined(GPIOI)
    if (GPIOx == GPIOI) { __HAL_RCC_GPIOI_CLK_ENABLE(); return; }
#endif
#if defined(GPIOJ)
    if (GPIOx == GPIOJ) { __HAL_RCC_GPIOJ_CLK_ENABLE(); return; }
#endif
#if defined(GPIOK)
    if (GPIOx == GPIOK) { __HAL_RCC_GPIOK_CLK_ENABLE(); }
#endif
}

void blink(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t count,
           uint32_t interval_ms) {
    for (uint8_t i = 0; i < count; ++i) {
        // Set the LED before delaying so a broken HAL tick leaves it visible.
        GPIOx->BSRR = GPIO_Pin;
        HAL_Delay(interval_ms);
        GPIOx->BSRR = static_cast<uint32_t>(GPIO_Pin) << 16U;
        HAL_Delay(interval_ms);
    }
}
} // namespace


void LedFault_fault_signal_blink(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t pattern) {
    // Only a single GPIO pin is supported by this interface.
    if (GPIOx == nullptr || GPIO_Pin == 0U ||
        (GPIO_Pin & static_cast<uint16_t>(GPIO_Pin - 1U)) != 0U) {
        return;
    }

    enable_gpio_clock(GPIOx);

    uint32_t pin_number = 0U;
    while ((GPIO_Pin & (1U << pin_number)) == 0U) {
        ++pin_number;
    }

    // Configure the pin as a push-pull output without relying on DigitalOut or
    // another higher-level driver. OTYPER reset means push-pull.
    GPIOx->MODER = (GPIOx->MODER & ~(3UL << (pin_number * 2U))) |
                   (1UL << (pin_number * 2U));
    GPIOx->OTYPER &= ~static_cast<uint32_t>(GPIO_Pin);

    const uint8_t slow_blinks = pattern & 0x0FU;
    const uint8_t fast_blinks = (pattern >> 4U) & 0x0FU;

    for (;;) {
        blink(GPIOx, GPIO_Pin, slow_blinks, SLOW_BLINK_MS);

        if (slow_blinks != 0U && fast_blinks != 0U) {
            HAL_Delay(GROUP_GAP_MS);
        }

        blink(GPIOx, GPIO_Pin, fast_blinks, FAST_BLINK_MS);
        HAL_Delay(PATTERN_GAP_MS);
    }
}

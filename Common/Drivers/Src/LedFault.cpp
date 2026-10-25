#include "LedFault.h"
#include "stm32_hal.h"
#include "FreeRTOS.h"
#include "task.h"

namespace {
// Busy-wait delay (~4 CPU cycles per loop) since SysTick/RTOS interrupts are disabled.
void spin_ms(uint32_t ms)
{
    uint32_t loops = (SystemCoreClock / 4000U) * ms;
    while (loops-- != 0U) { __NOP(); }
}
} // namespace

void LedFault_halt(enum LedFault_Faults fault)
{
    __disable_irq();

    // Configure PC1 (DEBUG_OUT LED on all car boards) as push-pull output.
    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
    GPIO_InitTypeDef led = {};
    led.Pin = GPIO_PIN_1;
    led.Mode = GPIO_MODE_OUTPUT_PP;
    led.Pull = GPIO_NOPULL;
    led.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &led);

    // Repeatedly blink the fault count on PC1, pausing between cycles.
    while (true) {
        for (unsigned pulse = 0; pulse < static_cast<unsigned>(fault); ++pulse) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
            spin_ms(200U);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
            spin_ms(200U);
        }
        spin_ms(1500U);
    }
}

// FreeRTOS hooks for heap allocation failures and task stack overflows.
extern "C" void vApplicationMallocFailedHook(void)
{
    LedFault_halt(LedFault_MallocFailed);
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t, char *)
{
    LedFault_halt(LedFault_StackOverflow);
}

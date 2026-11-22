#include "LedFault.h"
#include "stm32_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#if defined(NUCLEO_G474RE)
// NUCLEO-G474RE user LED LD2.
#define DEBUG_LED_PORT          GPIOA
#define DEBUG_LED_PIN           GPIO_PIN_5
#define DEBUG_LED_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#else
// PC1 matches the car board's debug LED.
#define DEBUG_LED_PORT          GPIOC
#define DEBUG_LED_PIN           GPIO_PIN_1
#define DEBUG_LED_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()
#endif

namespace {

// Busy-wait delay; SysTick and the RTOS tick are disabled here. The divisor
// assumes the ~4-cycle -O2 loop, so the -O0 Debug build (~12 cycles) runs
// every interval ~3x longer than nominal. Still readable by eye.
void spin_ms(uint32_t ms)
{
    uint32_t loops = (SystemCoreClock / 4000U) * ms;
    while (loops-- != 0U) { __NOP(); }
}

} // namespace

__attribute__((noreturn)) void LedFault_halt(enum LedFault_Faults fault)
{
    __disable_irq();

    DEBUG_LED_CLK_ENABLE();

    // Set the level before switching to output, to avoid a glitch.
    HAL_GPIO_WritePin(DEBUG_LED_PORT, DEBUG_LED_PIN, GPIO_PIN_SET);

    GPIO_InitTypeDef led = {};
    led.Pin = DEBUG_LED_PIN;
    led.Mode = GPIO_MODE_OUTPUT_PP;
    led.Pull = GPIO_NOPULL;
    led.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DEBUG_LED_PORT, &led);

    // Repeatedly blink the fault count, pausing between cycles.
    while (true) {
        for (unsigned pulse = 0; pulse < static_cast<unsigned>(fault); ++pulse) {
            HAL_GPIO_WritePin(DEBUG_LED_PORT, DEBUG_LED_PIN, GPIO_PIN_SET);
            spin_ms(200U);
            HAL_GPIO_WritePin(DEBUG_LED_PORT, DEBUG_LED_PIN, GPIO_PIN_RESET);
            spin_ms(200U);
        }
        spin_ms(1500U);
    }
}

__attribute__((noreturn)) void LedFault_halt_assert(void)
{
    LedFault_halt(LedFault_RTOSAssert);
}

// FreeRTOS hooks for heap allocation failures and task stack overflows.
extern "C" __attribute__((noreturn)) void vApplicationMallocFailedHook(void)
{
    LedFault_halt(LedFault_MallocFailed);
}

extern "C" __attribute__((noreturn)) void vApplicationStackOverflowHook(TaskHandle_t, char *)
{
    LedFault_halt(LedFault_StackOverflow);
}

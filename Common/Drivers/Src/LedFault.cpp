#include "LedFault.h"
#include "stm32_hal.h"
#include "FreeRTOS.h"
#include "task.h"

// Debug LED. PC1 matches MotorBoard's DEBUG_OUT (G474 build) and LED1 in
// log.cpp; unconfirmed on the other boards.
#define DEBUG_LED_PORT          GPIOC
#define DEBUG_LED_PIN           GPIO_PIN_1
#define DEBUG_LED_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()

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

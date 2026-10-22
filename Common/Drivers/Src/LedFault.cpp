#include "LedFault.h"
#include "stm32_hal.h"
#include "FreeRTOS.h"
#include "task.h"

namespace {
void spin_ms(uint32_t ms)
{
    // Approximate timing: faults cannot depend on interrupts or the RTOS tick.
    uint32_t loops = (SystemCoreClock / 4000U) * ms;
    while (loops-- != 0U) { __NOP(); }
}
} // namespace

void LedFault_halt(enum LedFault_Faults fault)
{
    __disable_irq();

    // PC1 is the debug output LED on every car board.
    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
    GPIO_InitTypeDef led = {};
    led.Pin = GPIO_PIN_1;
    led.Mode = GPIO_MODE_OUTPUT_PP;
    led.Pull = GPIO_NOPULL;
    led.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &led);

    while (true) {
        for (unsigned pulse = 0; pulse < static_cast<unsigned>(fault); ++pulse) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
            spin_ms(500U);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
            spin_ms(500U);
        }
        spin_ms(1500U);
    }
}

extern "C" void vApplicationMallocFailedHook(void)
{
    LedFault_halt(LedFault_MallocFailed);
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    (void)task_name;
    LedFault_halt(LedFault_StackOverflow);
}

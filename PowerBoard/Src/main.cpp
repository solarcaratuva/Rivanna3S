#include "blink.h"
#include "DigitalOut.h"
#include "pinmap.h"
#include "stm32h7xx_hal.h"
#include "cmsis_gcc.h"

namespace
{
// Crude delay loop so we can see the LED toggle without FreeRTOS/HAL tick support.
void busy_delay(volatile uint32_t cycles)
{
    while (cycles--)
    {
        __NOP();
    }
}
} // namespace

int main()
{
    HAL_Init();

    // PB_0 is brought out on the Nucleo board’s user LED (adjust as needed).
    DigitalOut led(PB_0);

    // for (;;)
    // {
    //     flashPin(led);
    //     busy_delay(2000000U);
    // }
    led.write(true);
    while (1)
    {
    }   
}
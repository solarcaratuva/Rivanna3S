#include "PeripheralMap.h"
#include "stm32h7xx_hal.h"

UART_Peripheral UART_Peripherals[] = {
    {USART2, PA_3.universal_mask, PA_2.universal_mask, NC, NC, GPIO_AF7_USART2, false},
    {UART4, PA_1.universal_mask, PA_0.universal_mask, NC, NC, GPIO_AF8_UART4, false},
    {UART7, (PF_6.universal_mask | PE_7.universal_mask), (PF_7.universal_mask | PE_8.universal_mask), NC, NC, GPIO_AF7_UART7, false}
};

const uint8_t UART_PERIPHERAL_COUNT = sizeof(UART_Peripherals) / sizeof(UART_Peripherals[0]);

I2C_Peripheral I2C_Peripherals[] = {
    {I2C2, PF_0.universal_mask, PF_1.universal_mask, NC, NC, false},
    {I2C4, PF_15.universal_mask, PF_14.universal_mask, NC, NC, false}
};

const uint8_t I2C_PERIPHERAL_COUNT = sizeof(I2C_Peripherals) / sizeof(I2C_Peripherals[0]);

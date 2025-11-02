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



ADC_Peripheral ADC_Peripherals[] = {
    // ---- ADC1 ----
    { ADC1, ADC_CHANNEL_0,  PA_0.universal_mask, NC, false},  // INP0  → PA0_C
    { ADC1, ADC_CHANNEL_1,  PA_1.universal_mask, NC, false },  // INP1  → PA1_C
    { ADC1, ADC_CHANNEL_2,  PF_11.universal_mask, NC, false }, // INP2  → PF11
    { ADC1, ADC_CHANNEL_3,  PA_6.universal_mask, NC, false },  // INP3  → PA6
    { ADC1, ADC_CHANNEL_4,  PC_4.universal_mask, NC, false },  // INP4  → PC4

    // ---- ADC2 ----
    { ADC2, ADC_CHANNEL_0,  PA_0.universal_mask, NC, false },  // INP0  → PA0_C
    { ADC2, ADC_CHANNEL_1,  PA_1.universal_mask, NC, false},  // INP1  → PA1_C
    { ADC2, ADC_CHANNEL_2,  PF_13.universal_mask, NC, false}, // INP2  → PF13
    { ADC2, ADC_CHANNEL_3,  PA_6.universal_mask, NC, false },  // INP3  → PA6
    { ADC2, ADC_CHANNEL_4,  PC_4.universal_mask, NC, false },  // INP4  → PC4
};

const uint8_t ADC_PERIPHERAL_COUNT = sizeof(ADC_Peripherals) / sizeof(ADC_Peripherals[0]);


const uint8_t ADC_PERIPHERAL_COUNT = sizeof(ADC_Peripherals) / sizeof(ADC_Peripherals[0]);
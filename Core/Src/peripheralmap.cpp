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
    { ADC1, ADC_CHANNEL_16,  PA_0.universal_mask, NC, false},  // INP0  → PA0_C
	{ ADC3, ADC_CHANNEL_6, PF_10.universal_mask, NC, false},
//    { ADC1, ADC_CHANNEL_1,  PA_1.universal_mask, NC, false },  // INP1  → PA1_C
//    { ADC1, ADC_CHANNEL_2,  PF_11.universal_mask, NC, false }, // INP2  → PF11
//    { ADC1, ADC_CHANNEL_3,  PA_6.universal_mask, NC, false },  // INP3  → PA6
//    { ADC1, ADC_CHANNEL_4,  PC_4.universal_mask, NC, false },  // INP4  → PC4
//
//    // ---- ADC2 ----
//    { ADC2, ADC_CHANNEL_0,  PA_0.universal_mask, NC, false },  // INP0  → PA0_C
//    { ADC2, ADC_CHANNEL_1,  PA_1.universal_mask, NC, false},  // INP1  → PA1_C
//    { ADC2, ADC_CHANNEL_2,  PF_13.universal_mask, NC, false}, // INP2  → PF13
//    { ADC2, ADC_CHANNEL_3,  PA_6.universal_mask, NC, false },  // INP3  → PA6
//    { ADC2, ADC_CHANNEL_4,  PC_4.universal_mask, NC, false },  // INP4  → PC4
};

const uint8_t ADC_PERIPHERAL_COUNT = sizeof(ADC_Peripherals) / sizeof(ADC_Peripherals[0]);


void uart_clock_enable(USART_TypeDef* handle){
    if (handle == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
    }
    else if (handle == UART4)
    {
        __HAL_RCC_UART4_CLK_ENABLE();
    }
    else if (handle == UART7)
    {
        __HAL_RCC_UART7_CLK_ENABLE();
    }
    else if (handle == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
    }
    return;
}

void gpio_clock_enable(GPIO_TypeDef* port) {
	if (port == GPIOA)
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();
	}
	else if (port == GPIOB)
	{
	    __HAL_RCC_GPIOB_CLK_ENABLE();
	}
	else if (port == GPIOC)
	{
	    __HAL_RCC_GPIOC_CLK_ENABLE();
	}
	else if (port == GPIOD)
	{
	    __HAL_RCC_GPIOD_CLK_ENABLE();
	}
	else if (port == GPIOE)
	{
	    __HAL_RCC_GPIOE_CLK_ENABLE();
	}
	else if (port == GPIOF)
	{
	    __HAL_RCC_GPIOF_CLK_ENABLE();
	}
	else if (port == GPIOG)
	{
	    __HAL_RCC_GPIOG_CLK_ENABLE();
	}
	else if (port == GPIOH)
	{
	    __HAL_RCC_GPIOH_CLK_ENABLE();
	}
	else if (port == GPIOI)
	{
	    __HAL_RCC_GPIOI_CLK_ENABLE();
	}
	#if defined(GPIOJ)
	    else if (port == GPIOJ)
	    {
	        __HAL_RCC_GPIOJ_CLK_ENABLE();
	    }
	#endif //GPIOJ
	#if defined(GPIOK)
	    else if (port == GPIOK)
	    {
	        __HAL_RCC_GPIOK_CLK_ENABLE();
	    }
	#endif //GPIOK
}


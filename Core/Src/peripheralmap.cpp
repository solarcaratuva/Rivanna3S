#include "PeripheralMap.h"
#include "stm32h7xx_hal.h"

UART_Peripheral UART_Peripherals[] = {
    {USART2, PA_3.universal_mask, PA_2.universal_mask, NC, NC, GPIO_AF7_USART2, false},
    {UART4, PA_1.universal_mask, PA_0.universal_mask, NC, NC, GPIO_AF8_UART4, false},
    {UART7, (PF_6.universal_mask | PE_7.universal_mask), (PF_7.universal_mask | PE_8.universal_mask), NC, NC, GPIO_AF7_UART7, false},
	{USART3, PD_9.universal_mask, PD_8.universal_mask, NC, NC, GPIO_AF7_USART3, false}
};

const uint8_t UART_PERIPHERAL_COUNT = sizeof(UART_Peripherals) / sizeof(UART_Peripherals[0]);

I2C_Peripheral I2C_Peripherals[] = {
    {I2C2, PF_0.universal_mask, PF_1.universal_mask, NC, NC, false},
    {I2C4, PF_15.universal_mask, PF_14.universal_mask, NC, NC, false}
};

const uint8_t I2C_PERIPHERAL_COUNT = sizeof(I2C_Peripherals) / sizeof(I2C_Peripherals[0]);

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


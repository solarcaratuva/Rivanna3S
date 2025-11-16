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



ADC_Peripheral ADC_Peripherals[] = {
    { ADC1, ADC_CHANNEL_16,  PA_0.universal_mask, NC, false, 0},
	{ ADC3, ADC_CHANNEL_6, PF_10.universal_mask, NC, false, 2},
	{ ADC3, ADC_CHANNEL_5, PF_4.universal_mask, NC, false, 2},
	{ ADC3, ADC_CHANNEL_4, PF_5.universal_mask, NC, false, 2},
	//for ranking ADC1 --> 0, ADC2 --> 1 etc.
};

const uint8_t ADC_PERIPHERAL_COUNT = sizeof(ADC_Peripherals) / sizeof(ADC_Peripherals[0]);

uint8_t adc_channels_claimed[] = {0, 0, 0};

uint32_t adc_get_rank(ADC_Peripheral* peripheral) {
	uint8_t index = peripheral->instance_num;

	uint32_t rank;
	switch (adc_channels_claimed[index]) {
		case 0: rank = ADC_REGULAR_RANK_1; break;
		case 1: rank = ADC_REGULAR_RANK_2; break;
		case 2: rank = ADC_REGULAR_RANK_3; break;
		case 3: rank = ADC_REGULAR_RANK_4; break;
		case 4: rank = ADC_REGULAR_RANK_5; break;
		case 5: rank = ADC_REGULAR_RANK_6; break;
		case 6: rank = ADC_REGULAR_RANK_7; break;
		case 7: rank = ADC_REGULAR_RANK_8; break;
		case 8: rank = ADC_REGULAR_RANK_9; break;
		case 9: rank = ADC_REGULAR_RANK_10; break;
		default: rank = ADC_REGULAR_RANK_1; break;
	}

	adc_channels_claimed[index] += 1;
	return rank;
}


void uart_clock_enable(USART_TypeDef* handle){
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (handle == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
    }
    else if (handle == UART4)
    {
    	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4;
    	PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
        __HAL_RCC_UART4_CLK_ENABLE();
    }
    else if (handle == UART7)
    {
    	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART7;
    	PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
        __HAL_RCC_UART7_CLK_ENABLE();
    }
    else if (handle == USART1)
    {
    	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    	PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
        __HAL_RCC_USART1_CLK_ENABLE();
    }
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
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


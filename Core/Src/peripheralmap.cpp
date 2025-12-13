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
    {I2C2, PF_0.universal_mask, PF_1.universal_mask, NC, NC, GPIO_AF4_I2C2, false},
    {I2C4, PF_15.universal_mask, PF_14.universal_mask, NC, NC, GPIO_AF4_I2C4, false}
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

uint8_t get_i2c_af(I2C_TypeDef* handle, Pin pin, uint8_t mode) {
	af_info i2c2_sda[] =
	{
		{PF_0, GPIO_AF4_I2C2}
	};

	uint8_t i2c2_sda_len = 2;

	af_info i2c2_scl[] =
	{
		{PF_1, GPIO_AF4_I2C2}
	};

	uint8_t i2c2_scl_len = 1;

	af_info i2c4_sda[] =
	{
		{PF_15, GPIO_AF4_I2C2}
	};

	uint8_t i2c4_sda_len = 1;

	af_info i2c4_scl[] =
	{
		{PF_14, GPIO_AF4_I2C2}
	};

	uint8_t i2c4_scl_len = 1;

	af_info *array;
	uint8_t array_len;

	if (handle == I2C2) {
		if (mode == SDA) {
			array = i2c2_sda;
			array_len = i2c2_sda_len;
		}
		if (mode == SCL) {
			array = i2c2_scl;
			array_len = i2c2_scl_len;
		}
	}

	if (handle == I2C4) {
		if (mode == SDA) {
		    array = i2c4_sda;
		    array_len = i2c4_sda_len;
		}
		if (mode == SCL) {
			array = i2c4_scl;
			array_len = i2c4_scl_len;
		}
	}

	for (uint8_t i = 0; i < array_len; i++) {
	  if (array[i].pin.universal_mask == pin.universal_mask) {
	    return array[i].af;
	  }
	}





}


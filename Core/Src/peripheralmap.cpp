#include "peripheralmap.h"
#include "stm32h7xx_hal.h"

UART_Peripheral UART_Peripherals[] = {
	{UART4, (PA_1.universal_mask), (PA_0.universal_mask), NC, NC, false},
	{UART7, (PF_6.universal_mask | PE_7.universal_mask), (PF_7.universal_mask | PE_8.universal_mask), NC, NC, false},
	{USART2, (PA_3.universal_mask), (PA_2.universal_mask), NC, NC, false},
	{USART3, (PD_9.universal_mask), (PD_8.universal_mask), NC, NC, false},
};

const uint8_t UART_PERIPHERAL_COUNT = sizeof(UART_Peripherals) / sizeof(UART_Peripherals[0]);

I2C_Peripheral I2C_Peripherals[] = {
	{I2C2, (PF_0.universal_mask), (PF_1.universal_mask), NC, NC, false},
	{I2C4, (PF_15.universal_mask), (PF_14.universal_mask), NC, NC, false},
};

const uint8_t I2C_PERIPHERAL_COUNT = sizeof(I2C_Peripherals) / sizeof(I2C_Peripherals[0]);

FDCAN_Peripheral FDCAN_Peripherals[] = {
	{FDCAN1, PB_8.universal_mask, PB_9.universal_mask, NC, NC, false} // need to add more FDCAN peripherals if used and verify the pins
};

const uint8_t FDCAN_PERIPHERAL_COUNT = sizeof(FDCAN_Peripherals) / sizeof(FDCAN_Peripherals[0]);

ADC_Peripheral ADC_Peripherals[] = {
	{ADC1, ADC_CHANNEL_10, PC_0.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_11, PC_1.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_16, PA_0.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_17, PA_1.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_14, PA_2.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_15, PA_3.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_18, PA_4.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_19, PA_5.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_3, PA_6.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_7, PA_7.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_4, PC_4.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_8, PC_5.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_9, PB_0.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_5, PB_1.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_2, PF_11.universal_mask, NC, false, 0},
	{ADC1, ADC_CHANNEL_6, PF_12.universal_mask, NC, false, 0},
	{ADC2, ADC_CHANNEL_10, PC_0.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_11, PC_1.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_14, PA_2.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_15, PA_3.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_18, PA_4.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_19, PA_5.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_3, PA_6.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_7, PA_7.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_4, PC_4.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_8, PC_5.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_9, PB_0.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_5, PB_1.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_2, PF_13.universal_mask, NC, false, 1},
	{ADC2, ADC_CHANNEL_6, PF_14.universal_mask, NC, false, 1},
	{ADC3, ADC_CHANNEL_5, PF_3.universal_mask, NC, false, 2},
	{ADC3, ADC_CHANNEL_9, PF_4.universal_mask, NC, false, 2},
	{ADC3, ADC_CHANNEL_4, PF_5.universal_mask, NC, false, 2},
	{ADC3, ADC_CHANNEL_8, PF_6.universal_mask, NC, false, 2},
	{ADC3, ADC_CHANNEL_3, PF_7.universal_mask, NC, false, 2},
	{ADC3, ADC_CHANNEL_7, PF_8.universal_mask, NC, false, 2},
	{ADC3, ADC_CHANNEL_2, PF_9.universal_mask, NC, false, 2},
	{ADC3, ADC_CHANNEL_6, PF_10.universal_mask, NC, false, 2},
	{ADC3, ADC_CHANNEL_10, PC_0.universal_mask, NC, false, 2},
	{ADC3, ADC_CHANNEL_11, PC_1.universal_mask, NC, false, 2},
};

const uint8_t ADC_PERIPHERAL_COUNT = sizeof(ADC_Peripherals) / sizeof(ADC_Peripherals[0]);

uint8_t adc_channels_claimed[] = {0, 0, 0};

void gpio_clock_enable(const GPIO_TypeDef *handle)
{
	if (handle == GPIOC)
	{
		__HAL_RCC_GPIOC_CLK_ENABLE();
	}
	else if (handle == GPIOF)
	{
		__HAL_RCC_GPIOF_CLK_ENABLE();
	}
	else if (handle == GPIOE)
	{
		__HAL_RCC_GPIOE_CLK_ENABLE();
	}
	else if (handle == GPIOB)
	{
		__HAL_RCC_GPIOB_CLK_ENABLE();
	}
	else if (handle == GPIOA)
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();
	}
	else if (handle == GPIOG)
	{
		__HAL_RCC_GPIOG_CLK_ENABLE();
	}
	else if (handle == GPIOD)
	{
		__HAL_RCC_GPIOD_CLK_ENABLE();
	}
	return;
}

uint8_t get_UART_AF(const USART_TypeDef *handle, const Pin *pin, uint8_t mode)
{
	static AF_Info UART4_TX[] = {
		{PA_0, GPIO_AF8_UART4},
	};
	static uint8_t UART4_TX_len = 1;

	static AF_Info UART4_RX[] = {
		{PA_1, GPIO_AF8_UART4},
	};
	static uint8_t UART4_RX_len = 1;

	static AF_Info UART7_RX[] = {
		{PF_6, GPIO_AF7_UART7},
		{PE_7, GPIO_AF7_UART7},
	};
	static uint8_t UART7_RX_len = 2;

	static AF_Info UART7_TX[] = {
		{PF_7, GPIO_AF7_UART7},
		{PE_8, GPIO_AF7_UART7},
	};
	static uint8_t UART7_TX_len = 2;

	static AF_Info USART2_TX[] = {
		{PA_2, GPIO_AF7_USART2},
	};
	static uint8_t USART2_TX_len = 1;

	static AF_Info USART2_RX[] = {
		{PA_3, GPIO_AF7_USART2},
	};
	static uint8_t USART2_RX_len = 1;

	static AF_Info USART3_TX[] = {
		{PD_8, GPIO_AF7_USART3},
	};
	static uint8_t USART3_TX_len = 1;

	static AF_Info USART3_RX[] = {
		{PD_9, GPIO_AF7_USART3},
	};
	static uint8_t USART3_RX_len = 1;

	AF_Info *array = NULL;
	uint8_t array_len = 0;

	if (handle == UART4)
	{
		if (mode == RX)
		{
			array = UART4_RX;
			array_len = UART4_RX_len;
		}
		else if (mode == TX)
		{
			array = UART4_TX;
			array_len = UART4_TX_len;
		}
	}

	if (handle == UART7)
	{
		if (mode == RX)
		{
			array = UART7_RX;
			array_len = UART7_RX_len;
		}
		else if (mode == TX)
		{
			array = UART7_TX;
			array_len = UART7_TX_len;
		}
	}

	if (handle == USART2)
	{
		if (mode == RX)
		{
			array = USART2_RX;
			array_len = USART2_RX_len;
		}
		else if (mode == TX)
		{
			array = USART2_TX;
			array_len = USART2_TX_len;
		}
	}

	if (handle == USART3)
	{
		if (mode == RX)
		{
			array = USART3_RX;
			array_len = USART3_RX_len;
		}
		else if (mode == TX)
		{
			array = USART3_TX;
			array_len = USART3_TX_len;
		}
	}

	for (uint8_t i = 0; i < array_len; i++)
	{
		if (array[i].pin == *pin)
		{
			return array[i].AF;
		}
	}
	return 0; // should never happen
}

uint8_t get_I2C_AF(const I2C_TypeDef *handle, const Pin *pin, uint8_t mode)
{
	static AF_Info I2C2_SDA[] = {
		{PF_0, GPIO_AF4_I2C2},
	};
	static uint8_t I2C2_SDA_len = 1;

	static AF_Info I2C2_SCL[] = {
		{PF_1, GPIO_AF4_I2C2},
	};
	static uint8_t I2C2_SCL_len = 1;

	static AF_Info I2C4_SCL[] = {
		{PF_14, GPIO_AF4_I2C4},
	};
	static uint8_t I2C4_SCL_len = 1;

	static AF_Info I2C4_SDA[] = {
		{PF_15, GPIO_AF4_I2C4},
	};
	static uint8_t I2C4_SDA_len = 1;

	AF_Info *array = NULL;
	uint8_t array_len = 0;

	if (handle == I2C2)
	{
		if (mode == SDA)
		{
			array = I2C2_SDA;
			array_len = I2C2_SDA_len;
		}
		else if (mode == SCL)
		{
			array = I2C2_SCL;
			array_len = I2C2_SCL_len;
		}
	}

	if (handle == I2C4)
	{
		if (mode == SDA)
		{
			array = I2C4_SDA;
			array_len = I2C4_SDA_len;
		}
		else if (mode == SCL)
		{
			array = I2C4_SCL;
			array_len = I2C4_SCL_len;
		}
	}

	for (uint8_t i = 0; i < array_len; i++)
	{
		if (array[i].pin == *pin)
		{
			return array[i].AF;
		}
	}
	return 0; // should never happen
}

uint8_t get_FDCAN_AF(const FDCAN_GlobalTypeDef *handle, const Pin *pin, uint8_t mode)
{
	AF_Info *array = NULL;
	uint8_t array_len = 0;

	static AF_Info FDCAN1_RX[] = {
		{PB_8, GPIO_AF9_FDCAN1},
	};
	static uint8_t FDCAN1_RX_len = 1;

	static AF_Info FDCAN1_TX[] = {
		{PB_9, GPIO_AF9_FDCAN1},
	};
	static uint8_t FDCAN1_TX_len = 1;

	if (handle == FDCAN1)
	{
		if (mode == RX)
		{
			array = FDCAN1_RX;
			array_len = FDCAN1_RX_len;
		}
		else if (mode == TX)
		{
			array = FDCAN1_TX;
			array_len = FDCAN1_TX_len;
		}
	}

	for (uint8_t i = 0; i < array_len; i++)
	{
		if (array[i].pin == *pin)
		{
			return array[i].AF;
		}
	}
	return 0; // should never happen
}
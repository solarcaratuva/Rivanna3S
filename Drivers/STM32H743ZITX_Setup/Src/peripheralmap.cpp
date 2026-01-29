#include "peripheralmap.h"
#include "stm32h7xx_hal.h"

UART_Peripheral UART_Peripherals[] = {
    {UART4, (PA_1.universal_mask), (PA_0.universal_mask), NC, NC, false},
    {UART7, (PF_6.universal_mask | PE_7.universal_mask), (PF_7.universal_mask | PE_8.universal_mask), NC, NC, false},
    {USART2, (PA_3.universal_mask), (PA_2.universal_mask), NC, NC, false},
};

extern const uint8_t UART_PERIPHERAL_COUNT = sizeof(UART_Peripherals) / sizeof(UART_Peripherals[0]);

I2C_Peripheral I2C_Peripherals[] = {
    {I2C2, (PF_0.universal_mask), (PF_1.universal_mask), NC, NC, false},
    {I2C4, (PF_15.universal_mask), (PF_14.universal_mask), NC, NC, false},
};

extern const uint8_t I2C_PERIPHERAL_COUNT = sizeof(I2C_Peripherals) / sizeof(I2C_Peripherals[0]);

FDCAN_Peripheral FDCAN_Peripherals[] = {
};

extern const uint8_t FDCAN_PERIPHERAL_COUNT = sizeof(FDCAN_Peripherals) / sizeof(FDCAN_Peripherals[0]);

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

extern const uint8_t ADC_PERIPHERAL_COUNT = sizeof(ADC_Peripherals) / sizeof(ADC_Peripherals[0]);

uint8_t adc_channels_claimed[] = {0, 0, 0};

SPI_Peripheral SPI_Peripherals[] = {
    {SPI1, (PA_7.universal_mask), (PA_6.universal_mask), (PA_5.universal_mask), NC, NC, NC, false},
    {SPI4, (PE_6.universal_mask), (PE_5.universal_mask), (PE_2.universal_mask), NC, NC, NC, false},
    {SPI5, (PF_9.universal_mask | PF_11.universal_mask), (PF_8.universal_mask), (PF_7.universal_mask), NC, NC, NC, false},
    {SPI6, (PA_7.universal_mask), (PA_6.universal_mask), (PA_5.universal_mask), NC, NC, NC, false},
};

extern const uint8_t SPI_PERIPHERAL_COUNT = sizeof(SPI_Peripherals) / sizeof(SPI_Peripherals[0]);

void gpio_clock_enable(const GPIO_TypeDef* handle){
    if(handle == GPIOA){
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if(handle == GPIOC){
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if(handle == GPIOG){
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
    else if(handle == GPIOF){
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
    else if(handle == GPIOE){
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
    else if(handle == GPIOB){
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    return;
}

uint8_t get_UART_AF(const USART_TypeDef* handle, const Pin* pin, uint8_t mode) {
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

    AF_Info* array = NULL;
    uint8_t array_len = 0;


    if(handle == UART4) {
		if(mode == TX) {
            array = UART4_TX;
            array_len = UART4_TX_len;
        }
		if(mode == RX) {
            array = UART4_RX;
            array_len = UART4_RX_len;
        }
    }

    if(handle == UART7) {
		if(mode == RX) {
            array = UART7_RX;
            array_len = UART7_RX_len;
        }
		if(mode == TX) {
            array = UART7_TX;
            array_len = UART7_TX_len;
        }
    }

    if(handle == USART2) {
		if(mode == TX) {
            array = USART2_TX;
            array_len = USART2_TX_len;
        }
		if(mode == RX) {
            array = USART2_RX;
            array_len = USART2_RX_len;
        }
    }


    for (uint8_t i = 0; i < array_len; i++) {
        if(array[i].pin == *pin) {
            return array[i].AF;
        }
    }
    return 0; // should never happen
}

uint8_t get_I2C_AF(const I2C_TypeDef* handle, const Pin* pin, uint8_t mode) {
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

    AF_Info* array = NULL;
    uint8_t array_len = 0;


    if(handle == I2C2) {
		if(mode == SDA) {
            array = I2C2_SDA;
            array_len = I2C2_SDA_len;
        }
		if(mode == SCL) {
            array = I2C2_SCL;
            array_len = I2C2_SCL_len;
        }
    }

    if(handle == I2C4) {
		if(mode == SCL) {
            array = I2C4_SCL;
            array_len = I2C4_SCL_len;
        }
		if(mode == SDA) {
            array = I2C4_SDA;
            array_len = I2C4_SDA_len;
        }
    }


    for (uint8_t i = 0; i < array_len; i++) {
        if(array[i].pin == *pin) {
            return array[i].AF;
        }
    }
    return 0; // should never happen
}

uint8_t get_FDCAN_AF(const FDCAN_GlobalTypeDef* handle, const Pin* pin, uint8_t mode) {
    AF_Info* array = NULL;
    uint8_t array_len = 0;



    for (uint8_t i = 0; i < array_len; i++) {
        if(array[i].pin == *pin) {
            return array[i].AF;
        }
    }
    return 0; // should never happen
}

uint8_t get_SPI_AF(const SPI_TypeDef* handle, const Pin* pin, uint8_t mode) {
    static AF_Info SPI1_SCK[] = {
        {PA_5, GPIO_AF5_SPI1},
    };
    static uint8_t SPI1_SCK_len = 1;

    static AF_Info SPI1_MISO[] = {
        {PA_6, GPIO_AF5_SPI1},
    };
    static uint8_t SPI1_MISO_len = 1;

    static AF_Info SPI1_MOSI[] = {
        {PA_7, GPIO_AF5_SPI1},
    };
    static uint8_t SPI1_MOSI_len = 1;

    static AF_Info SPI2_MOSI[] = {
        {PC_1, GPIO_AF5_SPI2},
    };
    static uint8_t SPI2_MOSI_len = 1;

    static AF_Info SPI3_MOSI[] = {
        {PB_2, GPIO_AF7_SPI3},
    };
    static uint8_t SPI3_MOSI_len = 1;

    static AF_Info SPI4_SCK[] = {
        {PE_2, GPIO_AF5_SPI4},
    };
    static uint8_t SPI4_SCK_len = 1;

    static AF_Info SPI4_MISO[] = {
        {PE_5, GPIO_AF5_SPI4},
    };
    static uint8_t SPI4_MISO_len = 1;

    static AF_Info SPI4_MOSI[] = {
        {PE_6, GPIO_AF5_SPI4},
    };
    static uint8_t SPI4_MOSI_len = 1;

    static AF_Info SPI5_SCK[] = {
        {PF_7, GPIO_AF5_SPI5},
    };
    static uint8_t SPI5_SCK_len = 1;

    static AF_Info SPI5_MISO[] = {
        {PF_8, GPIO_AF5_SPI5},
    };
    static uint8_t SPI5_MISO_len = 1;

    static AF_Info SPI5_MOSI[] = {
        {PF_9, GPIO_AF5_SPI5},
        {PF_11, GPIO_AF5_SPI5},
    };
    static uint8_t SPI5_MOSI_len = 2;

    static AF_Info SPI6_SCK[] = {
        {PA_5, GPIO_AF8_SPI6},
    };
    static uint8_t SPI6_SCK_len = 1;

    static AF_Info SPI6_MISO[] = {
        {PA_6, GPIO_AF8_SPI6},
    };
    static uint8_t SPI6_MISO_len = 1;

    static AF_Info SPI6_MOSI[] = {
        {PA_7, GPIO_AF8_SPI6},
    };
    static uint8_t SPI6_MOSI_len = 1;

    AF_Info* array = NULL;
    uint8_t array_len = 0;


    if(handle == SPI1) {
		if(mode == SCK) {
            array = SPI1_SCK;
            array_len = SPI1_SCK_len;
        }
		if(mode == MISO) {
            array = SPI1_MISO;
            array_len = SPI1_MISO_len;
        }
		if(mode == MOSI) {
            array = SPI1_MOSI;
            array_len = SPI1_MOSI_len;
        }
    }

    if(handle == SPI2) {
		if(mode == MOSI) {
            array = SPI2_MOSI;
            array_len = SPI2_MOSI_len;
        }
    }

    if(handle == SPI3) {
		if(mode == MOSI) {
            array = SPI3_MOSI;
            array_len = SPI3_MOSI_len;
        }
    }

    if(handle == SPI4) {
		if(mode == SCK) {
            array = SPI4_SCK;
            array_len = SPI4_SCK_len;
        }
		if(mode == MISO) {
            array = SPI4_MISO;
            array_len = SPI4_MISO_len;
        }
		if(mode == MOSI) {
            array = SPI4_MOSI;
            array_len = SPI4_MOSI_len;
        }
    }

    if(handle == SPI5) {
		if(mode == SCK) {
            array = SPI5_SCK;
            array_len = SPI5_SCK_len;
        }
		if(mode == MISO) {
            array = SPI5_MISO;
            array_len = SPI5_MISO_len;
        }
		if(mode == MOSI) {
            array = SPI5_MOSI;
            array_len = SPI5_MOSI_len;
        }
    }

    if(handle == SPI6) {
		if(mode == SCK) {
            array = SPI6_SCK;
            array_len = SPI6_SCK_len;
        }
		if(mode == MISO) {
            array = SPI6_MISO;
            array_len = SPI6_MISO_len;
        }
		if(mode == MOSI) {
            array = SPI6_MOSI;
            array_len = SPI6_MOSI_len;
        }
    }


    for (uint8_t i = 0; i < array_len; i++) {
        if(array[i].pin == *pin) {
            return array[i].AF;
        }
    }
    return 0; // should never happen
}


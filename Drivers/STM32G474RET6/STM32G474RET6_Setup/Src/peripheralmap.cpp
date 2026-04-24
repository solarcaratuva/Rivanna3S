#include "peripheralmap.h"
#include "stm32g4xx_hal.h"

UART_Peripheral UART_Peripherals[] = {
    {UART4, (PC_11.universal_mask), (PC_10.universal_mask), NC, NC, false},
    {UART5, (PD_2.universal_mask), (PC_12.universal_mask), NC, NC, false},
    {USART1, (PC_5.universal_mask | PA_10.universal_mask | PB_7.universal_mask), (PC_4.universal_mask | PA_9.universal_mask | PB_6.universal_mask), NC, NC, false},
    {USART2, (PA_3.universal_mask | PA_15.universal_mask | PB_4.universal_mask), (PA_2.universal_mask | PA_14.universal_mask | PB_3.universal_mask), NC, NC, false},
    {USART3, (PB_11.universal_mask | PC_11.universal_mask), (PB_10.universal_mask | PC_10.universal_mask | PB_9.universal_mask), NC, NC, false},
};

const uint8_t UART_PERIPHERAL_COUNT = sizeof(UART_Peripherals) / sizeof(UART_Peripherals[0]);

I2C_Peripheral I2C_Peripherals[] = {
    {I2C1, (PA_14.universal_mask | PB_7.universal_mask | PB_9.universal_mask), (PA_13.universal_mask | PA_15.universal_mask), NC, NC, false},
    {I2C2, (PA_8.universal_mask), (PC_4.universal_mask | PA_9.universal_mask), NC, NC, false},
    {I2C3, (PC_9.universal_mask | PC_11.universal_mask | PB_5.universal_mask), (PC_8.universal_mask | PA_8.universal_mask), NC, NC, false},
    {I2C4, (PC_7.universal_mask | PB_7.universal_mask), (PC_6.universal_mask | PA_13.universal_mask), NC, NC, false},
};

const uint8_t I2C_PERIPHERAL_COUNT = sizeof(I2C_Peripherals) / sizeof(I2C_Peripherals[0]);

FDCAN_Peripheral FDCAN_Peripherals[] = {
    {FDCAN1, (PA_11.universal_mask), (PA_12.universal_mask | PB_9.universal_mask), NC, NC, false},
    {FDCAN2, (PB_12.universal_mask | PB_5.universal_mask), (PB_13.universal_mask | PB_6.universal_mask), NC, NC, false},
    {FDCAN3, (PA_8.universal_mask | PB_3.universal_mask), (PA_15.universal_mask | PB_4.universal_mask), NC, NC, false},
};

const uint8_t FDCAN_PERIPHERAL_COUNT = sizeof(FDCAN_Peripherals) / sizeof(FDCAN_Peripherals[0]);

ADC_Peripheral ADC_Peripherals[] = {
    {ADC1, ADC_CHANNEL_6, PC_0.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_7, PC_1.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_8, PC_2.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_9, PC_3.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_1, PA_0.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_2, PA_1.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_3, PA_2.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_4, PA_3.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_15, PB_0.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_12, PB_1.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_14, PB_11.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_11, PB_12.universal_mask, NC, false, 0},
    {ADC1, ADC_CHANNEL_5, PB_14.universal_mask, NC, false, 0},
    {ADC2, ADC_CHANNEL_6, PC_0.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_7, PC_1.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_8, PC_2.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_9, PC_3.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_1, PA_0.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_2, PA_1.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_17, PA_4.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_13, PA_5.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_3, PA_6.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_4, PA_7.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_5, PC_4.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_11, PC_5.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_12, PB_2.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_14, PB_11.universal_mask, NC, false, 1},
    {ADC2, ADC_CHANNEL_15, PB_15.universal_mask, NC, false, 1},
    {ADC3, ADC_CHANNEL_12, PB_0.universal_mask, NC, false, 2},
    {ADC3, ADC_CHANNEL_1, PB_1.universal_mask, NC, false, 2},
    {ADC3, ADC_CHANNEL_5, PB_13.universal_mask, NC, false, 2},
    {ADC4, ADC_CHANNEL_3, PB_12.universal_mask, NC, false, 3},
    {ADC4, ADC_CHANNEL_4, PB_14.universal_mask, NC, false, 3},
    {ADC4, ADC_CHANNEL_5, PB_15.universal_mask, NC, false, 3},
    {ADC5, ADC_CHANNEL_1, PA_8.universal_mask, NC, false, 4},
    {ADC5, ADC_CHANNEL_2, PA_9.universal_mask, NC, false, 4},
};

const uint8_t ADC_PERIPHERAL_COUNT = sizeof(ADC_Peripherals) / sizeof(ADC_Peripherals[0]);

uint8_t adc_channels_claimed[] = {0, 0, 0, 0, 0};

bool use_adc(const ADC_TypeDef* handle) {
    if(handle == ADC1) {
        adc_channels_claimed[0]++;
        return adc_channels_claimed[0] == 1; // only enable the ADC if this is the first channel being used on it
    }
    if(handle == ADC2) {
        adc_channels_claimed[1]++;
        return adc_channels_claimed[1] == 1; // only enable the ADC if this is the first channel being used on it
    }
    if(handle == ADC3) {
        adc_channels_claimed[2]++;
        return adc_channels_claimed[2] == 1; // only enable the ADC if this is the first channel being used on it
    }
    if(handle == ADC4) {
        adc_channels_claimed[3]++;
        return adc_channels_claimed[3] == 1; // only enable the ADC if this is the first channel being used on it
    }
    if(handle == ADC5) {
        adc_channels_claimed[4]++;
        return adc_channels_claimed[4] == 1; // only enable the ADC if this is the first channel being used on it
    }
}



uint8_t get_UART_AF(const USART_TypeDef* handle, const Pin* pin, uint8_t mode) {
    static AF_Info UART4_TX[] = {
        {PC_10, GPIO_AF5_UART4},
    };
    static uint8_t UART4_TX_len = 1;

    static AF_Info UART4_RX[] = {
        {PC_11, GPIO_AF5_UART4},
    };
    static uint8_t UART4_RX_len = 1;

    static AF_Info UART5_TX[] = {
        {PC_12, GPIO_AF5_UART5},
    };
    static uint8_t UART5_TX_len = 1;

    static AF_Info UART5_RX[] = {
        {PD_2, GPIO_AF5_UART5},
    };
    static uint8_t UART5_RX_len = 1;

    static AF_Info USART1_TX[] = {
        {PC_4, GPIO_AF7_USART1},
        {PA_9, GPIO_AF7_USART1},
        {PB_6, GPIO_AF7_USART1},
    };
    static uint8_t USART1_TX_len = 3;

    static AF_Info USART1_RX[] = {
        {PC_5, GPIO_AF7_USART1},
        {PA_10, GPIO_AF7_USART1},
        {PB_7, GPIO_AF7_USART1},
    };
    static uint8_t USART1_RX_len = 3;

    static AF_Info USART2_TX[] = {
        {PA_2, GPIO_AF7_USART2},
        {PA_14, GPIO_AF7_USART2},
        {PB_3, GPIO_AF7_USART2},
    };
    static uint8_t USART2_TX_len = 3;

    static AF_Info USART2_RX[] = {
        {PA_3, GPIO_AF7_USART2},
        {PA_15, GPIO_AF7_USART2},
        {PB_4, GPIO_AF7_USART2},
    };
    static uint8_t USART2_RX_len = 3;

    static AF_Info USART3_TX[] = {
        {PB_10, GPIO_AF7_USART3},
        {PC_10, GPIO_AF7_USART3},
        {PB_9, GPIO_AF7_USART3},
    };
    static uint8_t USART3_TX_len = 3;

    static AF_Info USART3_RX[] = {
        {PB_11, GPIO_AF7_USART3},
        {PC_11, GPIO_AF7_USART3},
    };
    static uint8_t USART3_RX_len = 2;

    AF_Info* array = NULL;
    uint8_t array_len = 0;


    if(handle == UART4) {
        if(mode == RX) {
            array = UART4_RX;
            array_len = UART4_RX_len;
        } else if(mode == TX) {
            array = UART4_TX;
            array_len = UART4_TX_len;
        }
    }

    if(handle == UART5) {
        if(mode == RX) {
            array = UART5_RX;
            array_len = UART5_RX_len;
        } else if(mode == TX) {
            array = UART5_TX;
            array_len = UART5_TX_len;
        }
    }

    if(handle == USART1) {
        if(mode == RX) {
            array = USART1_RX;
            array_len = USART1_RX_len;
        } else if(mode == TX) {
            array = USART1_TX;
            array_len = USART1_TX_len;
        }
    }

    if(handle == USART2) {
        if(mode == RX) {
            array = USART2_RX;
            array_len = USART2_RX_len;
        } else if(mode == TX) {
            array = USART2_TX;
            array_len = USART2_TX_len;
        }
    }

    if(handle == USART3) {
        if(mode == RX) {
            array = USART3_RX;
            array_len = USART3_RX_len;
        } else if(mode == TX) {
            array = USART3_TX;
            array_len = USART3_TX_len;
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
    static AF_Info I2C1_SCL[] = {
        {PA_13, GPIO_AF4_I2C1},
        {PA_15, GPIO_AF4_I2C1},
    };
    static uint8_t I2C1_SCL_len = 2;

    static AF_Info I2C1_SDA[] = {
        {PA_14, GPIO_AF4_I2C1},
        {PB_7, GPIO_AF4_I2C1},
        {PB_9, GPIO_AF4_I2C1},
    };
    static uint8_t I2C1_SDA_len = 3;

    static AF_Info I2C2_SCL[] = {
        {PC_4, GPIO_AF4_I2C2},
        {PA_9, GPIO_AF4_I2C2},
    };
    static uint8_t I2C2_SCL_len = 2;

    static AF_Info I2C2_SDA[] = {
        {PA_8, GPIO_AF4_I2C2},
    };
    static uint8_t I2C2_SDA_len = 1;

    static AF_Info I2C3_SCL[] = {
        {PC_8, GPIO_AF8_I2C3},
        {PA_8, GPIO_AF2_I2C3},
    };
    static uint8_t I2C3_SCL_len = 2;

    static AF_Info I2C3_SDA[] = {
        {PC_9, GPIO_AF8_I2C3},
        {PC_11, GPIO_AF8_I2C3},
        {PB_5, GPIO_AF8_I2C3},
    };
    static uint8_t I2C3_SDA_len = 3;

    static AF_Info I2C4_SCL[] = {
        {PC_6, GPIO_AF8_I2C4},
        {PA_13, GPIO_AF3_I2C4},
    };
    static uint8_t I2C4_SCL_len = 2;

    static AF_Info I2C4_SDA[] = {
        {PC_7, GPIO_AF8_I2C4},
        {PB_7, GPIO_AF3_I2C4},
    };
    static uint8_t I2C4_SDA_len = 2;

    AF_Info* array = NULL;
    uint8_t array_len = 0;


    if(handle == I2C1) {
        if(mode == SDA) {
            array = I2C1_SDA;
            array_len = I2C1_SDA_len;
        } else if(mode == SCL) {
            array = I2C1_SCL;
            array_len = I2C1_SCL_len;
        }
    }

    if(handle == I2C2) {
        if(mode == SDA) {
            array = I2C2_SDA;
            array_len = I2C2_SDA_len;
        } else if(mode == SCL) {
            array = I2C2_SCL;
            array_len = I2C2_SCL_len;
        }
    }

    if(handle == I2C3) {
        if(mode == SDA) {
            array = I2C3_SDA;
            array_len = I2C3_SDA_len;
        } else if(mode == SCL) {
            array = I2C3_SCL;
            array_len = I2C3_SCL_len;
        }
    }

    if(handle == I2C4) {
        if(mode == SDA) {
            array = I2C4_SDA;
            array_len = I2C4_SDA_len;
        } else if(mode == SCL) {
            array = I2C4_SCL;
            array_len = I2C4_SCL_len;
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
    static AF_Info FDCAN1_RX[] = {
        {PA_11, GPIO_AF9_FDCAN1},
    };
    static uint8_t FDCAN1_RX_len = 1;

    static AF_Info FDCAN1_TX[] = {
        {PA_12, GPIO_AF9_FDCAN1},
        {PB_9, GPIO_AF9_FDCAN1},
    };
    static uint8_t FDCAN1_TX_len = 2;

    static AF_Info FDCAN2_RX[] = {
        {PB_12, GPIO_AF9_FDCAN2},
        {PB_5, GPIO_AF9_FDCAN2},
    };
    static uint8_t FDCAN2_RX_len = 2;

    static AF_Info FDCAN2_TX[] = {
        {PB_13, GPIO_AF9_FDCAN2},
        {PB_6, GPIO_AF9_FDCAN2},
    };
    static uint8_t FDCAN2_TX_len = 2;

    static AF_Info FDCAN3_RX[] = {
        {PA_8, GPIO_AF11_FDCAN3},
        {PB_3, GPIO_AF11_FDCAN3},
    };
    static uint8_t FDCAN3_RX_len = 2;

    static AF_Info FDCAN3_TX[] = {
        {PA_15, GPIO_AF11_FDCAN3},
        {PB_4, GPIO_AF11_FDCAN3},
    };
    static uint8_t FDCAN3_TX_len = 2;

    AF_Info* array = NULL;
    uint8_t array_len = 0;


    if(handle == FDCAN1) {
        if(mode == RX) {
            array = FDCAN1_RX;
            array_len = FDCAN1_RX_len;
        } else if(mode == TX) {
            array = FDCAN1_TX;
            array_len = FDCAN1_TX_len;
        }
    }

    if(handle == FDCAN2) {
        if(mode == RX) {
            array = FDCAN2_RX;
            array_len = FDCAN2_RX_len;
        } else if(mode == TX) {
            array = FDCAN2_TX;
            array_len = FDCAN2_TX_len;
        }
    }

    if(handle == FDCAN3) {
        if(mode == RX) {
            array = FDCAN3_RX;
            array_len = FDCAN3_RX_len;
        } else if(mode == TX) {
            array = FDCAN3_TX;
            array_len = FDCAN3_TX_len;
        }
    }

    for (uint8_t i = 0; i < array_len; i++) {
        if(array[i].pin == *pin) {
            return array[i].AF;
        }
    }
    return 0; // should never happen
}

SPI_Peripheral SPI_Peripherals[] = {
    {SPI2, (PB_15.universal_mask), (PB_14.universal_mask), (PB_13.universal_mask), NC, NC, NC, false},
};

const uint8_t SPI_PERIPHERAL_COUNT = sizeof(SPI_Peripherals) / sizeof(SPI_Peripherals[0]);

void gpio_clock_enable(const GPIO_TypeDef* handle){
    if(handle == GPIOB){
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if(handle == GPIOD){
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if(handle == GPIOA){
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if(handle == GPIOC){
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    return;
}

uint8_t get_SPI_AF(const SPI_TypeDef* handle, const Pin* pin, uint8_t mode) {
    static AF_Info SPI2_SCK_pins[] = {
        {PB_13, GPIO_AF5_SPI2},
    };
    static uint8_t SPI2_SCK_len = 1;

    static AF_Info SPI2_MISO_pins[] = {
        {PB_14, GPIO_AF5_SPI2},
    };
    static uint8_t SPI2_MISO_len = 1;

    static AF_Info SPI2_MOSI_pins[] = {
        {PB_15, GPIO_AF5_SPI2},
    };
    static uint8_t SPI2_MOSI_len = 1;

    AF_Info* array = NULL;
    uint8_t array_len = 0;

    if (handle == SPI2) {
        if (mode == SCK) {
            array = SPI2_SCK_pins;
            array_len = SPI2_SCK_len;
        } else if (mode == MISO) {
            array = SPI2_MISO_pins;
            array_len = SPI2_MISO_len;
        } else if (mode == MOSI) {
            array = SPI2_MOSI_pins;
            array_len = SPI2_MOSI_len;
        }
    }

    for (uint8_t i = 0; i < array_len; i++) {
        if (array[i].pin == *pin) {
            return array[i].AF;
        }
    }
    return 0;
}
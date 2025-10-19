
#include "UART.h"
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

extern UART_HandleTypeDef huart;


// Constructor
UART::UART(Pin tx, Pin rx, uint32_t baud)
    : tx(tx), rx(rx), baud(baud) {
    initGPIO(tx, rx);
    initUART(baud);
}

// Initialize GPIO for TX/RX
void UART::initGPIO(Pin tx, Pin rx) {
    __HAL_RCC_GPIOF_CLK_ENABLE(); // ⚠️ Change this depending on the port

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // TX
    GPIO_InitStruct.Pin       = tx.block_mask;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_UART7; // ⚠️ depends on which USART you’re using
    HAL_GPIO_Init(tx.block, &GPIO_InitStruct);

    // RX
    GPIO_InitStruct.Pin       = rx.block_mask;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF7_UART7; // ⚠️ match same USART
    HAL_GPIO_Init(rx.block, &GPIO_InitStruct);
}

// Initialize UART peripheral
void UART::initUART(uint32_t baud) {
    __HAL_RCC_UART7_CLK_ENABLE(); // ⚠️ match peripheral to your pins

    huart.Instance          = UART7; // ⚠️ change to USART2, USART3, etc.
    huart.Init.BaudRate     = baud;
    huart.Init.WordLength   = UART_WORDLENGTH_8B;
    huart.Init.StopBits     = UART_STOPBITS_1;
    huart.Init.Parity       = UART_PARITY_NONE;
    huart.Init.Mode         = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&huart);

//    HAL_UARTEx_SetTxFifoThreshold(&huart, UART_TXFIFO_THRESHOLD_1_8);
//
//    HAL_UARTEx_SetRxFifoThreshold(&huart, UART_RXFIFO_THRESHOLD_1_8);
//
//    HAL_UARTEx_DisableFifoMode(&huart);

}

void UART::read(uint8_t *buffer, size_t length){
	HAL_UART_Receive(&huart, buffer, length, HAL_MAX_DELAY);
}

void UART::write(uint8_t* buffer, size_t length) {
	HAL_UART_Transmit(&huart, buffer, length, HAL_MAX_DELAY);
}
        
UART_Peripheral* UART::findUARTPins(Pin tx, Pin rx) {
    for (size_t i = 0; i < UART_PERIPHERAL_COUNT; ++i) {
        UART_Peripheral* peripheral = &UART_Peripherals[i];
        if (((*peripheral).txd_valid_pins & tx.universal_mask) &&
            ((*peripheral).rxd_valid_pins & rx.universal_mask)) {
            if (peripheral->isClaimed) {
                (*peripheral).isClaimed = true;
                return peripheral;
            }
        }
    }
    return nullptr; // No matching peripheral found
}
        




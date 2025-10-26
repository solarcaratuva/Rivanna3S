
#include "UART.h"
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"


// Constructor
UART::UART(Pin tx, Pin rx, uint32_t baud)
    : tx(tx), rx(rx), baud(baud) {
	uart_periph = find_uart_pins(tx, rx);
    if(uart_periph != nullptr) {
    	uart_periph->rxd_used = rx;
    	uart_periph->txd_used = tx;
        init_gpio(uart_periph);
        init_uart(baud);
        initialized = true; 
    }
}

// Initialize GPIO for TX/RX
void UART::init_gpio(UART_Peripheral* uart_periph) {
    Pin rx_pin = uart_periph->rxd_used;
    Pin tx_pin = uart_periph->txd_used;

    //Turn on clocks for each gpio port
    gpio_clock_enable(rx_pin.block);
    gpio_clock_enable(tx_pin.block);


    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // TX
    GPIO_InitStruct.Pin       = tx_pin.block_mask;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = uart_periph->alternate_function;
    HAL_GPIO_Init(tx_pin.block, &GPIO_InitStruct);

    // RX
    GPIO_InitStruct.Pin       = rx_pin.block_mask;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate =  uart_periph->alternate_function;
    HAL_GPIO_Init(rx_pin.block, &GPIO_InitStruct);
}

// Initialize UART peripheral
void UART::init_uart(uint32_t baud) {
    //Turn on clocks for all possible UART

	uart_clock_enable(uart_periph->handle);

    huart.Instance          = uart_periph-> handle;
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

// Functions that was used in Embedded UART example
//    HAL_UARTEx_SetTxFifoThreshold(&huart, UART_TXFIFO_THRESHOLD_1_8);
//
//    HAL_UARTEx_SetRxFifoThreshold(&huart, UART_RXFIFO_THRESHOLD_1_8);
//
//    HAL_UARTEx_DisableFifoMode(&huart);

}

void UART::read(uint8_t *buffer, uint16_t length){
	if(initialized != 0) {
		HAL_UART_Receive(&huart, buffer, length, HAL_MAX_DELAY);
	}
}

void UART::read(uint8_t *buffer, uint16_t length, uint32_t timeout_ms){
	if(initialized != 0) {
		HAL_UART_Receive(&huart, buffer, length, timeout_ms);
	}
}

void UART::write(uint8_t* buffer, uint16_t length) {
	if(initialized != 0) {
		HAL_UART_Transmit(&huart, buffer, length, HAL_MAX_DELAY);
	}
}
        
UART_Peripheral* UART::find_uart_pins(Pin tx, Pin rx) {
    for (size_t i = 0; i < UART_PERIPHERAL_COUNT; ++i) {
        UART_Peripheral* peripheral = &UART_Peripherals[i];
        if (((*peripheral).txd_valid_pins & tx.universal_mask) &&
            ((*peripheral).rxd_valid_pins & rx.universal_mask)) {
            if (!peripheral->isClaimed) {
                (*peripheral).isClaimed = true;
                return peripheral;
            }
        }
    }
    return nullptr; // No matching peripheral found
}
        





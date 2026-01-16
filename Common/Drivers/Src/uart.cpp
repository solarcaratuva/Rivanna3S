
#include "UART.h"
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

extern "C" void HAL_UART_MspInit_custom(USART_TypeDef* uartHandle, Pin pin, uint8_t af);
extern "C" UART_HandleTypeDef* UART_init(USART_TypeDef* uart, uint32_t baudrate);

// Constructor
UART::UART(Pin tx, Pin rx, uint32_t baud)
    : tx(tx), rx(rx), baud(baud) {
	uart_periph = find_uart_pins(tx, rx);
    if(uart_periph == nullptr) {
        initialized = false;
        return;
    }
    uart_periph->rxd_used = rx;
    uart_periph->txd_used = tx;
    
    gpio_clock_enable(tx.block);
    gpio_clock_enable(rx.block);

    uint8_t tx_af = get_UART_AF(uart_periph->handle, &tx, TX);
    uint8_t rx_af = get_UART_AF(uart_periph->handle, &rx, RX);
    HAL_UART_MspInit_custom(uart_periph->handle, tx, tx_af);
    HAL_UART_MspInit_custom(uart_periph->handle, rx, rx_af);
    
    huart = UART_init(uart_periph->handle, baud);

    initialized = true; 
}



void UART::read(uint8_t *buffer, uint16_t length){
	if(initialized) {
		HAL_UART_Receive(huart, buffer, length, HAL_MAX_DELAY);
	}
}

void UART::read(uint8_t *buffer, uint16_t length, uint32_t timeout_ms){
	if(initialized) {
		HAL_UART_Receive(huart, buffer, length, timeout_ms);
	}
}

void UART::write(uint8_t* buffer, uint16_t length) {
	if(initialized) {
		HAL_UART_Transmit(huart, buffer, length, HAL_MAX_DELAY);
	}
}
        
UART_Peripheral* UART::find_uart_pins(Pin tx, Pin rx) {
    for (size_t i = 0; i < UART_PERIPHERAL_COUNT; ++i) {
        UART_Peripheral* peripheral = &UART_Peripherals[i];
        if ((peripheral->txd_valid_pins & tx.universal_mask) &&
            (peripheral->rxd_valid_pins & rx.universal_mask)) {
            if (!peripheral->isClaimed) {
                peripheral->isClaimed = true;
                return peripheral;
            }
        }
    }
    return nullptr; // No matching peripheral found
}




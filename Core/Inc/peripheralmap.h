#ifndef PERIPHERALMAP_H
#define PERIPHERALMAP_H


#include "pinmap.h"
#include "stm32h743xx.h"
#include <stdbool.h>

// I2C peripheral struct
typedef struct {
    I2C_TypeDef* handle;
    uint64_t sda_valid_pins;
    uint64_t scl_valid_pins;
    Pin sda_used;
    Pin scl_used;
    bool isClaimed;
    // Queue[float] return_value_queue
} I2C_Peripheral;

// UART peripheral struct
typedef struct {
    USART_TypeDef* handle;
    uint64_t rxd_valid_pins;
    uint64_t txd_valid_pins;
    Pin rxd_used;
    Pin txd_used;
    uint8_t alternate_function; 

    bool isClaimed;
    // Queue[float] return_value_queue
} UART_Peripheral;

// Declare global arrays
extern UART_Peripheral UART_Peripherals[];
extern const uint8_t UART_PERIPHERAL_COUNT;

extern I2C_Peripheral I2C_Peripherals[];
extern const uint8_t I2C_PERIPHERAL_COUNT;

void uart_clock_enable(USART_TypeDef* handle);

void gpio_clock_enable(GPIO_TypeDef* handle);



#endif /* PERIPHERALMAP */

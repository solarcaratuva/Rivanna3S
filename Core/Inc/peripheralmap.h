#ifndef PERIPHERALMAP_H
#define PERIPHERALMAP_H

#include <stdbool.h>
#include <stdint.h>
#include "pinmap.h"
#include "stm32h743xx.h"

#define SDA 1
#define SCL 2
#define RX 3
#define TX

typedef struct {
  Pin pin;
  uint8_t af;
} af_info;

// I2C peripheral struct
typedef struct {
    I2C_TypeDef* handle;
    uint64_t sda_valid_pins;
    uint64_t scl_valid_pins;
    Pin sda_used;
    Pin scl_used;
    uint8_t alternate_function;
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

typedef struct {
    ADC_TypeDef *instance;    // ADC1, ADC2, ...
    uint32_t channel;         // ADC_CHANNEL_0, ...
    uint64_t pin_mask;        // PA_0.universal_mask
    Pin used_pin;
    bool isClaimed;
    uint8_t instance_num;
} ADC_Peripheral;


// Declare global arrays
extern UART_Peripheral UART_Peripherals[];
extern const uint8_t UART_PERIPHERAL_COUNT;

extern I2C_Peripheral I2C_Peripherals[];
extern const uint8_t I2C_PERIPHERAL_COUNT;

extern ADC_Peripheral ADC_Peripherals[];
extern const uint8_t ADC_PERIPHERAL_COUNT;
extern uint8_t adc_channels_claimed[];

void uart_clock_enable(USART_TypeDef* handle);
void gpio_clock_enable(GPIO_TypeDef* port);
uint8_t get_i2c_af(I2C_TypeDef* handle, Pin pin, uint8_t mode);

#endif /* PERIPHERALMAP */


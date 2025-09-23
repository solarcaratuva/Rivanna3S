#ifndef PERIPHERALMAP_H
#define PERIPHERALMAP_H


#include "pinmap.h"
#include "stm32h743xx.h"
#include <stdbool.h>

// I2C peripheral struct
typedef struct {
    I2C_TypeDef handle;
    uint64_t sda_valid_pins;
    uint64_t scl_valid_pins;
    Pin sda_used;
    Pin scl_used;
    bool isClaimed;
    // Queue[float] return_value_queue
} I2C_Peripheral;

// UART peripheral struct
typedef struct {
    USART_TypeDef handle;
    uint64_t rxd_valid_pins;
    uint64_t txd_valid_pins;
    Pin rxd_used;
    Pin txd_used;
    bool isClaimed;
    // Queue[float] return_value_queue
} UART_Peripheral;


// Define UART peripherals
#define USART_2 (UART_Peripheral){USART2, PA_3.universal_mask, PA_2.universal_mask, nullptr, nullptr, false}
#define UART_4 (UART_Peripheral){UART4, PA_1.universal_mask, PA_0.universal_mask, nullptr, nullptr, false}
#define UART_7 (UART_Peripheral){UART7, (PF_6.universal_mask | PE_7.universal_mask), (PF_7.universal_mask | PE_8.universal_mask), nullptr, nullptr, false}

// Define I2C peripherals
#define I2C_2 (I2C_Peripheral){I2C2, PF_0.universal_mask, PF_1.universal_mask, nullptr, nullptr, false}
#define I2C_4 (I2C_Peripheral){I2C4, PF_15.universal_mask, PF_14.universal_mask, nullptr, nullptr, false}


#endif /* PERIPHERALMAP */
#include "pinmap.h"
#include "stm32h743xx.h"

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
// Use #defines in stm32h743xx.h file for USART_TypeDef handle;
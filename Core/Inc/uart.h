#ifndef uart
#define uart

#include "stm32h7xx_hal.h"
#include "pinmap.h"

extern UART_HandleTypeDef huart4;

class uart {
public:
    uart(Pin pin1, Pin pin2, uint32_t baud);
    void read(uint8_t *buffer, size_t length);
    void write(uint8_t *buffer, size_t length);
};

#endif
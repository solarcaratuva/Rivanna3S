#ifndef uart
#define uart

#include "stm32h7xx_hal.h"
#include "pinmap.h"

extern UART_HandleTypeDef huart;

class UART {
public:
    explicit UART(Pin tx, Pin rx, uint32_t baud);

    void read(uint8_t *buffer, size_t length);

    void write(uint8_t *buffer, size_t length);

private:
    UART_HandleTypeDef huart;
    void initGPIO(Pin tx, Pin rx);
    void initUART(uint32_t baud);

    Pin tx, rx;
    uint32_t baud;

    UART_Peripheral* findUARTPins(Pin tx, Pin rx);
};


#endif

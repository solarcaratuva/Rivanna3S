#ifndef uart
#define uart

#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

extern UART_HandleTypeDef huart;

class UART {
public:
    explicit UART(Pin tx, Pin rx, uint32_t baud);

    void read(uint8_t *buffer, size_t length);

    void write(uint8_t *buffer, size_t length);

private:
    UART_HandleTypeDef huart;
    void initGPIO(UART_Peripheral* uart_peripheral);
    void initUART(uint32_t baud);
    UART_Peripheral* findUARTPins(Pin tx, Pin rx);

    Pin tx, rx;
    uint32_t baud;
    bool initialized = false; 
    UART_Peripheral* uart_periph;

    


};


#endif

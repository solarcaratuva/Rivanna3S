#ifndef UART_H
#define UART_H

#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

class UART {
public:
	bool initialized = false;

    explicit UART(Pin tx, Pin rx, uint32_t baud);

    void read(uint8_t *buffer, uint16_t  length);

    void read(uint8_t *buffer, uint16_t length, uint32_t timeout_ms);

    void write(uint8_t *buffer, uint16_t  length);


private:
    UART_HandleTypeDef huart;
    void init_gpio(UART_Peripheral* uart_peripheral);
    void init_uart(uint32_t baud);
    UART_Peripheral* find_uart_pins(Pin tx, Pin rx);

    Pin tx, rx;
    uint32_t baud;
    UART_Peripheral* uart_periph;

    


};


#endif //UART_H

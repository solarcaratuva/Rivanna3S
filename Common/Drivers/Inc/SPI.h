#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"


class SPI {
public:
    bool initialized = false;
    explicit SPI(Pin mosi, Pin miso, Pin sck, uint32_t baudrate);
    int write(uint8_t *tx_buffer, uint16_t length);
    int read(uint8_t *rx_buffer, uint16_t length);
    int transfer(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t length);
private:
    SPI_Peripheral* spi_periph;
    SPI_Peripheral* findSPIPins(Pin mosi, Pin miso, Pin sck);
    SPI_HandleTypeDef* hspi;
};


#endif
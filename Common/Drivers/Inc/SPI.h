#ifndef SPI_H
#define SPI_H

/**
 * @file SPI.h
 * @brief SPI communication interface for STM32 microcontrollers using HAL.
 *
 * This header defines the SPI class, which provides methods for initializing
 * and using the SPI peripheral on STM32 microcontrollers. The class abstracts
 * the underlying HAL functions to simplify SPI communication in C++ projects.
 *
 * Example usage:
 * @code
 * #include "SPI.h"
 *
 * SPI spi(PA_5, PA_6, PA_7); // SPI1 (SCK=PA5, MISO=PA6, MOSI=PA7)
 *
 * uint8_t dataToSend = 0x55;
 * spi.write(&dataToSend, 1); // Write 1 byte to the SPI bus
 *
 * uint8_t buffer[4];
 * spi.read(buffer, 4); // Read 4 bytes from the SPI bus
 * @endcode
 *
 * @see stm32h7xx_hal_spi.h
 * @see pinmap.h
 * @see peripheralmap.h
 */

#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

class SPI
{
public:
    bool initalized = false;

    SPI(Pin MOSI, Pin MISO, Pin SCK, uint32_t frequency = 1000000);

    uint8_t transfer(uint8_t data);
    void transfer(uint8_t *txData, uint8_t *rxData, size_t length);
    void write(uint8_t *data, size_t length);
    void read(uint8_t *buffer, size_t length);

    void setFrequency(uint32_t frequency);

private:
    SPI_HandleTypeDef hspi;
    Pin mosi_, miso_, sck_;
};

#endif
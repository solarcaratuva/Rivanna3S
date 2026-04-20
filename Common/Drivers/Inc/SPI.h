#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "stm32_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"


class SPI {
public:
    bool initialized = false;

    /**
     * @brief Construct a new SPI object
     * 
     * This constructor initializes the SPI peripheral based on the provided MOSI, MISO, and SCK pins, as well as the desired baud rate. It configures the necessary GPIO pins and initializes the SPI peripheral using the STM32 HAL library.
     * 
     * @param mosi Pin to be used for MOSI
     * @param miso Pin to be used for MISO
     * @param sck Pin to be used for SCK
     * @param baudrate Desired baud rate for SPI communication
     * 
     */
    explicit SPI(Pin mosi, Pin miso, Pin sck, uint32_t baudrate);

    /**
     * @brief Writes data to the SPI bus
     * 
     * This function transmits data over the SPI bus using the STM32 HAL library. It checks if the SPI peripheral is initialized before attempting to transmit data. If the transmission is successful, it returns 0; otherwise, it returns a non-zero error code.
     * 
     * @param tx_buffer Pointer to the data to be transmitted
     * @param length Length of the data to be transmitted
     * 
     * @return 0 on success, non-zero error code on failure
     */
    int write(uint8_t *tx_buffer, uint16_t length);

    /**
     * @brief Reads data from the SPI bus
     * 
     * This function receives data from the SPI bus using the STM32 HAL library. It checks if the SPI peripheral is initialized before attempting to receive data. If the reception is successful, it returns 0; otherwise, it returns a non-zero error code.
     * 
     * @param rx_buffer Pointer to the buffer where received data will be stored
     * @param length Length of the data to be received
     * 
     * @return 0 on success, non-zero error code on failure
     */
    int read(uint8_t *rx_buffer, uint16_t length);

    /**
     * @brief Simultaneously writes and reads data on the SPI bus
     * 
     * This function performs a full-duplex SPI transfer, allowing simultaneous transmission and reception of data. It checks if the SPI peripheral is initialized before attempting the transfer. If the transfer is successful, it returns 0; otherwise, it returns a non-zero error code.
     * 
     * @note This function is not properly tested. It is recommended to use write() and read() separately instead of this function until it is tested.
     * 
     * @param tx_buffer Pointer to the data to be transmitted
     * @param rx_buffer Pointer to the buffer where received data will be stored
     * @param length Length of the data to be transmitted and received
     * 
     * @return 0 on success, non-zero error code on failure
     */
    int transfer(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t length);
private:
    SPI_Peripheral* spi_periph;
    SPI_Peripheral* findSPIPins(Pin mosi, Pin miso, Pin sck);
    SPI_HandleTypeDef* hspi;
};


#endif
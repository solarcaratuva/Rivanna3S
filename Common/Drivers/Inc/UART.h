/**
 * @file UART.h
 * @brief C++ wrapper class for UART communication using STM32 HAL drivers.
 *
 * This class provides a high-level object-oriented interface for configuring
 * and communicating over UART peripherals on STM32 microcontrollers.
 *
 * It automatically handles pin mapping, GPIO configuration, and peripheral
 * initialization based on the provided TX/RX pins and baud rate. It wraps
 * the STM32 HAL UART driver functions to simplify sending and receiving data.
 *
 * Example usage:
 * @code
 * #include "UART.h"
 *
 * UART serial(PA_2, PA_3, 115200);   // Create UART instance for USART2
 *
 * uint8_t message[] = "Hello";
 * serial.write(message, sizeof(message));
 *
 * uint8_t buffer[10];
 * serial.read(buffer, sizeof(buffer), 1000);  // Read with 1s timeout
 * @endcode
 *
 * @note The corresponding peripheral clock and GPIO clocks are enabled
 *       automatically inside the initialization routines.
 *
 * @see stm32h7xx_hal_uart.h
 * @see pinmap.h
 * @see peripheralmap.h
 */

#ifndef UART_H
#define UART_H

#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

/**
 * @class UART
 * @brief Provides UART communication interface using STM32 HAL.
 */
class UART {
public:
    /**
     * @brief Indicates whether the UART peripheral was successfully initialized.
     */
    bool initialized = false;

    /**
     * @brief Constructs a UART object and initializes the specified UART peripheral.
     * @param tx  TX pin (must correspond to a valid UART transmit pin).
     * @param rx  RX pin (must correspond to a valid UART receive pin).
     * @param baud Baud rate for the UART communication (e.g. 9600, 115200).
     */
    explicit UART(Pin tx, Pin rx, uint32_t baud);

    /**
     * @brief Reads data from the UART (blocking call, no timeout).
     * @param buffer Pointer to the buffer where received data will be stored.
     * @param length Number of bytes to read.
     */
    void read(uint8_t *buffer, uint16_t length);

    /**
     * @brief Reads data from the UART with a timeout.
     * @param buffer Pointer to the buffer where received data will be stored.
     * @param length Number of bytes to read.
     * @param timeout_ms Timeout in milliseconds before aborting the read.
     */
    void read(uint8_t *buffer, uint16_t length, uint32_t timeout_ms);

    /**
     * @brief Writes data to the UART (blocking call).
     * @param buffer Pointer to the data buffer to be transmitted.
     * @param length Number of bytes to transmit.
     */
    void write(uint8_t *buffer, uint16_t length);

private:
    UART_HandleTypeDef* huart;   /**< HAL UART handle used for configuration and I/O. */

    void init_gpio(UART_Peripheral* uart_periph);

    /**
     * @brief Initializes the UART peripheral with the specified baud rate.
     * @param baud Baud rate for UART communication.
     */
    void init_uart(uint32_t baud);

    /**
     * @brief Finds the UART peripheral that matches the provided TX/RX pins.
     * @param tx TX pin.
     * @param rx RX pin.
     * @return Pointer to the matching UART_Peripheral struct, or nullptr if invalid.
     */
    UART_Peripheral* find_uart_pins(Pin tx, Pin rx);

    Pin tx_;                     /**< TX pin object. */
    Pin rx_;                     /**< RX pin object. */
    uint32_t baud_;              /**< UART baud rate. */
    UART_Peripheral* uart_periph; /**< Pointer to matched UART peripheral. */
};

#endif // UART_H

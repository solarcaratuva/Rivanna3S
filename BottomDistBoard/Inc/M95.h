#ifndef INC_M95_H_
#define INC_M95_H_

//#include "main.h"
#include "SPI.h"

#define M95040		0
#define M95256		1
#define M95M04		2

// SPI commands
#define SPI_WREN	0x06
#define SPI_WRDI	0x04
#define SPI_RDSR	0x05
#define SPI_WRSR	0x01
#define SPI_READ	0x03
#define SPI_WRITE	0x02
#define SPI_RDID	0x83
#define SPI_WRID	0x82
#define SPI_RDLS	0x83
#define SPI_LID		0x82

extern SPI spi4;

/**
 * @brief Enable SPI chip select.
 * @retval None.
 * @param  M95Type: integer that indicates which memory
 * device is selected (distinguished by memory size).
 *
 * The accepted values are:
 * #define M95040		0
 * #define M95256		1
 * #define M95M04		2
 */
void EnableChipSelectM95(uint8_t M95Type);

/**
 * @brief Disable SPI chip select.
 * @retval None.
 * @param  M95Type: integer that indicates which memory
 * device is selected (distinguished by memory size).
 *
 * The accepted values are:
 * #define M95040		0
 * #define M95256		1
 * #define M95M04		2
 */
void DisableChipSelectM95(uint8_t M95Type);

/**
 * @brief Disable SPI write protect (active-low).
 * @retval None.
 * @param  None.
 */
void DisableWriteProtectM95(void);

/**
 * @brief Disable SPI hold (active-low);
 * @retval None.
 * @param  None.
 */
void DisableHoldM95(void);

/**
 * @brief Enable the selected device for writing.
 * @retval None.
 * @param  M95Type: integer that indicates which memory
 * device is selected (distinguished by memory size).
 *
 * The accepted values are:
 * #define M95040		0
 * #define M95256		1
 * #define M95M04		2
 */
void WriteEnableM95(uint8_t M95Type);

/**
 * @brief Write to the status register.
 * @retval None.
 * @param WriteData: Value to write to status register.
 * @param  M95Type: integer that indicates which memory
 * device is selected (distinguished by memory size).
 *
 * The accepted values are:
 * #define M95040		0
 * #define M95256		1
 * #define M95M04		2
 */
void WriteStatusRegisterM95(uint8_t M95Type, uint8_t WriteData);

/**
 * @brief Read from the status register.
 * @retval Contents of status register as uint8_t type.
 * @param  M95Type: integer that indicates which memory
 * device is selected (distinguished by memory size).
 *
 * The accepted values are:
 * #define M95040		0
 * #define M95256		1
 * #define M95M04		2
 */
uint8_t ReadStatusRegisterM95(uint8_t M95Type);

/**
 * @brief Write a single byte.
 * @retval None.
 * @param  M95Type: integer that indicates which memory
 * device is selected (distinguished by memory size).
 *
 * The accepted values are:
 * #define M95040		0
 * #define M95256		1
 * #define M95M04		2
 */
void WriteByteM95(uint8_t M95Type, uint8_t Address, uint8_t WriteData);

/**
 * @brief
 * @retval None.
 * @param  M95Type: integer that indicates which memory
 * device is selected (distinguished by memory size).
 *
 * The accepted values are:
 * #define M95040		0
 * #define M95256		1
 * #define M95M04		2
 */
uint8_t ReadByteM95(uint8_t M95Type, uint8_t Address);


#endif /* INC_M95_H_ */
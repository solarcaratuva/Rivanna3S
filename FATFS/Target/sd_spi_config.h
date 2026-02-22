/**
 * @file sd_spi_config.h
 * @brief SD Card SPI configuration for FatFS driver.
 *
 * This file defines the SPI peripheral and CS pin used for SD card access.
 * Include this file before user_diskio_spi.h.
 */

#ifndef SD_SPI_CONFIG_H
#define SD_SPI_CONFIG_H

#include "stm32h7xx_hal.h"

/* SD Card SPI configuration */
/* Define which SPI peripheral to use for SD card */
#define SD_SPI_HANDLE hspi2

/* Define the CS (Chip Select) pin for SD card */
#define SD_CS_GPIO_Port GPIOB
#define SD_CS_Pin GPIO_PIN_12

/* Declare the external SPI handle */
extern SPI_HandleTypeDef hspi2;

#endif /* SD_SPI_CONFIG_H */

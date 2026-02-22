/**
 ******************************************************************************
 * @file    spi.h
 * @brief   This file contains all the function prototypes for
 *          the spi.c file
 ******************************************************************************
 */

#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32h7xx_hal.h"

    extern SPI_HandleTypeDef hspi1;
    extern SPI_HandleTypeDef hspi2;

    void MX_SPI1_Init(void);
    void MX_SPI2_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

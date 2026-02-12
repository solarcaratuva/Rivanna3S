/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    fdcan.c
 * @brief   This file provides code for the configuration
 *          of the FDCAN instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "fdcan.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

FDCAN_HandleTypeDef *FDCAN_init(FDCAN_GlobalTypeDef *hadc, uint32_t baudrate)
{
  if (hadc == FDCAN1)
  {
    MX_FDCAN1_Init(baudrate);
    return &hfdcan1;
  }

  return &hfdcan1; // Default return to avoid compiler warning
}

uint32_t calculate_prescaler(FDCAN_HandleTypeDef *hfdcan, uint32_t peripheral_clock, uint32_t baudrate)
{
  uint32_t time_quanta = 1 + hfdcan->Init.NominalTimeSeg1 + hfdcan->Init.NominalTimeSeg2;
  return peripheral_clock / (baudrate * time_quanta);
}

/* FDCAN1 init function */
void MX_FDCAN1_Init(uint32_t baudrate)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;

  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 2;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 8;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 0;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 0;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 8;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.NominalPrescaler = calculate_prescaler(&hfdcan1, 40000000, baudrate);
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    // Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */
}


void HAL_FDCAN_MspInit_custom(FDCAN_GlobalTypeDef *fdcanHandle, Pin pin, uint8_t af)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (fdcanHandle == FDCAN1)
  {
    /* USER CODE BEGIN FDCAN1_MspInit 0 */

    /* USER CODE END FDCAN1_MspInit 0 */
    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    /* Enable GPIO clock for the correct port */
   

    
    /**FDCAN1 GPIO Configuration
    Configure the specific pin
    */
    GPIO_InitStruct.Pin = pin.block_mask;

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af;
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct);

    /* USER CODE BEGIN FDCAN1_MspInit 1 */

    /* USER CODE END FDCAN1_MspInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

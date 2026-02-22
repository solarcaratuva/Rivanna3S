uint32_t calculate_prescaler(FDCAN_HandleTypeDef *hfdcan, uint32_t peripheral_clock, uint32_t baudrate);

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
#include "pinmap.h"
#include "peripheralmap.h"
#include "stm32h7xx_hal.h"
 // <-- This line changed when importing
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

/* FDCAN1 init function */
void MX_FDCAN1_Init(uint32_t baudrate) // <-- This line changed when importing
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = calculate_prescaler(hfdcan1, 10000000, baudrate); // <-- This line changed when importing
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 1;
  hfdcan1.Init.NominalTimeSeg2 = 1;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

void HAL_FDCAN_MspInit_custom(FDCAN_GlobalTypeDef *fdcanHandle, Pin pin, uint8_t af) // <-- This line changed when importing
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(fdcanHandle == FDCAN1) // <-- This line changed when importing)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN1;
    PeriphClkInit.Fdcan1ClockSelection = RCC_FDCAN1CLKSOURCE_PLL1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      //Error_Handler(); // <-- This line changed when importing
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA12     ------> FDCAN1_TX
    PA11     ------> FDCAN1_RX
    */
    GPIO_InitStruct.Pin = pin.block_mask | pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

FDCAN_HandleTypeDef* FDCAN_init(FDCAN_GlobalTypeDef* inst, uint32_t baudrate) {
    if (inst == FDCAN1) {
        MX_FDCAN1_Init(baudrate);
        return &hfdcan1;
    }
    else {
        return NULL;
    }
}


uint32_t calculate_prescaler(FDCAN_HandleTypeDef *hfdcan, uint32_t peripheral_clock, uint32_t baudrate) {
    uint32_t time_quanta = 1 + hfdcan->Init.NominalTimeSeg1 + hfdcan->Init.NominalTimeSeg2;
    return peripheral_clock / (baudrate * time_quanta);
}
    
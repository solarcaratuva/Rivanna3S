/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fdcan.c
  * @brief   This file provides code for the configuration
  *          of the FDCAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "fdcan.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;

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

  // Set baud rate using helper function
  //hfdcan1.Init.NominalPrescaler = 16;
  //hfdcan1.Init.NominalSyncJumpWidth = 1;
  //hfdcan1.Init.NominalTimeSeg1 = 2;
  //hfdcan1.Init.NominalTimeSeg2 = 2;
  //hfdcan1.Init.DataPrescaler = 1;
  //hfdcan1.Init.DataSyncJumpWidth = 1;
 // hfdcan1.Init.DataTimeSeg1 = 1;
  //hfdcan1.Init.DataTimeSeg2 = 1;

  FDCAN1_SetBaudrate(baudrate);

  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 0;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 0;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 0;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */



}

    void FDCAN1_SetBaudrate(uint32_t baudrate){
      if (baudrate == 0U) {
        return;
      }

      uint32_t fdcan_ker_clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_FDCAN);

      const uint32_t min_nbtq  = 8U;
      const uint32_t max_nbtq  = 25U;
      const uint32_t max_presc = 512U;
      const uint32_t target_sp_permille = 800U; /* target 80% sample point */

      uint32_t best_presc = 0U, best_ts1 = 0U, best_ts2 = 0U, best_nbtq = 0U;
      uint64_t best_baud_err = UINT64_MAX;
      uint32_t best_sp_err = UINT32_MAX;

      /* search for best prescaler / time-quanta / sample-point combination */
      for (uint32_t presc = 1U; presc <= max_presc; ++presc) {
        for (uint32_t nbtq = min_nbtq; nbtq <= max_nbtq; ++nbtq) {
          /* compute achievable baud with integer presc and nbtq */
          uint64_t denom = (uint64_t)presc * (uint64_t)nbtq;
          if (denom == 0U) continue;
          uint64_t actual_baud = (uint64_t)fdcan_ker_clk / denom;
          uint64_t baud_err = (actual_baud > baudrate) ? (actual_baud - baudrate) : (baudrate - actual_baud);

          /* quick prune: if this baud error already worse than best, skip fine-grained checks */
          if (baud_err > best_baud_err) continue;

          /* split nbtq into SyncSeg(1) + TS1 + TS2; require TS1>=1, TS2>=1 */
          for (uint32_t ts1 = 1U; ts1 <= (nbtq - 2U); ++ts1) {
            uint32_t ts2 = (nbtq - 1U) - ts1;
            if (ts2 < 1U) continue;

            uint32_t sp_permille = ((1U + ts1) * 1000U) / nbtq;
            uint32_t sp_err = (sp_permille > target_sp_permille) ? (sp_permille - target_sp_permille)
                                                                 : (target_sp_permille - sp_permille);

            /* choose candidate: prefer lower baud error; for equal baud error prefer closer sample point */
            if ((baud_err < best_baud_err) || (baud_err == best_baud_err && sp_err < best_sp_err)) {
              best_baud_err = baud_err;
              best_sp_err = sp_err;
              best_presc = presc;
              best_nbtq = nbtq;
              best_ts1 = ts1;
              best_ts2 = ts2;
            }
          }
        }
      }

      /* Fallback to reasonable defaults if nothing found */
      if (best_presc == 0U) {
        best_presc = 16U;
        best_ts1 = 2U;
        best_ts2 = 2U;
      }

      /* Apply found timings to hfdcan1 Init structure */
      hfdcan1.Init.NominalPrescaler = best_presc;
      hfdcan1.Init.NominalTimeSeg1  = best_ts1;
      hfdcan1.Init.NominalTimeSeg2  = best_ts2;

      /* SJW: per RM, max 4; must be >=1 */
      uint32_t sjw = (best_ts2 > 4U) ? 4U : best_ts2;
      if (sjw == 0U) sjw = 1U;
      hfdcan1.Init.NominalSyncJumpWidth = sjw;

      /* Mirror nominal settings into data phase (classic CAN behavior) */
      hfdcan1.Init.DataPrescaler     = hfdcan1.Init.NominalPrescaler;
      hfdcan1.Init.DataTimeSeg1      = hfdcan1.Init.NominalTimeSeg1;
      hfdcan1.Init.DataTimeSeg2      = hfdcan1.Init.NominalTimeSeg2;
      hfdcan1.Init.DataSyncJumpWidth = hfdcan1.Init.NominalSyncJumpWidth;
    }



void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspInit 0 */

  /* USER CODE END FDCAN1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    __HAL_RCC_FDCAN_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN FDCAN1_MspInit 1 */

  /* USER CODE END FDCAN1_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* fdcanHandle)
{

  if(fdcanHandle->Instance==FDCAN1)
  {
  /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

  /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_FDCAN_CLK_DISABLE();

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

  /* USER CODE END FDCAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

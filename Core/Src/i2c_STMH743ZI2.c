/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;


/* I2C1 init function */
void MX_I2C1_Init(uint32_t timing_reg)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = timing_reg;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

void MX_I2C2_Init(uint32_t timing_reg)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = timing_reg;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

I2C_HandleTypeDef* I2C_init(const I2C_TypeDef* i2cHandle, uint32_t timing_reg) {
  if (i2cHandle == I2C1) {
	MX_I2C1_Init(timing_reg);
    return &hi2c1;
  }
  if (i2cHandle == I2C2) {
  	MX_I2C2_Init(timing_reg);
    return &hi2c2;
  }

  //NOTE: Fill with other I2C handles when needed

  return &hi2c1; // Default return to avoid compiler warning
}

void HAL_I2C_MspInit_custom(const I2C_TypeDef* i2cHandle, Pin pin, uint8_t af)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(i2cHandle==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */

    GPIO_InitStruct.Pin = pin.block_mask;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af;
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }

  if(i2cHandle==I2C2)
    {
    /* USER CODE BEGIN I2C1_MspInit 0 */

    /* USER CODE END I2C1_MspInit 0 */

    /** Initializes the peripherals clock
    */
      PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
      PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
      if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
      {
        Error_Handler();
      }

      __HAL_RCC_GPIOB_CLK_ENABLE();

      GPIO_InitStruct.Pin = pin.block_mask;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = af;
      HAL_GPIO_Init(pin.block, &GPIO_InitStruct);

      /* I2C1 clock enable */
      __HAL_RCC_I2C2_CLK_ENABLE();
    /* USER CODE BEGIN I2C1_MspInit 1 */

    /* USER CODE END I2C1_MspInit 1 */
    }
}

uint32_t compute_timing(uint32_t freq_hz)
{
    // ======== 100 kHz ========
    if (freq_hz <= 100000) {
        // Standard mode, Rise ≈ 100ns, Fall ≈ 10ns
        return 0x10805D89;
    }

    // ======== 400 kHz ========
    else if (freq_hz <= 400000) {
        // Fast mode, Rise ≈ 100ns, Fall ≈ 10ns
        return 0x00C0216C;
    }

    // ======== 1 MHz ========
    else if (freq_hz <= 1000000) {
        // Fast mode plus, Rise ≈ 60ns, Fall ≈ 10ns
        return 0x00707CBB;
    }

    // Unsupported speed — return safe default (100 kHz)
    return 0x10805D89;
}

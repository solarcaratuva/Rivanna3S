/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
I2C_HandleTypeDef hi2c4;
I2C_HandleTypeDef hi2c5;
I2C_HandleTypeDef hi2c6;

/* I2C1 init function */
void MX_I2C1_Init(uint32_t timing) // <-- This line changed when importing
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = timing; // <-- This line changed when importing
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}
/* I2C2 init function */
void MX_I2C2_Init(uint32_t timing) // <-- This line changed when importing
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = timing; // <-- This line changed when importing
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}
/* I2C3 init function */
void MX_I2C3_Init(uint32_t timing) // <-- This line changed when importing
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = timing; // <-- This line changed when importing
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}
/* I2C4 init function */
void MX_I2C4_Init(uint32_t timing) // <-- This line changed when importing
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = timing; // <-- This line changed when importing
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}
/* I2C5 init function */
void MX_I2C5_Init(uint32_t timing) // <-- This line changed when importing
{

  /* USER CODE BEGIN I2C5_Init 0 */

  /* USER CODE END I2C5_Init 0 */

  /* USER CODE BEGIN I2C5_Init 1 */

  /* USER CODE END I2C5_Init 1 */
  hi2c5.Instance = I2C5;
  hi2c5.Init.Timing = timing; // <-- This line changed when importing
  hi2c5.Init.OwnAddress1 = 0;
  hi2c5.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c5.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c5.Init.OwnAddress2 = 0;
  hi2c5.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c5.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c5.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c5) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c5, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c5, 0) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }
  /* USER CODE BEGIN I2C5_Init 2 */

  /* USER CODE END I2C5_Init 2 */

}
/* I2C6 init function */
void MX_I2C6_Init(uint32_t timing) // <-- This line changed when importing
{

  /* USER CODE BEGIN I2C6_Init 0 */

  /* USER CODE END I2C6_Init 0 */

  /* USER CODE BEGIN I2C6_Init 1 */

  /* USER CODE END I2C6_Init 1 */
  hi2c6.Instance = I2C6;
  hi2c6.Init.Timing = timing; // <-- This line changed when importing
  hi2c6.Init.OwnAddress1 = 0;
  hi2c6.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c6.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c6.Init.OwnAddress2 = 0;
  hi2c6.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c6.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c6.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c6) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c6, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c6, 0) != HAL_OK)
  {
    //Error_Handler(); // <-- This line changed when importing
  }
  /* USER CODE BEGIN I2C6_Init 2 */

  /* USER CODE END I2C6_Init 2 */

}

void HAL_SPI_MspInit_custom(I2C_TypeDef* i2cHandle, Pin pin, uint8_t af) // <-- This line changed when importing
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(i2cHandle == I2C1) // <-- This line changed when importing)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      //Error_Handler(); // <-- This line changed when importing
    }

    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PG13     ------> I2C1_SDA
    PB8     ------> I2C1_SCL
    */
    GPIO_InitStruct.Pin = pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

    GPIO_InitStruct.Pin = pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
  else if(i2cHandle == I2C2) // <-- This line changed when importing)
  {
  /* USER CODE BEGIN I2C2_MspInit 0 */

  /* USER CODE END I2C2_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
    PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      //Error_Handler(); // <-- This line changed when importing
    }

    __HAL_RCC_GPIOF_CLK_ENABLE();
    /**I2C2 GPIO Configuration
    PF0     ------> I2C2_SDA
    PF1     ------> I2C2_SCL
    */
    GPIO_InitStruct.Pin = pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

    /* I2C2 clock enable */
    __HAL_RCC_I2C2_CLK_ENABLE();
  /* USER CODE BEGIN I2C2_MspInit 1 */

  /* USER CODE END I2C2_MspInit 1 */
  }
  else if(i2cHandle == I2C3) // <-- This line changed when importing)
  {
  /* USER CODE BEGIN I2C3_MspInit 0 */

  /* USER CODE END I2C3_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C3;
    PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      //Error_Handler(); // <-- This line changed when importing
    }

    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C3 GPIO Configuration
    PH7     ------> I2C3_SCL
    PB4 (NJTRST)     ------> I2C3_SDA
    */
    GPIO_InitStruct.Pin = pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

    GPIO_InitStruct.Pin = pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

    /* I2C3 clock enable */
    __HAL_RCC_I2C3_CLK_ENABLE();
  /* USER CODE BEGIN I2C3_MspInit 1 */

  /* USER CODE END I2C3_MspInit 1 */
  }
  else if(i2cHandle == I2C4) // <-- This line changed when importing)
  {
  /* USER CODE BEGIN I2C4_MspInit 0 */

  /* USER CODE END I2C4_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C4;
    PeriphClkInit.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      //Error_Handler(); // <-- This line changed when importing
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**I2C4 GPIO Configuration
    PB11     ------> I2C4_SDA
    PD12     ------> I2C4_SCL
    */
    GPIO_InitStruct.Pin = pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

    GPIO_InitStruct.Pin = pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

    /* I2C4 clock enable */
    __HAL_RCC_I2C4_CLK_ENABLE();
  /* USER CODE BEGIN I2C4_MspInit 1 */

  /* USER CODE END I2C4_MspInit 1 */
  }
  else if(i2cHandle == I2C5) // <-- This line changed when importing)
  {
  /* USER CODE BEGIN I2C5_MspInit 0 */

  /* USER CODE END I2C5_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C5;
    PeriphClkInit.I2c5ClockSelection = RCC_I2C5CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      //Error_Handler(); // <-- This line changed when importing
    }

    __HAL_RCC_GPIOH_CLK_ENABLE();
    /**I2C5 GPIO Configuration
    PH5     ------> I2C5_SCL
    PH4     ------> I2C5_SDA
    */
    GPIO_InitStruct.Pin = pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

    /* I2C5 clock enable */
    __HAL_RCC_I2C5_CLK_ENABLE();
  /* USER CODE BEGIN I2C5_MspInit 1 */

  /* USER CODE END I2C5_MspInit 1 */
  }
  else if(i2cHandle == I2C6) // <-- This line changed when importing)
  {
  /* USER CODE BEGIN I2C6_MspInit 0 */

  /* USER CODE END I2C6_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C6;
    PeriphClkInit.I2c6ClockSelection = RCC_I2C6CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      //Error_Handler(); // <-- This line changed when importing
    }

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**I2C6 GPIO Configuration
    PD1     ------> I2C6_SCL
    PD0     ------> I2C6_SDA
    */
    GPIO_InitStruct.Pin = pin.block_mask; // <-- This line changed when importing
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = af; // <-- This line changed when importing
    HAL_GPIO_Init(pin.block, &GPIO_InitStruct); // <-- This line changed when importing

    /* I2C6 clock enable */
    __HAL_RCC_I2C6_CLK_ENABLE();
  /* USER CODE BEGIN I2C6_MspInit 1 */

  /* USER CODE END I2C6_MspInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

I2C_HandleTypeDef* I2C_init(I2C_TypeDef* inst, uint32_t timing) {
    if (inst == I2C1) {
        MX_I2C1_Init(timing);
        return &hi2c1;
    }
    else if (inst == I2C2) {
        MX_I2C2_Init(timing);
        return &hi2c2;
    }
    else if (inst == I2C3) {
        MX_I2C3_Init(timing);
        return &hi2c3;
    }
    else if (inst == I2C4) {
        MX_I2C4_Init(timing);
        return &hi2c4;
    }
    else if (inst == I2C5) {
        MX_I2C5_Init(timing);
        return &hi2c5;
    }
    else if (inst == I2C6) {
        MX_I2C6_Init(timing);
        return &hi2c6;
    }
    else {
        return NULL;
    }
}

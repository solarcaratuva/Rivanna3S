/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "UART.h"
#include "AnalogIn.h"
#include "Timeout.h"
#include "Clock.h"
#include "thread.h"
#include "Timeout.h"
#include "lock.h"
#include "log.h"

 // Address of Flash Banks
  // #define FLASH_BANK1_BASE          (0x08000000UL) /*!< Base address of : (up to 1 MB) Flash Bank1 accessible over AXI                          */
  // #define FLASH_BANK2_BASE          (0x08100000UL) /*!< Base address of : (up to 1 MB) Flash Bank2 accessible over AXI 

  void flash_bank2_ex (void) {

    log_debug("%s", "ENTERING FLASH BANK 2 EXAMPLE");

    //Example Data 256-bit
    uint32_t data[8] = {
        0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x87654321,
        0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD
    };

    //Disable Cache; Crashes???
    // SCB_DisableICache();
    // SCB_DisableDCache();

    // log_debug("%s", "DISABLE CACHE");

    HAL_FLASH_Unlock();
    log_debug("%s", "UNLOCK FLASH BLOCKS");

    FLASH_EraseInitTypeDef erase;
    uint32_t sector_error;

    erase.TypeErase   = FLASH_TYPEERASE_SECTORS;
    erase.Banks       = FLASH_BANK_2;        // <<<<<<<<<< IMPORTANT
    erase.Sector      = 0;        // Bank-2-relative sector
    erase.NbSectors   = 8;        // 8 Sectors for STM32H743
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&erase, &sector_error) != HAL_OK)
    {
        log_debug("%s", "FLASH ERASE FAILED");
        log_debug("%lx", (long unsigned int) sector_error);
    }

    HAL_Delay(200); // Delay after erase (just in case)

    log_debug("%s", "FLASH ERASE END");

    //Mass Erase function call from example HAL flash code
    // FLASH_MassErase(FLASH_VOLTAGE_RANGE_3, FLASH_BANK_2);

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, FLASH_BANK2_BASE, (uint32_t)data);

    HAL_FLASH_Lock();

    // SCB_CleanDCache();
    // SCB_InvalidateICache();
    // SCB_EnableICache();
    // SCB_EnableDCache();
  }

extern "C" void app_main(void *argument)
{
  (void)argument;

  /* USER CODE BEGIN Init */
  log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
  /* USER CODE END Init */

  DigitalOut LED1(PB_0);

  flash_bank2_ex();

  while (1)
  {
    log_debug("%s","HERE");
    HAL_Delay(1000);
    LED1.write(!LED1.read());
  }
}

 
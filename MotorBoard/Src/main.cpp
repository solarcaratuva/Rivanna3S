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

  //Flash Word: 32 Bytes
  void flash_bank2_ex (uint32_t num_flash_words) {

    log_debug("%s", "ENTERING FLASH BANK 2 EXAMPLE");
    log_debug("NUMBER OF FLASH WORDS: %ld", num_flash_words);
    log_debug("DATA SIZE: %ld", num_flash_words * 32);

    uint32_t start;
    uint32_t stop;
    uint32_t erase_time;
    uint32_t write_time; 
    uint32_t total_time;
    uint32_t flash_addr = FLASH_BANK2_BASE;

    //Example Data 256-bit
    uint32_t *test_data = (uint32_t *) malloc(num_flash_words * 8 * sizeof(uint32_t));
    
    for (uint32_t i = 0; i<num_flash_words*8; i++) {
      test_data[i] = i; 
    }

    log_debug("%s", "TEST DATA COMPLETE");

    //Disable Cache; Crashes???
    // SCB_DisableICache();
    // SCB_DisableDCache();

    //Measure Erase
    start = HAL_GetTick();

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
    
    stop = HAL_GetTick();
    erase_time = stop - start;
    log_debug("%s", "FLASH ERASE END");
    log_debug("Erase Time (ms): %ld", erase_time);

    //Mass Erase function call from example HAL flash code
    // FLASH_MassErase(FLASH_VOLTAGE_RANGE_3, FLASH_BANK_2);

    //Start Flash Write
    start = HAL_GetTick();

    for (uint32_t i = 0; i < num_flash_words; i++)
    {
      if (HAL_FLASH_Program(
        FLASH_TYPEPROGRAM_FLASHWORD,
        flash_addr,
        (uint32_t)&test_data[i * 8]) != HAL_OK)
      {
        log_debug("FLASH PROGRAM FAIL @ 0x%08lX", flash_addr);
        log_debug("ERR = 0x%08lX", HAL_FLASH_GetError());
      }

      flash_addr += 32;
    }

    stop = HAL_GetTick();

    write_time = stop - start;
    total_time = erase_time + write_time;
    
    log_debug("Write Time (ms): %ld", write_time);

    log_debug("Total Time (ms): %ld", total_time);


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

 

  for(int i = 0; i < 3; i++ )
  {
    log_debug("%s","-----------------------------------------------");
    flash_bank2_ex(320);
    HAL_Delay(1000);
    LED1.write(!LED1.read());
  }
}

 
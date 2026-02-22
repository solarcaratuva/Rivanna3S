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
#include "fatfs.h"
#include <string.h>

void write_to_sd_card(const char *data)
{
  FATFS FatFs;
  FIL fil;
  FRESULT fres;
  UINT bytesWrote;

  // Mount SD Card
  fres = f_mount(&FatFs, "", 1);
  if (fres != FR_OK)
  {
    log_debug("f_mount error (%d)", fres);
    return;
  }

  // Open or create file
  fres = f_open(&fil, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
  if (fres == FR_OK)
  {
    f_write(&fil, data, strlen(data), &bytesWrote);
    f_close(&fil);
    log_debug("Wrote %u bytes to test.txt", bytesWrote);
  }
  else
  {
    log_debug("f_open error (%d)", fres);
  }

  // Unmount SD Card
  f_mount(NULL, "", 0);
}

void app_main()
{

  /* USER CODE BEGIN Init */
  log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
  /* USER CODE END Init */

  DigitalOut LED1(PB_0);

  while (1)
  {
    log_debug("%s", "HERE");
    HAL_Delay(1000);
    LED1.write(!LED1.read());
  }
}
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
#include "spi.h"
#include <cstring>

void app_main()
{

  /* USER CODE BEGIN Init */
  log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
  /* USER CODE END Init */

  DigitalOut LED1(PB_0);

  log_info("~ SD card demo ~");
  log_info("");

  // Initialize SPI2 for SD card
  MX_SPI2_Init();

  HAL_Delay(1000); // A short delay is important to let the SD card settle

  // Some variables for FatFs
  FATFS FatFs;   // FatFs handle
  FIL fil;       // File handle
  FRESULT fres;  // Result after operations

  // Open the file system
  fres = f_mount(&FatFs, "", 1); // 1=mount now
  if (fres != FR_OK) {
    log_fault("f_mount error (%i)", fres);
    while (1) {
      LED1.write(!LED1.read());
      HAL_Delay(100); // Fast blink on error
    }
  }

  // Let's get some statistics from the SD card
  DWORD free_clusters, free_sectors, total_sectors;
  FATFS* getFreeFs;

  fres = f_getfree("", &free_clusters, &getFreeFs);
  if (fres != FR_OK) {
    log_fault("f_getfree error (%i)", fres);
    while (1) {
      LED1.write(!LED1.read());
      HAL_Delay(100);
    }
  }

  // Formula comes from ChaN's documentation
  total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
  free_sectors = free_clusters * getFreeFs->csize;

  log_info("SD card stats:");
  log_info("%lu KiB total drive space.", total_sectors / 2);
  log_info("%lu KiB available.", free_sectors / 2);

  // Now let's try to open file "test.txt"
  fres = f_open(&fil, "test.txt", FA_READ);
  if (fres != FR_OK) {
    log_warn("f_open error (%i) - test.txt not found", fres);
  } else {
    log_info("I was able to open 'test.txt' for reading!");

    // Read 30 bytes from "test.txt" on the SD card
    BYTE readBuf[30];

    // We can either use f_read OR f_gets to get data out of files
    // f_gets is a wrapper on f_read that does some string formatting for us
    TCHAR* rres = f_gets((TCHAR*)readBuf, 30, &fil);
    if (rres != 0) {
      log_info("Read string from 'test.txt' contents: %s", readBuf);
    } else {
      log_warn("f_gets error (%i)", fres);
    }

    // Be a tidy kiwi - don't forget to close your file!
    f_close(&fil);
  }

  // Now let's try and write a file "write.txt"
  fres = f_open(&fil, "write.txt", FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);
  if (fres == FR_OK) {
    log_info("I was able to open 'write.txt' for writing");
  } else {
    log_fault("f_open error (%i)", fres);
    while (1) {
      LED1.write(!LED1.read());
      HAL_Delay(100);
    }
  }

  // Copy in a string
  BYTE writeBuf[30];
  strncpy((char*)writeBuf, "a new file is made!", 19);
  UINT bytesWrote;
  fres = f_write(&fil, writeBuf, 19, &bytesWrote);
  if (fres == FR_OK) {
    log_info("Wrote %u bytes to 'write.txt'!", bytesWrote);
  } else {
    log_fault("f_write error (%i)", fres);
  }

  // Be a tidy kiwi - don't forget to close your file!
  f_close(&fil);

  // We're done, so de-mount the drive
  f_mount(NULL, "", 0);

  log_info("SD card test complete!");

  while (1)
  {
    // Blink the LED every second to indicate success
    LED1.write(!LED1.read());
    HAL_Delay(1000);
  }
}
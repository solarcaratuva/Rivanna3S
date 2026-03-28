/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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
#include "fatfs.h"

uint8_t retUSER;    /* Return value for USER */
char USERPath[4];   /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FIL USERFile;       /* File object for USER */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */
#include <string.h>

uint8_t FATFS_SD_TestWrite(void) {
    FRESULT res;
    UINT byteswritten;
    char* write_text = "SD card write test via SPI";

    res = f_mount(&USERFatFS, (TCHAR const*)USERPath, 1);
    if (res != FR_OK) return 1;

    res = f_open(&USERFile, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) { 
      f_mount(0, (TCHAR const*)USERPath, 0); // Unmount drive
      return 2; 
    }

    res = f_write(&USERFile, write_text, strlen(write_text), &byteswritten);
    f_close(&USERFile);
    f_mount(0, (TCHAR const*)USERPath, 0);

    if (res == FR_OK && byteswritten > 0){
      return 0;
    } else{
      return 3;
    }
}
/* USER CODE END Application */

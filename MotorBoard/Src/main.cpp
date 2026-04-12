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
#include "can.h"
#include "BPSCanStructs.h"
#include "Rivanna3CanStructs.h"
#include "FiniteQueue.h"
#include "CanInterface.h"
#include "FirmwareUploader.h"



void app_main()
{
    UART USB_UART(PC_12, PD_2, 115200);
    CanInterface main_can = CanInterface(PA_12, PA_11, PA_10, 250000, CanNetwork::Main);
    FirmwareUploader uploader(USB_UART, main_can);
    uploader.start();

    while (1) {
        uploader.run_once();   // blocks until one full upload completes
    }

}



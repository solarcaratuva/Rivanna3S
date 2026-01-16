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

extern "C" void app_main(void *argument)
{
  (void)argument;

  /* USER CODE BEGIN Init */
  log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
  /* USER CODE END Init */

  DigitalOut LED1(PB_0);

  while (1)
  {
    log_debug("%s","HERE");
    HAL_Delay(1000);
    LED1.write(!LED1.read());
  }
}
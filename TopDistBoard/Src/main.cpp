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

void app_main()
{

  /* USER CODE BEGIN Init */
#if defined(STM32G474xx)
  log_configure(DEBUG_LVL, PA_2, PA_3, 921600);
#else
  log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
#endif
  /* USER CODE END Init */

  log_debug("About to call emit py tests");
  // uartcobs_emit_py_tests(); FROM JUST TESTING the encode/decode functions

  DigitalOut LED1(PB_0);

  while (1)
  {
    log_debug("%s","HERE2");
    HAL_Delay(1000);
    LED1.write(!LED1.read());
  }
}

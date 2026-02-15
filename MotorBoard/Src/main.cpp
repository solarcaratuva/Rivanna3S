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

  DigitalOut LED1(PB_0);
  /* USER CODE BEGIN Init */
  log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
  /* USER CODE END Init */

  LED1.write(!LED1.read());

  log_debug("%s","UART SETUP");
  UART test_uart = UART(PF_7, PF_6, 115200);
  char *data = "hello solar car"; 
  char recieved_data[6]; 
  
  LED1.write(!LED1.read());

  while (1)
  {
    log_debug("%s","START UART TEST");
    // test_uart.write((uint8_t *) data, 16);
    log_debug("%s","UART WAITING TO RECIEVE DATA");
    test_uart.read((uint8_t *) recieved_data, 6);
    log_debug("%s","DATA RECIEVED:");
    test_uart.write((uint8_t *) recieved_data, 6);
    LED1.write(!LED1.read());
    HAL_Delay(500);
  }
}
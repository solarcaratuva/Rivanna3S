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
#include "SPI.h"
//#include "M95.h"

extern "C" void app_main(void *argument)
{
  (void)argument;

  /* USER CODE BEGIN Init */
  log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
  /* USER CODE END Init */
  log_debug("%s", "BEGIN TEST");

  
  DigitalOut LED1(PB_0);

  while (1)
  {
    log_debug("%s","HERE");
    HAL_Delay(1000);
    LED1.write(!LED1.read());
  }
  

  // SPI spi4(PE_6, PE_5, PE_2, 100000);

  // uint8_t rx;
  // uint8_t tx;

  // while (1) {
  //   // Read 1 byte
  //   spi4.read(&rx, 1);

  //   // Write it back
  //   tx = rx;
  //   spi4.write(&tx, 1);
  // }

  // uint8_t StatusRegisterValue = 0;
	// uint8_t ReadValue = 0;
	// uint8_t M95Type;
	// uint8_t TxData1 = 'D';
	// uint8_t TxData2 = 'E';
	// uint8_t TxData3 = 'F';

  // M95Type = M95040; DisableChipSelectM95(M95Type);

  // //Disable write-protect and hold
  // DisableWriteProtectM95();
  // DisableHoldM95();

  // //Turn off block protection
  // M95Type = M95040;
  // StatusRegisterValue = ReadStatusRegisterM95(M95Type);
  // StatusRegisterValue = StatusRegisterValue & ~(0x0C);
  // WriteStatusRegisterM95(M95Type, StatusRegisterValue);

  // while(1) {
  //   M95Type = M95040;
	//   WriteByteM95(M95Type, 0x00000001, TxData1);
	//   StatusRegisterValue = 0;
	//   StatusRegisterValue = ReadStatusRegisterM95(M95Type);
	//   while (StatusRegisterValue & 0x01) {
	// 	  StatusRegisterValue = ReadStatusRegisterM95(M95Type);
	//   }
	//   ReadValue = 0;
	//   ReadValue = ReadByteM95(M95Type, 0x00000001);
  //   log_debug("%c", ReadValue);

	//   if (TxData1 == ReadValue) {
	// 	  HAL_Delay(1);
	//   }
  // }
}
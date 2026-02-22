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
//#include "main.h"

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

#define M95040		0
#define M95256		1
#define M95M04		2

// SPI commands
#define SPI_WREN	0x06
#define SPI_WRDI	0x04
#define SPI_RDSR	0x05
#define SPI_WRSR	0x01
#define SPI_READ	0x03
#define SPI_WRITE	0x02
#define SPI_RDID	0x83
#define SPI_WRID	0x82
#define SPI_RDLS	0x83
#define SPI_LID		0x82

SPI spi4(PE_6, PE_5, PE_2, 100000);


void EnableChipSelectM95(uint8_t M95Type) //DONE
{
	switch (M95Type) {
		case M95040:
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
			break;
		// case M95256:
		// 	HAL_GPIO_WritePin(U6_Sn_GPIO_Port, U6_Sn_Pin, GPIO_PIN_RESET);
		// 	break;
		// case M95M04:
		// 	HAL_GPIO_WritePin(GPIOB, U7_Sn_Pin, GPIO_PIN_RESET);
		// 	break;
		default:
			return;
	}

	return;
}

void DisableChipSelectM95(uint8_t M95Type)	//DONE
{
	switch (M95Type) {
			case M95040:
				HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET);
				break;
			// case M95256:
			// 	HAL_GPIO_WritePin(U6_Sn_GPIO_Port, U6_Sn_Pin, GPIO_PIN_SET);
			// 	break;
			// case M95M04:
			// 	HAL_GPIO_WritePin(GPIOB, U7_Sn_Pin, GPIO_PIN_SET);
			// 	break;
			default:
				return;
		}
}

void DisableWriteProtectM95(void) //DONE
{
	//set the Write Protect pin high
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_6, GPIO_PIN_SET);
	return;
}

void DisableHoldM95(void) //DONE
{
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_5, GPIO_PIN_SET);
	return;
}

void WriteEnableM95(uint8_t M95Type) //DONE
{
	uint8_t Command = SPI_WREN;
	//Enable Chip Select
	EnableChipSelectM95(M95Type);

	//Send Command
	//spi4.write(&Command);
    spi4.write(&Command, 1);

	//Disable Chip Select to complete command
	DisableChipSelectM95(M95Type);

}

void WriteStatusRegisterM95(uint8_t M95Type, uint8_t WriteData)
{
	uint8_t Command = SPI_WRSR;

	//Enable Write
	WriteEnableM95(M95Type);

	//Enable Chip Select
	EnableChipSelectM95(M95Type);

	//Send Command and Status Register State
	spi4.write(&Command, 1);
	spi4.write(&WriteData, 1);

	//Disable Chip Select
	DisableChipSelectM95(M95Type);


}

uint8_t ReadStatusRegisterM95(uint8_t M95Type)
{
    uint8_t Command = SPI_RDSR;
	uint8_t ReturnValue = 0;
    
	//Enable Chip Select
	EnableChipSelectM95(M95Type);
    
    log_debug("%s", "SR:1");
	//Send Command & Recieve Data
	spi4.write(&Command, 1);
    log_debug("%s", "SR:2");
	spi4.read(&ReturnValue, 1);
    log_debug("%s", "SR:3");


	//Disable Chip Select
	DisableChipSelectM95(M95Type);

	return ReturnValue;
}

void WriteByteM95(uint8_t M95Type, uint8_t Address, uint8_t WriteData)//DONE
{

	uint8_t Command = SPI_WRITE;

	//Enable Write
	WriteEnableM95(M95Type);

	//Enable Chip Select
	EnableChipSelectM95(M95Type);



	switch (M95Type) {
				case M95040:
					Command = SPI_WRITE | ((Address & 0x100)>>5);
					//Send Command, Address, & Data
					spi4.write(&Command, 1);

					spi4.write(&Address, 1);
					break;
				// case M95256:
				// 	//Send Command, Address, & Data
				// 	spi4.write(&Command, 1);

				// 	spi4.write((uint8_t*) Address>>8, 1);
				// 	spi4.write((uint8_t*) Address, 1);
				// 	break;
				// case M95M04:
				// 	//Send Command, Address, & Data
				// 	spi4.write(&Command, 1);

				// 	spi4.write((uint8_t*) Address>>16, 1);
				// 	spi4.write((uint8_t*) Address>>8, 1);
				// 	spi4.write((uint8_t*) Address, 1);
				// 	break;
				default:
					return;
	}

	spi4.write(&WriteData, 1);

	//Disable Chip Select
	DisableChipSelectM95(M95Type);


}

uint8_t ReadByteM95(uint8_t M95Type, uint8_t Address) //DONE
{
	uint8_t Command = SPI_READ;
	uint8_t ReturnValue = 0;

	switch (M95Type) {
				case M95040:
					//Enable Chip Select
					EnableChipSelectM95(M95Type);
					Command = SPI_READ | ((Address & 0x100)>>5); //add A8 bit to instruction
					//Send commands and recieve
					spi4.write(&Command, 1);
					spi4.write(&Address, 1);
					spi4.read(&ReturnValue, 1);

					//Disable Chip Select
					DisableChipSelectM95(M95Type);
					break;

				// case M95256:
				// 	//Enable Chip Select
				// 	EnableChipSelectM95(M95Type);

				// 	spi4.write(&Command, 1);
				// 	spi4.write(&(uint8_t) Address>>8, 1);
				// 	spi4.write(&(uint8_t) Address, 1);
				// 	spi4.read(&ReturnValue, 1);

				// 	//Disable Chip Select
				// 	DisableChipSelectM95(M95Type);
				// 	break;
				// case M95M04:
				// 	//Enable Chip Select
				// 	EnableChipSelectM95(M95Type);

				// 	spi4.write(&Command, 1);
				// 	spi4.write(&Address>>16, 1);
				// 	spi4.write(&Address>>8, 1);
				// 	spi4.write(&Address, 1);
				// 	spi4.read(&ReturnValue, 1);

				// 	//Disable Chip Select
				// 	DisableChipSelectM95(M95Type);
				// 	break;

				default:
					return 0;
	}


	return ReturnValue;
}

void app_main()
{
  /* USER CODE BEGIN Init */
  log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
  /* USER CODE END Init */
  log_debug("%s", "BEGIN TEST");

  
  // DigitalOut LED1(PB_0);

  // while (1)
  // {
  //   log_debug("%s","HERE");
  //   HAL_Delay(1000);
  //   LED1.write(!LED1.read());
  // }
  

  //SPI spi4(PE_6, PE_5, PE_2, 100000);

  // uint8_t rx;
  // uint8_t tx;

  // while (1) {
  //   // Read 1 byte
  //   spi4.read(&rx, 1);

  //   // Write it back
  //   tx = rx;
  //   spi4.write(&tx, 1);
  // }

  uint8_t StatusRegisterValue = 0;
  uint8_t ReadValue = 0;
  uint8_t M95Type;
  uint8_t TxData1 = 'D';
  uint8_t TxData2 = 'E';
  uint8_t TxData3 = 'F';

  log_debug("%s", "ONE");

  M95Type = M95040; DisableChipSelectM95(M95Type);

  //Disable write-protect and hold
  DisableWriteProtectM95();
  DisableHoldM95();

  log_debug("%s", "TWO");

  //Turn off block protection
  M95Type = M95040;
  StatusRegisterValue = ReadStatusRegisterM95(M95Type);
  StatusRegisterValue = StatusRegisterValue & ~(0x0C);
  log_debug("READ SR STATUS: %x", StatusRegisterValue);
  WriteStatusRegisterM95(M95Type, StatusRegisterValue);

  log_debug("%s", "THREE");

  while(1) {
    log_debug("%s", "HERE");
    HAL_Delay(1000);
    M95Type = M95040;
	  WriteByteM95(M95Type, 0x00000001, TxData1);
	  StatusRegisterValue = 0;
	  StatusRegisterValue = ReadStatusRegisterM95(M95Type);
	  while (StatusRegisterValue & 0x01) {
		  StatusRegisterValue = ReadStatusRegisterM95(M95Type);
	  }
	  ReadValue = 0;
	  ReadValue = ReadByteM95(M95Type, 0x00000001);
      log_debug("%c", ReadValue);

	  if (TxData1 == ReadValue) {
		  HAL_Delay(1);
	  }
  }
}
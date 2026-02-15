#include "M95.h"
#include "SPI.h"
#include "stm32h7xx_hal.h"
#include "log.h"

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

void WriteStatusRegisterM95(uint8_t M95Type, uint8_t WriteData) //DONE
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

uint8_t ReadStatusRegisterM95(uint8_t M95Type) //DONE
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
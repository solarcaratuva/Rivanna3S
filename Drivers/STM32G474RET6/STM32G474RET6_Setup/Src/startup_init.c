#include "stm32_hal.h"
#include "fdcan.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"

#define STARTUP_SPI_BAUD_MHZ 1U
#define STARTUP_I2C_BAUD_HZ 100000U
#define STARTUP_FDCAN_BAUD_HZ 500000U
#define STARTUP_UART_BAUD_HZ 115200U

void SystemClock_Config(void);
void Error_Handler(void);

void startup_init(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_SPI1_Init(STARTUP_SPI_BAUD_MHZ);
  MX_SPI2_Init(STARTUP_SPI_BAUD_MHZ);
  MX_SPI3_Init(STARTUP_SPI_BAUD_MHZ);
  MX_FDCAN1_Init(STARTUP_FDCAN_BAUD_HZ);
  MX_FDCAN2_Init(STARTUP_FDCAN_BAUD_HZ);
  MX_FDCAN3_Init(STARTUP_FDCAN_BAUD_HZ);
  MX_I2C1_Init(STARTUP_I2C_BAUD_HZ);
  MX_I2C2_Init(STARTUP_I2C_BAUD_HZ);
  MX_I2C3_Init(STARTUP_I2C_BAUD_HZ);
  MX_I2C4_Init(STARTUP_I2C_BAUD_HZ);
  MX_LPUART1_UART_Init(STARTUP_UART_BAUD_HZ);
  MX_UART4_Init(STARTUP_UART_BAUD_HZ);
  MX_UART5_Init(STARTUP_UART_BAUD_HZ);
  MX_USART1_UART_Init(STARTUP_UART_BAUD_HZ);
  MX_USART2_UART_Init(STARTUP_UART_BAUD_HZ);
  MX_USART3_UART_Init(STARTUP_UART_BAUD_HZ);
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

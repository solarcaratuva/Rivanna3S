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
#include "i2c.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
Timeout t;
DigitalOut pin(PB_0);

void onTimeout() {
  // Turn pin on for 3 seconds
  pin.write(true);
  vTaskDelay(pdMS_TO_TICKS(3000));
  pin.write(false);
}

void timeout_test_task(void *argument) {
  // Attach a 2 second timeout
  t.attach(onTimeout, 2000);

  // Wait 1 second, then refresh
  vTaskDelay(pdMS_TO_TICKS(1000));
  t.refresh();

  // Wait 3 seconds, callback should trigger
  vTaskDelay(pdMS_TO_TICKS(3000));

  // Stop timer
  t.stop();

  // End task
  vTaskDelete(NULL);
}
/* USER CODE END 0 */
DigitalOut pin1(PB_0);
DigitalOut pin2(PA_5);
Clock Timer;

void test_get_current_time() {
    while (1) {
        uint32_t t1 = Timer.get_current_time();
        if (t1 > 10000) { pin1.write(true); }
        else { pin1.write(false); }
    }
}

void test_logging() {
  float x = 0.55;
  float y = 10989.021;
  while (1) {
    log_debug("Debug, SHOULD NOT PRINT");
    log_info("Info x: %f y: %f", x, y);
    log_warn("warn level, this is a massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflow massive overflowmassive overflow!");
    log_fault("Fault Level, should Print, y: %f and x: %f", y , x);
    x += 1;
    y -= 1;
  }
}

void app_main()
{

  /* USER CODE BEGIN Init */
  log_configure(DEBUG_LVL, PD_8, PD_9, 921600);
  /* USER CODE END Init */

  DigitalOut LED1(PB_0);

  // AnalogIn analogInput1(PF_5);
  // AnalogIn analogInput2(PF_10);

  I2C test_i2c(PF_0, PF_1, I2C::FAST);
  uint8_t test_data[2] = {6, 7};
  uint8_t received_data[3];

  while (1)
  {
    log_debug("%s","HERE");
    HAL_Delay(1000);
    LED1.write(!LED1.read());
  }

  /* USER CODE BEGIN 2 */
  Thread thread;

  thread.start(test_logging);
  xTaskCreate(timeout_test_task, "Timeout Test", 128, NULL, 2, NULL);
  /* USER CODE END 2 */
}

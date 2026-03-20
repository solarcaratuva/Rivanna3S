#include "startup.h"
#include "thread.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32_hal.h"
#include "DigitalOut.h"


int main(void)
{
  startup_init();
  DigitalOut LED1(PC_1);
  LED1.write(1);

  // Thread main;
  // main.start(app_main);
  // vTaskStartScheduler();

  while (1)
  {
      HAL_Delay(1000);
      LED1.write(!LED1.read());
  }
}

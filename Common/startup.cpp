#include "startup.h"
#include "thread.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32h7xx_hal.h"



int main(void)
{
  startup_init();

  Thread main;
  main.start(app_main);
  vTaskStartScheduler();

  while (1)
  {
  }
}

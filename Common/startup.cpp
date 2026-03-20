#include "startup.h"
#include "thread.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32_hal.h"
#include "DigitalIn.h"
#include "DigitalOut.h"

#define PC_1 (Pin){GPIOC, 1 << 1, 1ULL << 8}

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

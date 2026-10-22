#include "startup.h"
#include "thread.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32_hal.h"
#include "DigitalOut.h"

int main(void)
{
  // Report these faults separately instead of escalating them to HardFault.
  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk
              | SCB_SHCSR_USGFAULTENA_Msk;

  startup_init();

  Thread main;
  main.start(app_main);
  vTaskStartScheduler();

  while (1)
  {
  }
}

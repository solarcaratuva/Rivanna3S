#include "startup.h"
#include "thread.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32_hal.h"
#include "DigitalOut.h"

int main(void)
{
  // Enable separate handlers for MemManage, BusFault, and UsageFault
  // so they report their own blink codes instead of escalating to HardFault.
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

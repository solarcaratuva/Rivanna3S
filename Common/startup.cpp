#include "startup.h"
#include "thread.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32_hal.h"
#include "DigitalOut.h"

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  (void)pcTaskName;
  
  // --- STACK OVERFLOW: SOLID ON ---
  DigitalOut LED(PC_1);
  LED.write(1);

  vTaskSuspendAll();
  taskDISABLE_INTERRUPTS();
  while (1);
}

int main(void)
{
  startup_init();
  DigitalOut LED1(PC_1);
  LED1.write(0);

  Thread main;
  main.start(app_main);
  vTaskStartScheduler();

  while (1)
  {
  }
}

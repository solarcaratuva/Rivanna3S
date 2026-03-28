#include "FreeRTOS.h"
#include "task.h"
#include "pinmap.h"
#include "DigitalOut.h"
#include "UART.h"

/**
 * STABLE DIAGNOSTIC (PC_1):
 * - TOGGLE: One successful 6-byte message received and echoed.
 * - RAPID BLINK (500ms): UART Hardware Error occurred (recovered).
 * - DOUBLE BLINK: UART Initialization failed (check pins).
 * - SOLID ON: Stack Overflow (handled in startup.cpp).
 */
void app_main()
{
    DigitalOut LED(PC_1); 
    
    // Ensure pins match your wiring!
    UART test(PA_2, PA_3, 115200); 
    uint8_t data[6];

    if (!test.initialized) {
        while(1) {
            LED.write(1); vTaskDelay(100); LED.write(0); vTaskDelay(100);
            LED.write(1); vTaskDelay(100); LED.write(0); vTaskDelay(700);
        }
    }

    while (1)
    {
        // Try to read 6 bytes
        int result = test.read(data, 6);

        if (result == 0) {
            // SUCCESS: Toggle the LED
            LED.write(!LED.read());

            // Echo it back
            test.write(data, 6);

            // Stability delay
            vTaskDelay(1);
        } 
        else {
            // --- RECOVERABLE ERROR: Rapid blink then continue ---
            for(int i=0; i<5; i++) {
                LED.write(1); vTaskDelay(50);
                LED.write(0); vTaskDelay(50);
            }
        }
    }
}

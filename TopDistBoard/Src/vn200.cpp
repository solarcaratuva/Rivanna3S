#include "UART.h"
#include "pindef.h"
#include "log.h"
#include "vn200.h"
#include "Clock.h"
#include "string.h"

UART serial(VN200_UART_TX, VN200_UART_RX, 115200);

bool VN200::init()
{
    Clock timer;

    // step 1: async output enable
    uint8_t message[] = "$VNASY,0*XX";
    serial.write(message, sizeof(message));

    uint8_t buffer[10];
    serial.read(buffer, sizeof(buffer), 1000); // Read with 1s timeout
    if (!strcmp("$VNERR", (const char *)buffer))
    {
        return false; // init fails if VN200 returns VNERR
    }

    // step 2: Binary Output Message Configuration #1
    /*
    75     Register 75
    1      UART-1
    8      800 Hz / 8 = 100 Hz
    0B     Common + Time + GNSS groups
    11EA   Common outputs
    0200   TimeStatus
    0618   GNSS status/uncertainty outputs
    */
    uint8_t message[] = "$VNWRG,75,1,8,0B,11EA,0200,0618*XX";
    serial.write(message, sizeof(message));

    uint8_t buffer[10];
    serial.read(buffer, sizeof(buffer), 1000); // Read with 1s timeout
    if (!strcmp("$VNERR", (const char *)buffer))
    {
        return false; // init fails if VN200 returns VNERR
    }

    // step 3:
    uint8_t message[] = "$VNASY,1*XX";
    serial.write(message, sizeof(message));

    uint8_t buffer[10];
    serial.read(buffer, sizeof(buffer), 1000); // Read with 1s timeout
    if (!strcmp("$VNERR", (const char *)buffer))
    {
        return false; // init fails if VN200 returns VNERR
    }

    return true;
}

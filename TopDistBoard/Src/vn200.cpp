#include "UART.h"
#include "log.h"
#include "vn200.h"
#include "Clock.h"
#include "string.h"


VN200::VN200(Pin tx, Pin rx, uint32_t baud)
    : serial(tx, rx, baud)
{


}

bool VN200::init()
{
    // step 1: async output enable
    if (send_ascii_command("$VNASY,0*XX\r\n") == false) { return false; }
    

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
    if (send_ascii_command("$VNWRG,75,1,8,0B,11EA,0200,0618*XX\r\n") == false) { return false; }

    // step 3:
    if (send_ascii_command("$VNASY,1*XX\r\n") == false) { return false; }

    return true;
}

bool VN200::send_ascii_command(const char *cmd){
    serial.write((uint8_t *) cmd, strlen(cmd));

    char errorPrefix[] = "$VNERR"; //errors start with this prefix
    uint8_t buffer[32] = {};
    for (size_t i = 0; i < sizeof(buffer)-1; i++){
        int success = serial.read(buffer+i, 1, 1000); // Read one byte at a time with 1s timeout per byte
        if (success != 0){ //timeout occoured when reading byte
            return false;
        }
        if (buffer[i] == '\n') { //reached end of return message
            break;
        }
    }
    
    if (strncmp(errorPrefix, (const char *)buffer,sizeof(errorPrefix)-1) == 0)
    {
        return false; // error if VN200 returns VNERR
    }
    return true;
}
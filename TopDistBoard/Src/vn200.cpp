#include "UART.h"
#include "log.h"
#include "vn200.h"
#include "Clock.h"
#include "string.h"


VN200::VN200(Pin tx, Pin rx, uint32_t baud)
    : serial(tx, rx, baud)
{
    crc_error = 0;

    //initialize memory to 0
    memset(&ang_rate, 0, sizeof(ang_rate));
    memset(&accel, 0, sizeof(accel));
    memset(&pos, 0, sizeof(pos));
    memset(&vel, 0, sizeof(vel));
    memset(&status, 0, sizeof(status));

}

// Calculates the 16-bit CRC checksum for the given byte sequence .
uint16_t VN200:: compute_crc16 ( const uint8_t data [], const uint16_t length){
    unsigned int i;
    unsigned short crc = 0;
    for(i=0; i<length; i++){
        crc = ( unsigned char)(crc >> 8) | (crc << 8);
        crc ^= data[i];
        crc ^= ( unsigned char)(crc & 0xff) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0x00ff) << 5;
    }
    return crc;
}

const uint32_t VN200::get_crc_error_count(){
    return crc_error;
}

bool VN200::poll(){
     //read bytes need to define if want to be blocking or passive checking
    uint8_t byte;
    int result = serial.read(&byte, 1, 10);

    //blocking for 10 ms
    if (result == 0){
        //byte received do something with it

        return true;
    }
    else{
        //otherwise
        return false;
    }
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

//getters
const VN200AngularRate& VN200::get_latest_sample_angular_rate(){
    return ang_rate;
}

const VN200Acceleration& VN200::get_latest_sample_acceleration(){
    return accel;
}

const VN200Position& VN200::get_latest_sample_position(){
    return pos;
}

const VN200Velocity& VN200::get_latest_sample_velocity(){
    return vel;
}  

const VN200Status& VN200::get_latest_sample_status(){
    return status;
}
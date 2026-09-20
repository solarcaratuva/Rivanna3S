#include "UART.h"
#include "log.h"
#include "vn200.h"
#include "Clock.h"
#include "string.h"

/* ---- Binary packet parser constants ----------------------------------------
 *
 * Packet layout (from VN-200 binary output protocol):
 *   idx 0      0xFA sync byte
 *   idx 1      group mask byte (0x0B = Common + Time + GNSS)
 *   idx 2-3    Common field mask, little-endian (0x11EA)
 *   idx 4-5    Time field mask,   little-endian (0x0200)
 *   idx 6-7    GNSS field mask,   little-endian (0x0018)
 *   idx 8-92   payload (85 bytes)
 *   idx 93-94  CRC16 (little-endian)
 *
 * Payload field map (ascending bit order, offsets relative to payload start):
 *   ip 0-7     TimeGps      (uint64)
 *   ip 8-19    Ypr          (3 floats)
 *   ip 20-31   AngularRate  (3 floats)
 *   ip 32-55   PosLla       (3 doubles: lat, lon, alt)
 *   ip 56-67   VelNed       (3 floats)
 *   ip 68-79   Accel        (3 floats)
 *   ip 80-81   InsStatus    (uint16)
 *   ip 82      TimeStatus   (uint8)
 *   ip 83      NumSats      (uint8)
 *   ip 84      GnssFix      (uint8)
 *
 * Dropped from GNSS group (mask 0x0018 instead of full 0x0618):
 *   PosU  (GNSS bit 9, 12 B, 3 floats) -> would occupy ip 85-96, rx_buffer idx 93-104
 *   VelU  (GNSS bit 10, 4 B,  1 float) -> would occupy ip 97-100, rx_buffer idx 105-108
 *   CRC would then move to idx 109-110, packet length -> 111
 *
 * To re-enable: change GNSS mask 0x0018 -> 0x0618 in init(), update the three
 * constants below, AND raise the baud rate ($VNWRG,5,921600 in init() plus the
 * constructor baud argument in main.cpp) since 111 B @ 100 Hz is ~96% of
 * 115200 (unsafe) vs ~42% at 921600. rx_buffer is already sized for 111 so
 * re-enabling requires no memory changes.
 --------------------------------------------------------------------------- */
#define HEADER_END   8    // sync + groups + 3 field masks
#define PAYLOAD_END  93   // HEADER_END + 85 payload bytes
#define PACKET_LEN   95   // HEADER_END + payload (85) + CRC (2)


VN200::VN200(Pin tx, Pin rx, uint32_t baud)
    : serial(tx, rx, baud)
{
    crc_error = 0;
    header_error = 0;
    parser_state = WAIT_SYNC;
    rx_index = 0;

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
void VN200::handle_byte(uint8_t byte){
    
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
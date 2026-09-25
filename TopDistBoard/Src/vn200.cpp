#include "UART.h"
#include "log.h"
#include "vn200.h"
#include "Clock.h"
#include "string.h"

/* ---- Binary packet parser constants ----------------------------------------
 *
 * Link runs at 921600 baud
 *
 * Packet layout (from VN-200 binary output protocol):
 *   idx 0        0xFA sync byte
 *   idx 1        group mask byte (0x0B = Common + Time + GNSS)
 *   idx 2-3      Common field mask, little-endian (0x11EA)
 *   idx 4-5      Time field mask,   little-endian (0x0200)
 *   idx 6-7      GNSS field mask,   little-endian (0x0618)
 *   idx 8-108    payload (101 bytes)
 *   idx 109-110  CRC16
 *
 * Payload field map (ascending bit order, offsets relative to payload start):
 *   Common group (0x11EA, 82 B)
 *     ip 0-7      TimeGps      (uint64)
 *     ip 8-19     Ypr          (3 floats)
 *     ip 20-31    AngularRate  (3 floats)
 *     ip 32-55    PosLla       (3 doubles: lat, lon, alt)
 *     ip 56-67    VelNed       (3 floats)
 *     ip 68-79    Accel        (3 floats)
 *     ip 80-81    InsStatus    (uint16)
 *   Time group (0x0200, 1 B)
 *     ip 82       TimeStatus   (uint8)
 *   GNSS group (0x0618, 18 B)
 *     ip 83       NumSats      (uint8)
 *     ip 84       GnssFix      (uint8)
 *     ip 85-96    PosU         (3 floats, position uncertainty, m)   //currently not stored will be updated once we switch to fdcan
 *     ip 97-100   VelU         (1 float,  velocity uncertainty, m/s) //currently not stored will be updated once we switch to fdcan
 *
 * CRC is CRC-16/XMODEM (poly 0x1021, init 0x0000, non-reflected, no final XOR),
 * computed from idx 1 -- it does NOT cover the 0xFA sync byte. It is stored
 * BIG-endian at idx 109-110, unlike the field masks above, which are little-
 * endian.
 --------------------------------------------------------------------------- */
#define HEADER_END 8    // sync + groups + 3 field masks
#define PAYLOAD_END 109 // HEADER_END + 101 payload bytes
#define PACKET_LEN 111  // HEADER_END + payload (101) + CRC (2)

VN200::VN200(Pin tx, Pin rx, uint32_t baud)
    : serial(tx, rx, baud)
{
    crc_error = 0;
    header_error = 0;
    parser_state = WAIT_SYNC;
    rx_index = 0;
    sequence = 0;

    // initialize memory to 0
    memset(&ang_rate, 0, sizeof(ang_rate));
    memset(&accel, 0, sizeof(accel));
    memset(&pos, 0, sizeof(pos));
    memset(&vel, 0, sizeof(vel));
    memset(&status, 0, sizeof(status));
}

bool VN200::init()
{
    // step 1: async output enable
    if (send_ascii_command("$VNASY,0*XX\r\n") == false)
    {
        return false;
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
    if (send_ascii_command("$VNWRG,75,1,8,0B,11EA,0200,0618*XX\r\n") == false)
    {
        return false;
    }

    // step 3:
    if (send_ascii_command("$VNASY,1*XX\r\n") == false)
    {
        return false;
    }

    return true;
}

bool VN200::poll()
{
    // read bytes need to define if want to be blocking or passive checking
    uint8_t byte;
    int result = serial.read(&byte, 1, 10);

    // blocking for 10 ms
    if (result == 0)
    {
        // byte received do something with it

        return true;
    }
    else
    {
        // otherwise
        return false;
    }
}
/*
 * Confirms that a candidate buffer really is a VN-200 binary packet.
 *
 * Takes the WHOLE packet (rx_buffer), not just the payload: the header bytes
 * live at idx 1-7 and the CRC covers idx 1-110, both outside the payload.
 *
 * Returns false without side effects; poll() owns the error counters and the
 * decision to resync.
 */
bool VN200::validate_packet(const uint8_t *packet)
{
    // 1. Sync byte: packet[0] == 0xFA.
    // 2. Group byte: packet[1] == 0x0B (Common + Time + GNSS).
    // 3. Field masks, little-endian:
    //      idx 2-3  Common == 0x11EA
    //      idx 4-5  Time   == 0x0200
    //      idx 6-7  GNSS   == 0x0618
    //    Build each with packet[n] | (packet[n+1] << 8).
    //    A mismatch at step 2 or 3 almost always means we latched onto a 0xFA
    //    that was payload data, not a packet start. Caller: header_error++.

    // 4. CRC: compute_crc16(packet + 1, PACKET_LEN - 1) == 0.
    //    110 bytes, idx 1 through 110: the sync byte is excluded and the stored
    //    CRC is included, which makes a good packet come out to 0.
    //    Equivalent long form: CRC over idx 1..108 (length PAYLOAD_END - 1)
    //    compared against (packet[109] << 8) | packet[110] -- big-endian.
    //    Caller: crc_error++.

    // 5. True only if every check passed.
    return false; // TODO
}

/*
 * Unpacks a validated payload into the five sample structs.
 *
 * `payload` is rx_buffer + HEADER_END.
 *
 * Byte order: the VN-200 sends little-endian and the STM32 is little-endian,
 * so no swapping is needed. Copy with memcpy.
 */
void VN200::decode_payload(const uint8_t *payload)
{ 
    // ---- Common group (ip 0-81) ----
    // ip 0-7     TimeGps      (uint64)  -- no home in the structs; skip or add one
    // ip 8-19    Ypr          (3 floats) -- not stored; yaw/pitch/roll if ever needed
    // ip 20-31   AngularRate  -> ang_rate.gyro_x / gyro_y / gyro_z   (rad/s)
    // ip 32-55   PosLla       (3 doubles)
    //              [0] lat -> pos.latitude   (deg)
    //              [1] lon -> pos.longitude  (deg)
    //              [2] alt -> status.altitude (m) -- double narrowed to float,
    //                         fine for altitude but it is a real conversion
    // ip 56-67   VelNed       -> vel.vel_n / vel_e / vel_d           (m/s)
    // ip 68-79   Accel        -> accel.accel_x / accel_y / accel_z   (m/s^2)
    // ip 80-81   InsStatus    (uint16)  -> status.ins_status

    // ---- Time group (ip 82) ----
    // ip 82      TimeStatus   (uint8)   -> status.time_status

    // ---- GNSS group (ip 83-100) ----
    // ip 83      NumSats      (uint8)   -> vel.num_sats
    // ip 84      GnssFix      (uint8)   -> vel.gnss_fix
    // ip 85-96   PosU         (3 floats, m)    -- no struct field yet
    // ip 97-100  VelU         (1 float,  m/s)  -- no struct field yet

    // Stamp every struct decoded from this packet with the same `sequence`
    // value, so consumers can tell which CAN messages came from one sample.
    // Increment once per accepted packet and let it wrap.
}

// Calculates the 16-bit CRC checksum for the given byte sequence .
uint16_t VN200::compute_crc16(const uint8_t data[], const uint16_t length)
{
    unsigned int i;
    unsigned short crc = 0;
    for (i = 0; i < length; i++)
    {
        crc = (unsigned char)(crc >> 8) | (crc << 8);
        crc ^= data[i];
        crc ^= (unsigned char)(crc & 0xff) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0x00ff) << 5;
    }
    return crc;
}

bool VN200::send_ascii_command(const char *cmd)
{
    serial.write((uint8_t *)cmd, strlen(cmd));

    char errorPrefix[] = "$VNERR"; // errors start with this prefix
    uint8_t buffer[32] = {};
    for (size_t i = 0; i < sizeof(buffer) - 1; i++)
    {
        int success = serial.read(buffer + i, 1, 1000); // Read one byte at a time with 1s timeout per byte
        if (success != 0)
        { // timeout occoured when reading byte
            return false;
        }
        if (buffer[i] == '\n')
        { // reached end of return message
            break;
        }
    }

    if (strncmp(errorPrefix, (const char *)buffer, sizeof(errorPrefix) - 1) == 0)
    {
        return false; // error if VN200 returns VNERR
    }
    return true;
}

// getters
const VN200AngularRate &VN200::get_latest_sample_angular_rate()
{
    return ang_rate;
}

const VN200Acceleration &VN200::get_latest_sample_acceleration()
{
    return accel;
}

const VN200Position &VN200::get_latest_sample_position()
{
    return pos;
}

const VN200Velocity &VN200::get_latest_sample_velocity()
{
    return vel;
}

const VN200Status &VN200::get_latest_sample_status()
{
    return status;
}

const uint32_t VN200::get_crc_error_count()
{
    return crc_error;
}
#ifndef VN200_H
#define VN200_H

#include <cstdint>
#include "pindef.h"
#include "peripheralmap.h"
#include "UART.h"

struct VN200AngularRateSample {
    float gyro_x;       // rad/s
    float gyro_y;       // rad/s
    float gyro_z;       // rad/s
    uint8_t sequence;
};

struct VN200AccelerationSample {
    float accel_x;      // m/s^2
    float accel_y;      // m/s^2
    float accel_z;      // m/s^2
    uint8_t sequence;
};

struct VN200PositionSample {
    double latitude;    // degrees
    double longitude;   // degrees
    uint8_t sequence;
};

struct VN200VelocitySample {
    float vel_n;        // m/s
    float vel_e;        // m/s
    float vel_d;        // m/s
    uint8_t num_sats;
    uint8_t gnss_fix;
    uint8_t sequence;
};

struct VN200StatusSample {
    float altitude;     // meters
    uint16_t ins_status;
    uint8_t time_status;
    uint8_t sequence;
};

//define the VN200 class
class VN200 {
public:
    VN200(Pin tx, Pin rx, uint32_t baud);
    bool init();
    const VN200AngularRateSample& get_latest_sample_angular_rate();
    const VN200AccelerationSample& get_latest_sample_acceleration();
    const VN200PositionSample& get_latest_sample_position();
    const VN200VelocitySample& get_latest_sample_velocity();
    const VN200StatusSample& get_latest_sample_status();
    const uint32_t get_crc_error_count();
    const uint32_t get_header_error_count();    
    bool poll();

private:
    bool send_ascii_command(const char *cmd);
    void decode_payload(const uint8_t *payload);
    bool validate_packet(const uint8_t *packet);
    uint16_t compute_crc16(const uint8_t *data, uint16_t length);
    UART serial;
    uint32_t crc_error;
    uint32_t header_error;
    VN200AngularRateSample ang_rate;
    VN200AccelerationSample accel;
    VN200PositionSample pos;
    VN200VelocitySample vel;
    VN200StatusSample status;

    // ---- Binary packet state ----
    // One full packet. poll() hunts VN200_SYNC_BYTE, bulk-reads the remainder,
    // and validates it. A failed frame may be reconstructed from a later sync
    // byte before poll() resumes searching on a later call.
    uint8_t  rx_buffer[111];   // one full packet (PACKET_LEN in vn200.cpp)
    uint8_t sequence;
};



#endif

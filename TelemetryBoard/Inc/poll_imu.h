/**
 * @file poll_imu.h
 * @brief Interface for polling VectorNav VN200 IMU data over UART.
 */

#ifndef POLL_IMU_H
#define POLL_IMU_H

#include <stdint.h>

/**
 * @brief Parsed INS data from the VectorNav VN200.
 */
typedef struct {
  double time;
  float yaw, pitch, roll;
  double latitude, longitude, altitude;
} IMUData;

/**
 * @brief Polls the VectorNav VN200 for the latest INS data.
 *
 * Opens a UART connection using the IMU pins defined in pindef.h,
 * sends a register read command, and parses the response.
 *
 * @param data_type  Register query string (e.g. "VNINS", "VNRRG").
 *                   Pass NULL to use the default ("VNINS").
 * @param out        Pointer to IMUData struct to populate with results.
 * @return 0 on success, -1 on failure.
 */
int poll_imu(const char *data_type, IMUData *out);

#endif /* POLL_IMU_H */

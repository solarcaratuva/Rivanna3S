/**
 * @file poll_imu.cpp
 * @brief Polls VectorNav VN200 IMU for INS data over UART.
 */

#include "poll_imu.h"
#include "UART.h"
#include "pindef.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* ---------- config ---------- */

#define IMU_BAUD 115200
#define IMU_TX_PIN USB_TX // pins for the vn200 IMU
#define IMU_RX_PIN USB_RX

#define READ_BUF_SIZE 256
#define READ_TIMEOUT 500

/* ---------- private helpers ---------- */

/**
 * @brief Builds the VN register-read command string.
 * For "VNINS" the VN200 register is 58, so the command is "$VNRRG,58*XX\r\n".
 */
static int build_command(const char *data_type, char *cmd, size_t cmd_len) {
  if (std::strcmp(data_type, "VNINS") == 0) {
    std::snprintf(cmd, cmd_len, "$VNRRG,58*XX\r\n");
    return 0;
  }
  return -1;
}

/**
 * @brief Parses a $VNINS response line into an IMUData struct.
 *
 * Expected format: $VNINS,<time>,<week>,<status>,<yaw>,<pitch>,<roll>,
 *                  <lat>,<lon>,<alt>,<vel_x>,<vel_y>,<vel_z>,<att_uncertainty>,
 *                  <pos_uncertainty>,<vel_uncertainty>*XX
 */
static int parse_vnins(const char *line, IMUData *out) {
  const char *start = std::strstr(line, "$VNINS");
  if (!start)
    return -1;

  /* skip past "$VNINS," */
  start += 7;

  /* work on a mutable copy (strip checksum) */
  char buf[READ_BUF_SIZE];
  std::strncpy(buf, start, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  char *asterisk = std::strchr(buf, '*');
  if (asterisk)
    *asterisk = '\0';

  /* tokenize */
  char *tokens[16];
  int count = 0;
  char *tok = std::strtok(buf, ",");
  while (tok && count < 16) {
    tokens[count++] = tok;
    tok = std::strtok(NULL, ",");
  }

  if (count < 10)
    return -1;

  out->time = std::strtod(tokens[0], NULL);
  out->yaw = std::strtof(tokens[3], NULL);
  out->pitch = std::strtof(tokens[4], NULL);
  out->roll = std::strtof(tokens[5], NULL);
  out->latitude = std::strtod(tokens[6], NULL);
  out->longitude = std::strtod(tokens[7], NULL);
  out->altitude = std::strtod(tokens[8], NULL);

  return 0;
}

/* ---------- public API ---------- */

int poll_imu(const char *data_type, IMUData *out) {
  if (!out)
    return -1;

  const char *dtype = data_type ? data_type : "VNINS";

  char cmd[32];
  if (build_command(dtype, cmd, sizeof(cmd)) != 0)
    return -1;

  UART imu_uart(IMU_TX_PIN, IMU_RX_PIN, IMU_BAUD); // opens to uart
  if (!imu_uart.initialized)
    return -1;

  imu_uart.write(reinterpret_cast<uint8_t *>(cmd),
                 static_cast<uint16_t>(std::strlen(cmd)));

  uint8_t buf[READ_BUF_SIZE];
  std::memset(buf, 0, sizeof(buf));
  imu_uart.read(buf, sizeof(buf) - 1, READ_TIMEOUT);

  if (std::strcmp(dtype, "VNINS") == 0)
    return parse_vnins(reinterpret_cast<const char *>(buf), out);

  return -1;
}

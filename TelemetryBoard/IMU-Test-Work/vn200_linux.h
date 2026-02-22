#ifndef VN200_LINUX_H
#define VN200_LINUX_H

#include <string>
#include <vector>

struct VNData {
  double time;
  float yaw, pitch, roll;
  double latitude, longitude, altitude;
};

class VN200 {
public:
  VN200();
  ~VN200();

  bool connect(const std::string &port, int baudrate);
  void disconnect();
  std::string readRawLine();
  bool parseVNINS(const std::string &line, VNData &data);
  bool pollOnce(VNData &data, int timeout_ms = 2000);

private:
  int fd;
  std::string remainder; // Buffer to handle fragmented serial data
};

#endif
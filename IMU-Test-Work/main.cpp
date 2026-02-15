#include "vn200_linux.h"
#include <iomanip>
#include <iostream>
#include <unistd.h>

int main() {
  VN200 sensor;
  VNData data;

  if (!sensor.connect("/dev/ttyUSB0", 115200)) {
    std::cerr << "ERROR: Could not open /dev/ttyUSB0. Try: sudo chmod 666 "
                 "/dev/ttyUSB0"
              << std::endl;
    return 1;
  }

  std::cout << "Connected to VN-200. Waiting for data..." << std::endl;

  while (true) {
    std::string line = sensor.readRawLine();

    if (!line.empty()) {
      if (sensor.parseVNINS(line, data)) {
        // Successful Parse
        std::cout << "\033[2J\033[H"; // Clear Screen
        std::cout << "--- VECTORNAV VN-200 (INS MODE) ---" << std::endl;
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "YAW:   " << data.yaw << " deg" << std::endl;
        std::cout << "PITCH: " << data.pitch << " deg" << std::endl;
        std::cout << "ROLL:  " << data.roll << " deg" << std::endl;
        std::cout << std::setprecision(6);
        std::cout << "LAT:   " << data.latitude << std::endl;
        std::cout << "LON:   " << data.longitude << std::endl;
        std::cout << "ALT:   " << data.altitude << " m" << std::endl;
      } else if (line.find("$VN") != std::string::npos) {
        // Fallback: If it's a VN message but not INS, print raw so you know
        // it's working
        std::cout << "RAW DATA: " << line << std::endl;
      }
    }
    usleep(1000); // 1ms
  }
  return 0;
}
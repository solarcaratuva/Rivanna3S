#include <iostream>
#include <thread>
#include <chrono>
#include "vn200_linux.h"

int main() {
    VN200 sensor;

    std::cout << "Attempting to connect to VN-200 on /dev/ttyUSB0..." << std::endl;

    if (sensor.connect("/dev/ttyUSB0", 115200)) {
        std::cout << "Connected!" << std::endl;

        // 1. Read the Model Number (Register 1)
        std::cout << "Model Info: " << sensor.readRegister(1) << std::endl;

        // 2. Loop to read streaming IMU data (YPR, Accel, etc.)
        std::cout << "Streaming data for 5 seconds..." << std::endl;
        for(int i = 0; i < 50; ++i) {
            std::cout << "Data: " << sensor.readRawLine() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        sensor.disconnect();
    } else {
        std::cerr << "Failed to connect. Check permissions (sudo chmod 666 /dev/ttyUSB0)" << std::endl;
    }

    return 0;
}
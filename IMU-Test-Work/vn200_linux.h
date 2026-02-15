#ifndef VN200_LINUX_H
#define VN200_LINUX_H

#include <string>

class VN200 {
public:
    VN200();
    ~VN200();

    bool connect(const std::string& port, int baudrate);
    void disconnect();
    std::string readRegister(int regId);
    std::string readRawLine();

private:
    int fd; // File descriptor for the serial port
};

#endif
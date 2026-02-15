#include "vn200_linux.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

VN200::VN200() : fd(-1) {}

VN200::~VN200() { disconnect(); }

bool VN200::connect(const std::string& port, int baudrate) {
    fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) return false;

    struct termios tty;
    tcgetattr(fd, &tty);

    // Set Baud Rate (Assuming 115200)
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cflag |= (CLOCAL | CREAD);    // Ignore modem controls, enable reading
    tty.c_cflag &= ~CSIZE;              // Clear size bits
    tty.c_cflag |= CS8;                 // 8-bit characters
    tty.c_cflag &= ~PARENB;             // No parity
    tty.c_cflag &= ~CSTOPB;             // 1 stop bit

    tcsetattr(fd, TCSANOW, &tty);
    return true;
}

void VN200::disconnect() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

// Sends a command to read a specific register
std::string VN200::readRegister(int regId) {
    if (fd == -1) return "Not Connected";

    // Format: $VNRRG,RegID\r\n
    std::string cmd = "$VNRRG," + std::to_string(regId) + "\r\n";
    write(fd, cmd.c_str(), cmd.length());

    return readRawLine();
}

// Reads one line from the sensor
std::string VN200::readRawLine() {
    char buf[256];
    memset(buf, 0, sizeof(buf));
    
    // Simple read (in a real app, you'd buffer this until \n)
    usleep(100000); // Wait 100ms for response
    int bytes = read(fd, buf, sizeof(buf) - 1);
    
    return (bytes > 0) ? std::string(buf) : "No Data";
}
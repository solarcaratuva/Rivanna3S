#include "vn200_linux.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <termios.h>
#include <unistd.h>

VN200::VN200() : fd(-1), remainder("") {}

VN200::~VN200() { disconnect(); }

bool VN200::connect(const std::string &port, int baudrate) {
  fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd == -1)
    return false;

  struct termios tty;
  if (tcgetattr(fd, &tty) != 0)
    return false;

  // Set Baud Rate (Default 115200)
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  tty.c_cflag |= (CLOCAL | CREAD | CS8);
  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw mode
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_oflag &= ~OPOST;

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &tty);
  return true;
}

void VN200::disconnect() {
  if (fd != -1) {
    close(fd);
    fd = -1;
  }
}

std::string VN200::readRawLine() {
  char buf[1024];
  int n = read(fd, buf, sizeof(buf) - 1);

  if (n > 0) {
    buf[n] = '\0';
    remainder += std::string(buf);
  }

  size_t newline_pos = remainder.find("\n");
  if (newline_pos != std::string::npos) {
    std::string line = remainder.substr(0, newline_pos);
    remainder = remainder.substr(newline_pos + 1);
    return line;
  }
  return "";
}

bool VN200::parseVNINS(const std::string &line, VNData &data) {
  size_t start = line.find("$VNINS");
  if (start == std::string::npos)
    return false;

  std::string clean = line.substr(start + 7);
  size_t asterisk = clean.find('*');
  if (asterisk != std::string::npos)
    clean = clean.substr(0, asterisk);

  std::vector<std::string> tokens;
  std::string token;
  std::stringstream ss(clean);
  while (std::getline(ss, token, ',')) {
    tokens.push_back(token);
  }

  if (tokens.size() < 10)
    return false;

  try {
    data.time = std::stod(tokens[0]);
    data.yaw = std::stof(tokens[3]);
    data.pitch = std::stof(tokens[4]);
    data.roll = std::stof(tokens[5]);
    data.latitude = std::stod(tokens[6]);
    data.longitude = std::stod(tokens[7]);
    data.altitude = std::stod(tokens[8]);
    return true;
  } catch (...) {
    return false;
  }
}

bool VN200::pollOnce(VNData &data, int timeout_ms) {
  int elapsed = 0;
  const int interval = 1000; // 1 ms in microseconds

  while (elapsed < timeout_ms * 1000) {
    std::string line = readRawLine();
    if (!line.empty() && parseVNINS(line, data))
      return true;
    usleep(interval);
    elapsed += interval;
  }
  return false;
}
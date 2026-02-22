#ifndef MOTOR_INTERFACE_H
#define MOTOR_INTERFACE_H

#include "I2C.h"

#define THROTTLE_ADR    (0b0101111 << 1)
#define REGEN_ADR       (0b0101110 << 1)

class MotorInterface {

  public:
    MotorInterface(I2C *serial_bus);
    void sendThrottle(uint16_t throttle);
    void sendRegen(uint16_t regen);

  private:
    I2C *bus;
};

#endif
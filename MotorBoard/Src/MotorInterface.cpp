#include "MotorInterface.h"
#include "pindef.h"

MotorInterface::MotorInterface(I2C *serial_bus)
    : bus(serial_bus) {}

void MotorInterface::sendThrottle(uint16_t throttle) {
    uint16_t updated_throttle = 0x100 - throttle;
    uint8_t cmd[2];
    cmd[0] = (updated_throttle & 0x100) >> 8;
    cmd[1] = updated_throttle & 0xFF;
    bus->write(THROTTLE_ADR, cmd, 2);
}

void MotorInterface::sendRegen(uint16_t regen) {
    uint16_t updated_regen = 0x100 - regen;
    uint8_t cmd[2];
    cmd[0] = (updated_regen & 0x100) >> 8;
    cmd[1] = updated_regen & 0xFF;
    bus->write(REGEN_ADR, cmd, 2);
}

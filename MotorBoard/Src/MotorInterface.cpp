#include "MotorInterface.h"
#include "pindef.h"

/**
 * Motor driver that controls the throttle and regen values
 * @param throttle Mbed I2C object for throttle communication
 * @param regen Mbed I2C object for regen communication
 */
MotorInterface::MotorInterface(I2C *serial_bus)
    : bus(serial_bus) {}

/**
 * Sends throttle value to digital POT
 * @param throttle Throttle value [0, 256]
 */
void MotorInterface::sendThrottle(uint16_t throttle) {
    uint16_t updated_throttle = 0x100 - throttle;
    uint8_t cmd[2];
    cmd[0] = (updated_throttle & 0x100) >> 8;
    cmd[1] = updated_throttle & 0xFF;
    bus->write(THROTTLE_ADR, cmd, 2);
}

/**
 * Sends regen value to digital POT
 * @param regen Regen value [0, 256]
 */
void MotorInterface::sendRegen(uint16_t regen) {
    uint16_t updated_regen = 0x100 - regen;
    uint8_t cmd[2];
    cmd[0] = (updated_regen & 0x100) >> 8;
    cmd[1] = updated_regen & 0xFF;
    bus->write(REGEN_ADR, cmd, 2);
}

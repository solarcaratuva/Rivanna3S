#include "AnalogIn.h"
#include "ScaledAnalogIn.h"


ScaledAnalogIn::ScaledAnalogIn(Pin pin, float voltageMin, float voltageMax, float valueMin, float valueMax)
    : AnalogIn(pin), voltageMin_(voltageMin), voltageMax_(voltageMax), valueMin_(valueMin), valueMax_(valueMax) {
}

float ScaledAnalogIn::read() {
    float voltage = AnalogIn::read_voltage();
    if (voltage < voltageMin_) voltage = voltageMin_;
    if (voltage > voltageMax_) voltage = voltageMax_;
    return valueMin_ + (voltage - voltageMin_) * (valueMax_ - valueMin_) / (voltageMax_ - voltageMin_);
}

uint8_t ScaledAnalogIn::read_hex_percent() {
    float voltage = read_voltage();
    float val = (voltage - voltageMin_) * 255.0f / (voltageMax_ - voltageMin_);
    if (val < 0.0f) val = 0.0f;
    if (val > 255.0f) val = 255.0f;
    return static_cast<uint8_t>(val);
}
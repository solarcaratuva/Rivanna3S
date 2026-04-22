#ifndef ScaledAnalogIn_h
#define ScaledAnalogIn_h

#include "AnalogIn.h"


class ScaledAnalogIn : public AnalogIn {
public:
    /**
     * @brief Construct an analog input that linearly maps a voltage window to a value window.
     * @param pin The analog-capable pin to sample.
     * @param voltageMin Lower bound of the input voltage window (in volts).
     * @param voltageMax Upper bound of the input voltage window (in volts).
     * @param valueMin Output value corresponding to @p voltageMin.
     * @param valueMax Output value corresponding to @p voltageMax.
     * @note This constructor does not validate bounds. Callers should ensure @p voltageMax != @p voltageMin.
     */
    explicit ScaledAnalogIn(Pin pin, float voltageMin, float voltageMax, float valueMin, float valueMax);

    /**
     * @brief Read the input voltage and map it to the configured output range.
     * @return Linearly scaled value in the [valueMin, valueMax] range after clamping voltage to
     *         [voltageMin, voltageMax].
     * @note Behavior is undefined if voltageMin equals voltageMax due to division by zero.
     */
    float read();

    /**
     * @brief Read the input as an 8-bit scaled value over the configured voltage window.
     * @return Unsigned 8-bit value computed from the clamped voltage.
     * @note Behavior is undefined if voltageMin equals voltageMax due to division by zero.
     */
    uint8_t read_hex_percent();
    
private:
    float voltageMin_;
    float voltageMax_;
    float valueMin_;
    float valueMax_;
};



#endif
#ifndef AnalogIn_h
#define AnalogIn_h

#include <stdint.h>
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"


class AnalogIn {
public:
    bool initialized = false;
    
    /**
     * @brief Construct an AnalogIn object for reading analog values from a pin
     * @param pin The pin to configure for analog input
     */
    explicit AnalogIn(Pin pin);
    
    /**
     * @brief Read the analog input value as a 12-bit unsigned integer
     * @return Raw ADC value (0-4095)
     */
    uint16_t read_u12();
    
    /**
     * @brief Read the analog input value as a normalized float
     * @return Normalized value between 0.0 and 1.0
     */
    float read();
    
    /**
     * @brief Read the analog input value as a voltage
     * @return Voltage value from 0.0 to 3.3V
     */
    float read_voltage();
private:
    ADC_Peripheral* adc_periph;
    uint32_t adc_get_rank(const ADC_Peripheral* peripheral);
    ADC_Peripheral* findADCPin(Pin pin);
    ADC_HandleTypeDef* hadc;
};

#endif

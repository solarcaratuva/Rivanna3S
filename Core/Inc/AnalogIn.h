#ifndef AnalogIn_h
#define AnalogIn_h

#include <stdint.h>
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"


class AnalogIn {
public:
    bool initialized = false;
    explicit AnalogIn(Pin pin);
    uint16_t read_u12();
    float read();
    float read_voltage();
private:
    ADC_Peripheral* adc_periph;
    uint32_t adc_get_rank(const ADC_Peripheral* peripheral);
    ADC_Peripheral* findADCPin(Pin pin);
    ADC_HandleTypeDef* hadc;
};

#endif

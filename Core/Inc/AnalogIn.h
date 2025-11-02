#ifndef AnalogIn_h
#define AnalogIn_h
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"
class AnalogIn {
public:
    bool initialized = false;
    explicit AnalogIn(Pin pin);
    float read();
private:
    ADC_Peripheral* adc_periph;
    void initGPIO(ADC_Peripheral* adc_peripheral);
    void initADC();
    ADC_Peripheral* findADCPin(Pin pin);
    ADC_HandleTypeDef hadc1;
};
#endif

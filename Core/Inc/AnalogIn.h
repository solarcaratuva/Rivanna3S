#ifndef AnalogIn_h
#define AnalogIn_h
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"
extern ADC_HandleTypeDef hadc1;
extern ADC_Peripheral* adc_periph;
class AnalogIn {
public:
    bool initialized = false;
    explicit AnalogIn(Pin pin, uint32_t baud = 0);
    void read(uint16_t* value);
private:
    void initGPIO(ADC_Peripheral* adc_peripheral);
    void initADC();
    ADC_Peripheral* findADCPin(Pin pin);
};
#endif
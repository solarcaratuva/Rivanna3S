
#include "AnalogIn.h"
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "adc.h"

AnalogIn::AnalogIn(Pin pin) {
    adc_periph = findADCPin(pin);
    if(adc_periph == nullptr) {
        initialized = false;
        return;
    }
    adc_periph->used_pin = pin;
    gpio_clock_enable(pin.block);
    HAL_ADC_MspInit2(adc_periph->instance, pin);
    hadc = ADC_init(adc_periph->instance, adc_periph->channel);
    initialized = true;
}


float AnalogIn::read() {
	/*
    if (!initialized || adc_periph == nullptr) {
        return ; // Not initialized properly
    }
    */

    // Start ADC conversion
    HAL_ADC_Start(hadc);

    float value;
    // Poll for conversion completion
    HAL_StatusTypeDef check = HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
    if (check == HAL_OK) {
        // Get the converted value
        value = (HAL_ADC_GetValue(hadc)/ (float)65536 )*(float)3.3;
    }

    // Stop ADC
    HAL_ADC_Stop(hadc);
    return value;
}


//should work
ADC_Peripheral* AnalogIn::findADCPin(Pin pin) {
    for (uint8_t i = 0; i < ADC_PERIPHERAL_COUNT; i++) {
        ADC_Peripheral* peripheral = &ADC_Peripherals[i];
        if ((peripheral->pin_mask & pin.universal_mask) != 0) {
            if (!peripheral->isClaimed) {
                peripheral->isClaimed = true;
                return peripheral;
            }
        }
    }
    return nullptr; // No matching ADC peripheral found
}







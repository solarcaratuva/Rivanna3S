#include "AnalogIn.h"
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

// function declarations for init functions in a hardware-specific adc.c file
extern "C" ADC_HandleTypeDef* ADC_init(ADC_TypeDef* hadc, uint32_t channel, uint32_t rank);
extern "C" void HAL_ADC_MspInit_custom(ADC_TypeDef* adcHandle, Pin pin);


AnalogIn::AnalogIn(Pin pin) {
    adc_periph = findADCPin(pin);
    if(adc_periph == nullptr) {
        initialized = false;
        return;
    }
    adc_periph->used_pin = pin;
    gpio_clock_enable(pin.block);
    HAL_ADC_MspInit_custom(adc_periph->instance, pin);
    uint32_t rank = adc_get_rank(adc_periph);
    hadc = ADC_init(adc_periph->instance, adc_periph->channel, rank);
    initialized = true;
}


float AnalogIn::read() {

    // Start ADC conversion
    HAL_ADC_Start(hadc);

    float value;
    // Poll for conversion completion
    HAL_StatusTypeDef check = HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
    if (check == HAL_OK) {
        // Get the converted value
        value = (HAL_ADC_GetValue(hadc)/(float)4095 );
    }
    // Stop ADC
    HAL_ADC_Stop(hadc);
    return value;
}

float AnalogIn::read_voltage(){
	return read()*(float)3.3;
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







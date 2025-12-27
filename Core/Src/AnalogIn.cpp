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
    if (!initialized) {
        return 10.0f;
    }

    // Start ADC conversion
    HAL_StatusTypeDef check = HAL_ADC_Start(hadc);
    if (check != HAL_OK) return 11.0f;

    // Poll for conversion completion
    check = HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
    if (check != HAL_OK) return 12.0f;

    // Get the converted value
    float value = (HAL_ADC_GetValue(hadc)/4095.0f);

    // Stop ADC
    check = HAL_ADC_Stop(hadc);
    if (check != HAL_OK) return 13.0f;

    return value;
}

float AnalogIn::read_voltage(){
	return read() * 3.3f;
}


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

uint32_t AnalogIn::adc_get_rank(const ADC_Peripheral* peripheral) {
	uint8_t index = peripheral->instance_num;

	uint32_t rank;
	switch (adc_channels_claimed[index]) {
		case 0: rank = ADC_REGULAR_RANK_1; break;
		case 1: rank = ADC_REGULAR_RANK_2; break;
		case 2: rank = ADC_REGULAR_RANK_3; break;
		case 3: rank = ADC_REGULAR_RANK_4; break;
		case 4: rank = ADC_REGULAR_RANK_5; break;
		case 5: rank = ADC_REGULAR_RANK_6; break;
		case 6: rank = ADC_REGULAR_RANK_7; break;
		case 7: rank = ADC_REGULAR_RANK_8; break;
		case 8: rank = ADC_REGULAR_RANK_9; break;
		case 9: rank = ADC_REGULAR_RANK_10; break;
		default: rank = ADC_REGULAR_RANK_1; break;
	}

	adc_channels_claimed[index] += 1;
	return rank;
}


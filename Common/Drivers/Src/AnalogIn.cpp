#include "AnalogIn.h"
#include "stm32_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "log.h"
#include "lock.h"

// function declarations for init functions in a hardware-specific adc.c file
extern "C" ADC_HandleTypeDef* ADC_init(ADC_TypeDef* hadc, uint32_t channel, bool first_use);
extern "C" void HAL_ADC_MspInit_custom(ADC_TypeDef* adcHandle, Pin pin);

Lock AnalogIn::lock_;


AnalogIn::AnalogIn(Pin pin) {
    lock_.lock(); 
    adc_periph_ = findADCPin(pin);
    if(adc_periph_ == nullptr) {
        initialized = false;
        log_warn("AnalogIn init failed: no matching ADC peripheral for pin");
        lock_.unlock();
        return;
    }
    adc_periph_->used_pin = pin;

    gpio_clock_enable(pin.block);

    HAL_ADC_MspInit_custom(adc_periph_->instance, pin);

    bool first_use = use_adc(adc_periph_->instance);
    hadc_ = ADC_init(adc_periph_->instance, adc_periph_->channel, first_use);
    
    initialized = true;
    lock_.unlock();
}

uint16_t AnalogIn::read_u12() {
    if (!initialized) {
        log_warn("AnalogIn read failed: AnalogIn not initialized");
        return 10.0f;
    }
    lock_.lock();
    ADC_init(adc_periph_->instance, adc_periph_->channel, false);

    // Start ADC conversion
    HAL_StatusTypeDef check = HAL_ADC_Start(hadc_);
    if (check != HAL_OK) {
        log_warn("AnalogIn read failed: HAL_ADC_Start status %d", check);
        lock_.unlock();
        return 11.0f;
    }

    // Poll for conversion completion
    check = HAL_ADC_PollForConversion(hadc_, HAL_MAX_DELAY);
    if (check != HAL_OK) {
        log_warn("AnalogIn read failed: HAL_ADC_PollForConversion status %d", check);
        lock_.unlock();
        return 12.0f;
    }

    // Get the converted value
    uint16_t value = (HAL_ADC_GetValue(hadc_));

    // Stop ADC
    check = HAL_ADC_Stop(hadc_);
    if (check != HAL_OK) {
        log_warn("AnalogIn read failed: HAL_ADC_Stop status %d", check);
        lock_.unlock();
        return 13.0f;
    }

    lock_.unlock();
    return value;
}


float AnalogIn::read() {
    uint16_t value = read_u12();
    return static_cast<float>(value) / 4095.0f;
}

float AnalogIn::read_voltage(){
	return read() * 3.3f;
}

uint8_t AnalogIn::read_hex_percent() {
    float voltage = read_voltage();
    float percent = (voltage / 3.3f) * 255.0f;
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 255.0f) percent = 255.0f;
    return static_cast<uint8_t>(percent);
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


#include "AnalogIn.h"
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_Peripheral* adc_periph;


AnalogIn::AnalogIn(Pin pin, uint32_t baud) {
    adc_periph = findADCPin(pin);
    if(adc_periph != nullptr) {
        adc_periph->used_pin = pin;
        initGPIO(adc_periph);
        initADC();
        initialized = true;
    }
}

// Initialize GPIO for ADC pin
void AnalogIn::initGPIO(ADC_Peripheral* adc_periph) { 
    Pin adc_pin = adc_periph->used_pin;

    //Turn on clocks for every port
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE(); 
    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // ADC Pin
    GPIO_InitStruct.Pin       = adc_pin.block_mask;
    GPIO_InitStruct.Mode      = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    HAL_GPIO_Init(adc_pin.block, &GPIO_InitStruct);
}



//should work
ADC_Peripheral* AnalogIn::findADCPin(Pin pin) {
    for (uint8_t i = 0; i < ADC_PERIPHERAL_COUNT; i++) {
        ADC_Peripheral* peripheral = &ADC_Peripherals[i];
        if (peripherial->pin_mask & pin.universal_mask) {
            if (!peripheral->isClaimed) {
                (*peripheral).isClaimed = true;
                return peripheral;
            }
        }
    }
    return nullptr; // No matching ADC peripheral found
}


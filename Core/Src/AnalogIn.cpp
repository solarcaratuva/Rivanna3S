
#include "AnalogIn.h"
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"

ADC_Peripheral* adc_periph;


AnalogIn::AnalogIn(Pin pin) {
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

float AnalogIn::read() {
	/*
    if (!initialized || adc_periph == nullptr) {
        return ; // Not initialized properly
    }
    */

    // Start ADC conversion
    HAL_ADC_Start(&hadc1);

    float value;
    // Poll for conversion completion
    if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
        // Get the converted value
        value = (float) HAL_ADC_GetValue(&hadc1);
    }

    // Stop ADC
    HAL_ADC_Stop(&hadc1);
    return value;
}


//should work
ADC_Peripheral* AnalogIn::findADCPin(Pin pin) {
    for (uint8_t i = 0; i < ADC_PERIPHERAL_COUNT; i++) {
        ADC_Peripheral* peripheral = &ADC_Peripherals[i];
        // use !=0 for any-bit overlap, or == pin.universal_mask for full match
        if ((peripheral->pin_mask & pin.universal_mask) != 0) {
            if (!peripheral->isClaimed) {
                peripheral->isClaimed = true;
                return peripheral;
            }
        }
    }
    return nullptr; // No matching ADC peripheral found
}

// Configure ADC channel for this AnalogIn instance using the global HAL handle
void AnalogIn::initADC() {
    if (adc_periph == nullptr) {
        return;
    }

    hadc1.Instance = adc_periph->instance;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc1.Init.OversamplingMode = DISABLE;
    hadc1.Init.Oversampling.Ratio = 1;

    HAL_ADC_Init(&hadc1);


    ADC_ChannelConfTypeDef sConfig = {0};
     sConfig.Channel = adc_periph->channel;
     sConfig.Rank = ADC_REGULAR_RANK_1;
   //  sConfig.SamplingTime = adc_periph->sampling_time;
     sConfig.SingleDiff = ADC_DIFFERENTIAL_ENDED;
     sConfig.OffsetNumber = ADC_OFFSET_NONE;
     sConfig.Offset = 0;
     sConfig.OffsetSignedSaturation = DISABLE;

     HAL_ADC_ConfigChannel(&hadc1, &sConfig);


}







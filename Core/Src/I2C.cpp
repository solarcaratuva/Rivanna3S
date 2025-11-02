#include "pinmap.h"
#include "peripheralmap.h"

//Constructor
I2C::I2C(Pin sda, Pin scl, uint32_t baudrate)
	: sda(sda), scl(scl), baudrate(baudrate){
	i2c_periph = find_i2c_pins(sda, scl);
	if(i2c_periph != nullptr) {
			i2c_periph->sda_used = sda;
			i2c_periph->scl_used = scl;
	        init_gpio(i2c_periph);
	        init_i2c(baud);
	        initialized = true;
	    }
}

void I2C::init_gpio(I2C_Peripheral* i2c_periph) {
    Pin sda_pin = i2c_periph->sda_used;
    Pin scl_pin = i2c_periph->scl_used;

    //Turn on clocks for each gpio port
    gpio_clock_enable(sda_pin.block);
    gpio_clock_enable(scl_pin.block);


    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // TX
    GPIO_InitStruct.Pin       = sda_pin.block_mask;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = i2c_periph->alternate_function;
    HAL_GPIO_Init(sda_pin.block, &GPIO_InitStruct);

    // RX
    GPIO_InitStruct.Pin       = scl_pin.block_mask;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Alternate =  i2c_periph->alternate_function;
    HAL_GPIO_Init(scl_used.block, &GPIO_InitStruct);
}



I2C_Peripheral* I2C::find_i2c_pins(Pin sda, Pin scl) {
	for (size_t i = 0; i < I2C_PERIPHERAL_COUNT; ++i) {
		I2C_Peripheral* peripheral = &I2C_Peripherals[i];
	        if (((*peripheral).sda_valid_pins & sda.universal_mask) &&
	            ((*peripheral).scl_valid_pins & scl.universal_mask)) {
	            if (!peripheral->isClaimed) {
	                (*peripheral).isClaimed = true;
	                return peripheral;
	            }
	        }
	    }
	    return nullptr; // No matching peripheral found
}

#include "pinmap.h"
#include "peripheralmap.h"
#include "I2C.h"

// function declarations for init functions in a hardware-specific i2c.c file
extern "C" I2C_HandleTypeDef* I2C_init(I2C_TypeDef* i2cHandle, uint32_t timing_reg);
extern "C" void HAL_I2C_MspInit_custom(I2C_TypeDef* i2cHandle, Pin sda, Pin scl, uint8_t af_sda, uint8_t af_scl);
extern "C" uint32_t compute_timing(uint32_t freq_hz);

//Constructor
I2C::I2C(Pin sda, Pin scl, I2C_BaudRate baudrate)
	: sda(sda), scl(scl), baudrate(baudrate){
	i2c_periph = find_i2c_pins(sda, scl);
	if(i2c_periph == nullptr) {
		initialized = false;
		return;
	}

	i2c_periph->sda_used = sda;
	i2c_periph->scl_used = scl;
	gpio_clock_enable(sda.block);
	gpio_clock_enable(scl.block);
	uint8_t sda_af = get_i2c_af(i2c_periph->handle, sda, SDA);
	uint8_t scl_af = get_i2c_af(i2c_periph->handle, scl, SCL);

	HAL_I2C_MspInit_custom(i2c_periph->handle, sda, scl, sda_af, scl_af);
	uint32_t timing_reg = compute_timing(baudrate);
    hi2c = I2C_init(i2c_periph->handle, timing_reg);
    initialized = true;
}


void I2C::write(uint16_t address, uint8_t *buffer, uint16_t length){
	if(initialized) {
	    HAL_I2C_Master_Transmit(hi2c, address, buffer, length, HAL_MAX_DELAY);
	}
}


void I2C::read(uint16_t address, uint8_t *buffer, uint16_t length){
	if(initialized) {
		HAL_I2C_Master_Receive(hi2c, address, buffer, length, HAL_MAX_DELAY);
	}
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



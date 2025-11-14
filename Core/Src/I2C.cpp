#include "pinmap.h"
#include "peripheralmap.h"
#include "I2C.h"

//Constructor
I2C::I2C(Pin sda, Pin scl, uint32_t baudrate)
	: sda(sda), scl(scl), baudrate(baudrate){
	i2c_periph = find_i2c_pins(sda, scl);
	if(i2c_periph != nullptr) {
			i2c_periph->sda_used = sda;
			i2c_periph->scl_used = scl;
	        init_gpio(i2c_periph);
	        init_i2c(baudrate);
	        initialized = true;
	    }
}


//TODO: FINISH READ & WRITE
void I2C::write(uint16_t address, uint8_t *buffer, uint16_t length){
	if(initialized != 0) {
	    HAL_I2C_Master_Transmit(&hi2c, address, buffer, length, HAL_MAX_DELAY);
	}
}


void I2C::read(uint16_t address, uint8_t *buffer, uint16_t length){
	if(initialized != 0) {
		HAL_I2C_Master_Receive(&hi2c, address, buffer, length, HAL_MAX_DELAY);
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
    HAL_GPIO_Init(scl_pin.block, &GPIO_InitStruct);
}


// Initialize UART peripheral
void I2C::init_i2c(uint32_t baudrate) {
    //Turn on clocks for all possible UART

	i2c_clock_enable(i2c_periph->handle);

	hi2c.Instance = i2c_periph->handle;
	hi2c.Init.Timing = compute_timing(baudrate);
	hi2c.Init.OwnAddress1 = 0;
	hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c.Init.OwnAddress2 = 0;
	hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c);

    /** Configure Analogue filter
      */
    HAL_I2CEx_ConfigAnalogFilter(&hi2c, I2C_ANALOGFILTER_ENABLE);

    /** Configure Digital filter
      */
    HAL_I2CEx_ConfigDigitalFilter(&hi2c, 0);


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

uint32_t I2C::compute_timing(uint32_t freq_hz)
{
    // I2C kernel clock = 64 MHz for STM32H743 (D2PCLK1)
    constexpr uint32_t I2C_CLK = 64000000;

    // ======== 100 kHz ========
    if (freq_hz <= 100000) {
        // Standard mode, Rise ≈ 100ns, Fall ≈ 10ns
        return 0x10805D89;
    }

    // ======== 400 kHz ========
    else if (freq_hz <= 400000) {
        // Fast mode, Rise ≈ 100ns, Fall ≈ 10ns
        return 0x00C0216C;
    }

    // ======== 1 MHz ========
    else if (freq_hz <= 1000000) {
        // Fast mode plus, Rise ≈ 60ns, Fall ≈ 10ns
        return 0x00707CBB;
    }

    // Unsupported speed — return safe default (100 kHz)
    return 0x10805D89;
}

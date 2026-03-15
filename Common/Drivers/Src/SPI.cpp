#include "SPI.h"
#include "stm32h7xx_hal.h"
#include "pinmap.h"
#include "peripheralmap.h"
#include "log.h"

extern "C" SPI_HandleTypeDef* SPI_init(SPI_TypeDef* spi_instance, uint32_t baudrate);
extern "C" void HAL_SPI_MspInit_custom(SPI_TypeDef* spi_instance, Pin pin, uint8_t af);


SPI::SPI(Pin mosi, Pin miso, Pin sck, uint32_t baudrate) {
    spi_periph = findSPIPins(mosi, miso, sck);
    if(spi_periph == nullptr) {
        initialized = false;
        log_warn("SPI init failed: no matching peripheral for MOSI/MISO/SCK pins");
        return;
    }
    spi_periph->mosi_used = mosi;
    spi_periph->miso_used = miso;
    spi_periph->sck_used = sck;

    gpio_clock_enable(mosi.block);
    gpio_clock_enable(miso.block);
    gpio_clock_enable(sck.block);

    uint8_t mosi_af = get_SPI_AF(spi_periph->instance, &mosi, MOSI);
    uint8_t miso_af = get_SPI_AF(spi_periph->instance, &miso, MISO);
    uint8_t sck_af = get_SPI_AF(spi_periph->instance, &sck, SCK);

    HAL_SPI_MspInit_custom(spi_periph->instance, mosi, mosi_af);
    HAL_SPI_MspInit_custom(spi_periph->instance, miso, miso_af);
    HAL_SPI_MspInit_custom(spi_periph->instance, sck, sck_af);

    hspi = SPI_init(spi_periph->instance, baudrate);
    initialized = true;
}

int SPI::write(uint8_t *tx_buffer, uint16_t length) {
    if(!initialized) {
        log_warn("SPI write failed: SPI not initialized");
        return 1;
    }

    HAL_StatusTypeDef status = HAL_SPI_Transmit(hspi, tx_buffer, length, HAL_MAX_DELAY);

    if (status != HAL_OK) {
        log_warn("SPI write failed: status %d", status);
        return 2;
    }

    return 0;
}

int SPI::read(uint8_t *rx_buffer, uint16_t length) {
    if(!initialized) {
        log_warn("SPI read failed: SPI not initialized");
        return 1;
    }

    HAL_StatusTypeDef status = HAL_SPI_Receive(hspi, rx_buffer, length, HAL_MAX_DELAY);

    if (status != HAL_OK) {
        log_warn("SPI read failed: status %d", status);
        return 2;
    }

    return 0;
}

/* 
THIS FUNCTION IS NOT PROPERLY TESTED

WARNING: DO NOT USE UNTIL PROPERLY TESTED

It is recommended to use write() and read() separately instead of this function until it is tested
*/
int SPI::transfer(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t length) {
    if(!initialized) {
        log_warn("SPI transfer failed: SPI not initialized");
        return 1;
    }

    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, tx_buffer, rx_buffer, length, HAL_MAX_DELAY);

    if(status != HAL_OK) {
        log_warn("SPI transfer failed: status %d", status);
        return 2;
    }

    return 0;
}

SPI_Peripheral* SPI::findSPIPins(Pin mosi, Pin miso, Pin sck) {
    for(size_t i = 0; i < SPI_PERIPHERAL_COUNT; ++i) {
        SPI_Peripheral* peripheral = &SPI_Peripherals[i];
        if (((*peripheral).mosi_valid_pins & mosi.universal_mask) &&
            ((*peripheral).miso_valid_pins & miso.universal_mask) &&
            ((*peripheral).sck_valid_pins & sck.universal_mask)) {
            if (!peripheral->isClaimed) {
                (*peripheral).isClaimed = true;
                return peripheral;
            }
        }
    }

    return nullptr;
}
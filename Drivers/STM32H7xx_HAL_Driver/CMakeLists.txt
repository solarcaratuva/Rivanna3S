add_library(STM32H7xx_HAL_DRIVER-lib
    Src/stm32h7xx_hal_adc.c
    Src/stm32h7xx_hal_flash_ex.c
    Src/stm32h7xx_hal_rcc_ex.c
    Src/stm32h7xx_hal_adc_ex.c
    Src/stm32h7xx_hal_gpio.c
    Src/stm32h7xx_hal_spi.c
    Src/stm32h7xx_hal.c
    Src/stm32h7xx_hal_hsem.c
    Src/stm32h7xx_hal_spi_ex.c
    Src/stm32h7xx_hal_cortex.c
    Src/stm32h7xx_hal_i2c.c
    Src/stm32h7xx_hal_tim.c
    Src/stm32h7xx_hal_dma.c
    Src/stm32h7xx_hal_i2c_ex.c
    Src/stm32h7xx_hal_tim_ex.c
    Src/stm32h7xx_hal_dma_ex.c
    Src/stm32h7xx_hal_mdma.c
    Src/stm32h7xx_hal_uart.c
    Src/stm32h7xx_hal_exti.c
    Src/stm32h7xx_hal_pwr.c
    Src/stm32h7xx_hal_uart_ex.c
    Src/stm32h7xx_hal_fdcan.c
    Src/stm32h7xx_hal_pwr_ex.c
    Src/stm32h7xx_hal_flash.c
    Src/stm32h7xx_hal_rcc.c
)


target_include_directories(STM32H7xx_HAL_DRIVER-lib PUBLIC include)
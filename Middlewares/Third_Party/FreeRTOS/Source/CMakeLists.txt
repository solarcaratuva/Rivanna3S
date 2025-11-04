# FreeRTOS Kernel CMake configuration
cmake_minimum_required(VERSION 3.15)

# Ensure toolchain and project settings are inherited
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# --- Library target ---
add_library(freertos_kernel STATIC
    croutine.c
    event_groups.c
    list.c
    queue.c
    stream_buffer.c
    tasks.c
    timers.c
    CMSIS_RTOS_V2/cmsis_os2.c
    portable/MemMang/heap_4.c
)

# --- MCU port selection ---
set(FREERTOS_PORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/portable/GCC/ARM_CM4F)
target_sources(freertos_kernel PRIVATE
    ${FREERTOS_PORT_DIR}/port.c
)

# --- Include directories ---
target_include_directories(freertos_kernel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include                    # FreeRTOS kernel headers
    ${FREERTOS_PORT_DIR}                                   # Port headers
    ${CMAKE_SOURCE_DIR}/Core/Inc                           # FreeRTOSConfig.h
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include              # CMSIS compiler/ARM headers
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Core/Include         # CMSIS core headers
    ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc   # HAL driver headers
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include  # Device-specific headers
)

# --- Common compile settings ---
target_compile_definitions(freertos_kernel PUBLIC
    USE_HAL_DRIVER
    STM32H743xx
)

# --- Compiler options ---
set(MCU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
set(OPT_FLAGS -O0 -g3 -ffunction-sections -fdata-sections -fno-common)
set(WARN_FLAGS -Wall -Wextra -Wundef -Wdouble-promotion -Wformat=2 -Wshadow)

target_compile_options(freertos_kernel PRIVATE
    ${MCU_FLAGS} ${OPT_FLAGS} ${WARN_FLAGS}
)

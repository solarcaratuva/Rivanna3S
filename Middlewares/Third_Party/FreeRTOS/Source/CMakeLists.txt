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
set(FREERTOS_PORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/portable/GCC/${FREERTOS_PORT})
if(NOT EXISTS "${FREERTOS_PORT_DIR}")
    set(FREERTOS_FALLBACK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/portable/GCC/ARM_CM4F)
    if(EXISTS "${FREERTOS_FALLBACK_DIR}")
        message(WARNING "FreeRTOS port '${FREERTOS_PORT}' not found; falling back to ARM_CM4F.")
        set(FREERTOS_PORT_DIR "${FREERTOS_FALLBACK_DIR}")
    else()
        message(FATAL_ERROR "FreeRTOS port '${FREERTOS_PORT}' not found and no ARM_CM4F fallback present.")
    endif()
endif()
target_sources(freertos_kernel PRIVATE
    ${FREERTOS_PORT_DIR}/port.c
)

# --- Include directories ---
target_include_directories(freertos_kernel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include                    # FreeRTOS kernel headers
    ${FREERTOS_PORT_DIR}                                   # Port headers
    ${CMAKE_SOURCE_DIR}/Common/Libs/Inc                    # FreeRTOSConfig.h
    ${BOARD_SETUP_INC}                                     # stm32*_hal_conf.h
    ${CMAKE_SOURCE_DIR}/Core/Inc                           # main.h for Error_Handler
    ${BOARD_CMSIS_INC}                                     # CMSIS compiler/ARM headers
    ${BOARD_CMSIS_CORE_INC}                                # CMSIS core headers (if present)
    ${BOARD_HAL_INC}                                       # HAL driver headers
    ${BOARD_CMSIS_DEVICE_INC}                              # Device-specific headers
)

# --- Common compile settings ---
target_compile_definitions(freertos_kernel PUBLIC
    USE_HAL_DRIVER
    ${BOARD_DEVICE_DEFINE}
)

# --- Compiler options ---
set(OPT_FLAGS -O0 -g3 -ffunction-sections -fdata-sections -fno-common)
set(WARN_FLAGS -Wall -Wextra -Wundef -Wdouble-promotion -Wformat=2 -Wshadow)

target_compile_options(freertos_kernel PRIVATE
    ${MCU_FLAGS} ${OPT_FLAGS} ${WARN_FLAGS}
)

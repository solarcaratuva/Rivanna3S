cmake_minimum_required(VERSION 3.21)
project(stm32h743_freertos C CXX ASM)

# -------- Toolchain & MCU flags --------
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/arm-gcc.cmake")

# Cortex-M7 + hard float
set(MCU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
set(OPT_FLAGS -O0 -g3 -ffunction-sections -fdata-sections -fno-common)
set(WARN_FLAGS -Wall -Wextra -Wformat=2 -Wshadow -Wno-missing-field-initializers)

# Linker script
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32H743ZITX_FLASH.ld" CACHE FILEPATH "LD script")

# -------- Board helper --------
include(${CMAKE_SOURCE_DIR}/cmake/board.cmake)

# -------- Add FreeRTOS as a subdirectory (builds freertos_kernel library) --------
add_subdirectory(Middlewares/Third_Party/FreeRTOS/Source)
add_subdirectory(Drivers)
add_subdirectory(Common)

# -------- Boards --------
add_subdirectory(BottomDistBoard)
add_subdirectory(MotorBoard)
add_subdirectory(RelayBoard)
add_subdirectory(TelemetryBoard)
add_subdirectory(TopDistBoard)

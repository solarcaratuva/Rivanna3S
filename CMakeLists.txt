cmake_minimum_required(VERSION 3.21)

# -------- Toolchain & MCU flags --------
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/arm-gcc.cmake" CACHE FILEPATH "Toolchain file")
endif()

project(stm32h743_freertos C CXX ASM)

set(BOARD "STM32H743ZITX" CACHE STRING "Target board name (Drivers/<board>)")

set(OPT_FLAGS -O0 -g3 -ffunction-sections -fdata-sections -fno-common)
set(WARN_FLAGS -Wall -Wextra -Wformat=2 -Wshadow -Wno-missing-field-initializers)

# -------- Board helper --------
include(${CMAKE_SOURCE_DIR}/cmake/board.cmake)

# -------- Add FreeRTOS as a subdirectory (builds freertos_kernel library) --------
add_subdirectory(Middlewares/Third_Party/FreeRTOS/Source)
if(EXISTS "${BOARD_ROOT}/CMakeLists.txt")
  add_subdirectory(${BOARD_ROOT})
endif()
add_subdirectory(Common)

# -------- Boards --------
add_subdirectory(BottomDistBoard)
add_subdirectory(MotorBoard)
add_subdirectory(RelayBoard)
add_subdirectory(TelemetryBoard)
add_subdirectory(TopDistBoard)

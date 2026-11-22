cmake_minimum_required(VERSION 3.21)

# Configure cross-compilation toolchain when one is not provided by the caller.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/arm-gcc.cmake" CACHE FILEPATH "Toolchain file")
endif()

# Root firmware project (C, C++, and startup assembly).
project(stm32h743_freertos C CXX ASM)

# Select target board; board.cmake maps this to include paths, defines, and flags.
set(BOARD "STM32H743ZITX" CACHE STRING "Target board name (Drivers/<board>)")
option(UART_UPLOAD_MOCK "Enable UART upload with a streaming mock consumer on MotorBoard and TopDistBoard" OFF)
option(CAN_FIRMWARE_UPDATE "Enable UART-to-CAN updates into the inactive flash bank" OFF)
option(NUCLEO_G474RE "Use NUCLEO-G474RE LED and ST-Link UART pins" OFF)
if(NUCLEO_G474RE)
  if(NOT BOARD STREQUAL "STM32G474RET6")
    message(FATAL_ERROR "NUCLEO_G474RE requires BOARD=STM32G474RET6")
  endif()
  add_compile_definitions(NUCLEO_G474RE)
endif()
if(CAN_FIRMWARE_UPDATE AND UART_UPLOAD_MOCK)
  message(FATAL_ERROR "Select CAN_FIRMWARE_UPDATE or UART_UPLOAD_MOCK, not both")
endif()
if(CAN_FIRMWARE_UPDATE)
  if(NOT BOARD STREQUAL "STM32G474RET6" AND NOT BOARD STREQUAL "STM32H743ZITX")
    message(FATAL_ERROR "CAN firmware updates support G474RE and H743ZI only")
  endif()
  # Use the vector table at the linked flash address, including after bank remapping.
  add_compile_definitions(CAN_FIRMWARE_UPDATE USER_VECT_TAB_ADDRESS)
endif()

# Shared compile option groups consumed by child targets.
set(OPT_FLAGS -O0 -g3 -ffunction-sections -fdata-sections -fno-common)
set(WARN_FLAGS -Wall -Wextra -Wformat=2 -Wshadow -Wno-missing-field-initializers)

# Load helper macros/functions like add_board() and board-specific variables.
include(${CMAKE_SOURCE_DIR}/cmake/board.cmake)

# Build core support libraries first so boards can link against them.
add_subdirectory(Middlewares/Third_Party/FreeRTOS/Source)
# Add per-board driver package if that board directory has its own CMake target setup.
if(EXISTS "${BOARD_ROOT}/CMakeLists.txt")
  add_subdirectory(${BOARD_ROOT})
endif()
add_subdirectory(Common)

# Register each board firmware target.
add_subdirectory(BottomDistBoard)
add_subdirectory(MotorBoard)
add_subdirectory(RelayBoard)
add_subdirectory(TelemetryBoard)
add_subdirectory(TopDistBoard)

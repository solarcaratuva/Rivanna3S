## Top-level CMake for Drivers/STM32G474RET6
# This file delegates building optional driver package subtargets.

cmake_minimum_required(VERSION 3.21)

# Option to control building the STM32G4 HAL driver
# (useful for selectively disabling heavy vendor components during debugging).
option(BUILD_STM32G4_HAL "Build STM32G4xx HAL driver" ON)

if(BUILD_STM32G4_HAL)
  # Expose STM32G4xx HAL as a subtarget for consumers in the root project.
  add_subdirectory(STM32G4xx_HAL_Driver)
endif()

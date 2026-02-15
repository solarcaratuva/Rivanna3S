## Top-level CMake for Drivers/ directory
# This file delegates building each driver package in the Drivers tree.

cmake_minimum_required(VERSION 3.21)

# Option to control building the STM32H7 HAL driver
# (useful for selectively disabling heavy vendor components during debugging).
option(BUILD_STM32H7_HAL "Build STM32H7xx HAL driver" ON)

if(BUILD_STM32H7_HAL)
  # Expose STM32H7xx HAL as a subtarget for consumers in the root project.
  add_subdirectory(STM32H7xx_HAL_Driver)
endif()

# Other driver subdirectories can be added here, for example:
# add_subdirectory(Other_Driver)

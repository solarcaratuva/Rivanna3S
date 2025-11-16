cmake_minimum_required(VERSION 3.20)
project(H7_HAL LANGUAGES C ASM)

# Tell CMake we’re cross-compiling with arm-none-eabi (via toolchain file ideally)
# toolchain: set CMAKE_C_COMPILER=arm-none-eabi-gcc, etc.

# ---- Common MCU flags shared by all targets ----
add_library(mcu_flags INTERFACE)

# CPU/ABI + sectioning
target_compile_options(mcu_flags INTERFACE
  -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
  -ffunction-sections -fdata-sections
  -Wall
)

# Link-time GC of unused sections + newlib-nano specs
target_link_options(mcu_flags INTERFACE
  -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
  -Wl,--gc-sections
  --specs=nano.specs
)

# C standard & GNU extensions (equivalent to -std=gnu11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

# Debug/Release behaviors (maps to -g3/-O0 and -O2 etc.)
set(CMAKE_C_FLAGS_DEBUG   "-g3 -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

# Common macros
target_compile_definitions(mcu_flags INTERFACE
  USE_HAL_DRIVER
  STM32H743xx
)

# ---- Include directories (adjust FreeRTOS portable path for M7!) ----
# Keep only non-HAL-common includes here; the HAL driver subdirectory will
# export its own public include directories so consumers that link the HAL
# target automatically get them.
target_include_directories(mcu_flags INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include

  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F)

# ---- HAL driver subdirectory ----
# Delegate HAL driver build to its own CMakeLists in the Drivers tree. That
# file will create the `STM32H7xx_HAL_DRIVER` target and export include dirs.
# Add the Drivers umbrella directory which will add individual driver
# subdirectories (e.g. STM32H7xx_HAL_Driver). This keeps the root file
# cleaner when multiple driver packages exist.
add_subdirectory(Drivers)

# Common shared drivers/modules library
add_subdirectory(Common)

# Build the PowerBoard module (creates `powerboard` library)
add_subdirectory(PowerBoard)

# If you also build your app here (example):
# enable_language(ASM)    # already implied by LANGUAGES C ASM
# set(startup "${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32h743zitx.s")
# add_executable(app.elf Core/Src/main.c ${startup})
# target_link_libraries(app.elf PRIVATE STM32H7xx_HAL_DRIVER)
# # Make sure the startup is treated as ASM with CPP:
# set_source_files_properties(${startup} PROPERTIES LANGUAGE ASM)
# target_compile_options(app.elf PRIVATE -x assembler-with-cpp)
# # You’ll also add your linker script via: target_link_options(app.elf PRIVATE -T path/to/STM32H743ZI_FLASH.ld)

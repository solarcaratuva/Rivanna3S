cmake_minimum_required(VERSION 3.21)
project(stm32h743_freertos C CXX ASM)

# -------- Toolchain & MCU flags --------
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/arm-gcc-toolchain.cmake")

# Cortex-M7 + hard float
set(MCU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
set(OPT_FLAGS -O0 -g3 -ffunction-sections -fdata-sections -fno-common)
set(WARN_FLAGS -Wall -Wextra -Wformat=2 -Wshadow -Wno-missing-field-initializers)

# Linker script
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32H743ZITX_FLASH.ld" CACHE FILEPATH "LD script")

# -------- Include directories --------
set(INC_CORE          ${CMAKE_SOURCE_DIR}/Core/Inc)
set(INC_BOARD         ${CMAKE_SOURCE_DIR}/Drivers/STM32H743ZITX_Setup/Inc)
set(INC_COMMON_LIBS   ${CMAKE_SOURCE_DIR}/Common/Libs/Inc)
set(INC_HAL           ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc)
set(INC_HAL_LEGACY    ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy)
set(INC_CMSIS_DEV     ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include)
set(INC_CMSIS_CORE    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include)
set(INC_RTOS_KERNEL   ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include)
set(INC_RTOS_CMSISV2  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2)

# Optional TouchGFX (only if present)
# set(INC_TGFX_APP      ${CMAKE_SOURCE_DIR}/TouchGFX/App)
# set(INC_TGFX_TGT_GEN  ${CMAKE_SOURCE_DIR}/TouchGFX/target/generated)
# set(INC_TGFX_TGT      ${CMAKE_SOURCE_DIR}/TouchGFX/target)

# -------- Startup + Source files --------
set(STARTUP_S ${CMAKE_SOURCE_DIR}/Drivers/STM32H743ZITX_Setup/Src/startup_stm32h743zitx.s)

file(GLOB APP_C_SOURCES
  ${CMAKE_SOURCE_DIR}/Core/Src/*.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H743ZITX_Setup/Src/*.c
)
file(GLOB APP_CPP_SOURCES
  ${CMAKE_SOURCE_DIR}/Core/Src/*.cpp
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H743ZITX_Setup/Src/*.cpp
)

file(GLOB HAL_SRCS
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal*.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_*.c
)

# -------- Executable target --------
add_executable(${PROJECT_NAME}.elf
  ${STARTUP_S}
  ${APP_C_SOURCES}
  ${APP_CPP_SOURCES}
  ${HAL_SRCS}
)

# -------- Includes --------
target_include_directories(${PROJECT_NAME}.elf PRIVATE
  ${INC_CORE}
  ${INC_BOARD}
  ${INC_COMMON_LIBS}
  ${INC_HAL}
  ${INC_HAL_LEGACY}
  ${INC_CMSIS_DEV}
  ${INC_CMSIS_CORE}
  ${INC_RTOS_KERNEL}
  ${INC_RTOS_CMSISV2}
  ${FREERTOS_PORT_DIR}
)

# -------- Defines --------
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
  DEBUG
  USE_HAL_DRIVER
  STM32H743xx
)

# -------- Compile options --------
target_compile_options(${PROJECT_NAME}.elf PRIVATE
  ${MCU_FLAGS} ${OPT_FLAGS} ${WARN_FLAGS}
  -fstack-usage
)

set_source_files_properties(${APP_CPP_SOURCES} PROPERTIES COMPILE_FLAGS "-std=gnu++14 -fno-exceptions -fno-rtti -fno-use-cxa-atexit")
set_source_files_properties(${APP_C_SOURCES} PROPERTIES COMPILE_FLAGS "-std=gnu11")

# -------- Link options --------
target_link_options(${PROJECT_NAME}.elf PRIVATE
  ${MCU_FLAGS}
  -T ${LINKER_SCRIPT}
  -Wl,-Map=SolarCar_FreeRtos.map
  -Wl,--gc-sections
  -static
  --specs=nano.specs
  -Wl,--start-group
  -lc -lm -lstdc++
  -lsupc++
  -Wl,--end-group
)

# -------- Add FreeRTOS as a subdirectory (builds freertos_kernel library) --------
add_subdirectory(Middlewares/Third_Party/FreeRTOS/Source)
add_subdirectory(Drivers)
add_subdirectory(Common)

# -------- Link targets --------
target_link_libraries(${PROJECT_NAME}.elf
  common
  freertos_kernel
  m c gcc
)

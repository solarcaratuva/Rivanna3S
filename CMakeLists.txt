cmake_minimum_required(VERSION 3.21)
project(stm32h743_freertos C CXX ASM)

# -------- Toolchain & MCU flags --------
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/arm-gcc-toolchain.cmake")

# H743: Cortex-M7 + hard float
set(MCU_FLAGS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")
# Match your CubeIDE log: -O0 -g3
set(OPT_FLAGS "-O0 -g3 -ffunction-sections -fdata-sections -fno-common")
set(WARN_FLAGS "-Wall -Wextra -Wundef -Wdouble-promotion -Wformat=2 -Wshadow")

# Your linker script lives in repo root
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32H743ZITX_FLASH.ld" CACHE FILEPATH "LD script")

# -------- Include directories (exactly like your log) --------
set(INC_CORE          ${CMAKE_SOURCE_DIR}/Core/Inc)
set(INC_HAL           ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc)
set(INC_HAL_LEGACY    ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy)
set(INC_CMSIS_DEV     ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include)
set(INC_CMSIS_CORE    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include)
set(INC_RTOS_KERNEL   ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include)
set(INC_RTOS_CMSISV2  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2)
# Optional TouchGFX (only if present)
set(INC_TGFX_APP      ${CMAKE_SOURCE_DIR}/TouchGFX/App)
set(INC_TGFX_TGT_GEN  ${CMAKE_SOURCE_DIR}/TouchGFX/target/generated)
set(INC_TGFX_TGT      ${CMAKE_SOURCE_DIR}/TouchGFX/target)

# -------- FreeRTOS in Middlewares (CM7 if available; else CM4F like your log) --------
set(FREERTOS_BASE      ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source)
set(FREERTOS_PORT_CM7  ${FREERTOS_BASE}/portable/GCC/ARM_CM7)
set(FREERTOS_PORT_CM4F ${FREERTOS_BASE}/portable/GCC/ARM_CM4F)
if(EXISTS "${FREERTOS_PORT_CM7}/port.c")
  set(FREERTOS_PORT_DIR "${FREERTOS_PORT_CM7}")
else()
  message(WARNING "ARM_CM7 port not found; using ARM_CM4F to mirror CubeIDE log.")
  set(FREERTOS_PORT_DIR "${FREERTOS_PORT_CM4F}")
endif()

# -------- Sources (mirror CubeIDE) --------
# Startup (from your log)
set(STARTUP_S ${CMAKE_SOURCE_DIR}/Core/Startup/startup_stm32h743zitx.s)

# App
file(GLOB APP_C_SOURCES   ${CMAKE_SOURCE_DIR}/Core/Src/*.c)
file(GLOB APP_CPP_SOURCES ${CMAKE_SOURCE_DIR}/Core/Src/*.cpp)

# HAL (compile everything you listed)
file(GLOB HAL_SRCS
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal*.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_*.c
)

# FreeRTOS kernel + CMSIS-RTOS v2 shim + heap_4 + GCC port
set(FREERTOS_KERNEL_SRCS
  ${FREERTOS_BASE}/croutine.c
  ${FREERTOS_BASE}/event_groups.c
  ${FREERTOS_BASE}/list.c
  ${FREERTOS_BASE}/queue.c
  ${FREERTOS_BASE}/stream_buffer.c
  ${FREERTOS_BASE}/tasks.c
  ${FREERTOS_BASE}/timers.c
  ${FREERTOS_BASE}/CMSIS_RTOS_V2/cmsis_os2.c
  ${FREERTOS_PORT_DIR}/port.c
  ${FREERTOS_BASE}/portable/MemMang/heap_4.c
)

# -------- Executable --------
add_executable(${PROJECT_NAME}.elf
  ${STARTUP_S}
  ${APP_C_SOURCES}
  ${APP_CPP_SOURCES}
  ${HAL_SRCS}
  ${FREERTOS_KERNEL_SRCS}
)

# -------- Includes --------
target_include_directories(${PROJECT_NAME}.elf PRIVATE
  ${INC_CORE}
  ${INC_HAL}
  ${INC_HAL_LEGACY}
  ${INC_CMSIS_DEV}
  ${INC_CMSIS_CORE}
  ${INC_RTOS_KERNEL}
  ${INC_RTOS_CMSISV2}
  ${FREERTOS_PORT_DIR}
  $<$<EXISTS:${INC_TGFX_APP}>:${INC_TGFX_APP}>
  $<$<EXISTS:${INC_TGFX_TGT_GEN}>:${INC_TGFX_TGT_GEN}>
  $<$<EXISTS:${INC_TGFX_TGT}>:${INC_TGFX_TGT}>
)

# -------- Defines (match log) --------
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

# Per-language standard/flags like CubeIDE
set_source_files_properties(${APP_CPP_SOURCES} PROPERTIES
  COMPILE_FLAGS "-std=gnu++14 -fno-exceptions -fno-rtti -fno-use-cxa-atexit"
)
set_source_files_properties(${APP_C_SOURCES}      PROPERTIES COMPILE_FLAGS "-std=gnu11")
set_source_files_properties(${FREERTOS_KERNEL_SRCS} PROPERTIES COMPILE_FLAGS "-std=gnu11")

# -------- Link options --------
target_link_options(${PROJECT_NAME}.elf PRIVATE
  ${MCU_FLAGS}
  -Wl,--gc-sections
  -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
  -Wl,--print-memory-usage
  -T ${LINKER_SCRIPT}
)

target_link_libraries(${PROJECT_NAME}.elf m c gcc)

# -------- Post-build artifacts --------
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
  COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}.elf>
  BYPRODUCTS ${PROJECT_NAME}.bin ${PROJECT_NAME}.hex
  COMMENT "Generating BIN/HEX and printing size"
)

# -------- Optional flash targets (no OpenOCD needed) --------
find_program(ST_FLASH st-flash)
if(ST_FLASH)
  add_custom_target(flash_st
    COMMAND ${ST_FLASH} --reset write ${PROJECT_NAME}.bin 0x08000000
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing with st-flash"
  )
endif()

find_program(PYOCD pyocd)
if(PYOCD)
  add_custom_target(flash_pyocd
    COMMAND ${PYOCD} flash -t stm32h743zitx ${PROJECT_NAME}.hex
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing with pyOCD"
  )
endif()

find_program(STM32PROG STM32_Programmer_CLI)
if(STM32PROG)
  add_custom_target(flash_cubeprog
    COMMAND ${STM32PROG} -c port=SWD mode=UR -w ${PROJECT_NAME}.bin 0x08000000 -v -rst
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing with STM32_Programmer_CLI"
  )
endif()

# Disassembly (optional)
add_custom_target(dump
  COMMAND ${CMAKE_OBJDUMP} -dC $<TARGET_FILE:${PROJECT_NAME}.elf> > ${PROJECT_NAME}.lst
  DEPENDS ${PROJECT_NAME}.elf
  COMMENT "Generating disassembly"
)


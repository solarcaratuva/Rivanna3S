# Shared board target helper.

if(NOT DEFINED BOARD)
  message(FATAL_ERROR "BOARD is not set. Pass -DBOARD=<board> (e.g. STM32H743ZITX).")
endif()

set(BOARD_ROOT "${CMAKE_SOURCE_DIR}/Drivers/${BOARD}")
if(NOT EXISTS "${BOARD_ROOT}")
  message(FATAL_ERROR "Board folder not found: ${BOARD_ROOT}")
endif()

set(BOARD_SETUP_DIR "${BOARD_ROOT}/${BOARD}_Setup")
set(BOARD_SETUP_INC "${BOARD_SETUP_DIR}/Inc")
set(BOARD_SETUP_SRC "${BOARD_SETUP_DIR}/Src")

file(GLOB BOARD_STARTUP_CANDIDATES "${BOARD_SETUP_SRC}/startup_*.s")
list(LENGTH BOARD_STARTUP_CANDIDATES BOARD_STARTUP_COUNT)
if(NOT BOARD_STARTUP_COUNT EQUAL 1)
  message(FATAL_ERROR "Expected 1 startup file in ${BOARD_SETUP_SRC}, found ${BOARD_STARTUP_COUNT}")
endif()
list(GET BOARD_STARTUP_CANDIDATES 0 STARTUP_S)

file(GLOB APP_C_SOURCES "${BOARD_SETUP_SRC}/*.c")
file(GLOB APP_CPP_SOURCES "${BOARD_SETUP_SRC}/*.cpp")
set_source_files_properties(${APP_CPP_SOURCES} PROPERTIES COMPILE_FLAGS "-std=gnu++14 -fno-exceptions -fno-rtti -fno-use-cxa-atexit")
set_source_files_properties(${APP_C_SOURCES} PROPERTIES COMPILE_FLAGS "-std=gnu11")

file(GLOB BOARD_HAL_DIRS LIST_DIRECTORIES true "${BOARD_ROOT}/STM32*xx_HAL_Driver")
list(LENGTH BOARD_HAL_DIRS BOARD_HAL_DIRS_COUNT)
if(BOARD_HAL_DIRS_COUNT LESS 1)
  message(FATAL_ERROR "No HAL driver folder found in ${BOARD_ROOT}")
endif()
list(GET BOARD_HAL_DIRS 0 BOARD_HAL_DIR)
set(BOARD_HAL_INC "${BOARD_HAL_DIR}/Inc")
set(BOARD_HAL_LEGACY_INC "${BOARD_HAL_DIR}/Inc/Legacy")

file(GLOB HAL_SRCS
  "${BOARD_HAL_DIR}/Src/stm32*_hal*.c"
  "${BOARD_HAL_DIR}/Src/stm32*_ll_*.c"
)

set(BOARD_CMSIS_DIR "${BOARD_ROOT}/CMSIS")
set(BOARD_CMSIS_INC "${BOARD_CMSIS_DIR}/Include")
set(BOARD_CMSIS_CORE_INC "")
if(EXISTS "${BOARD_CMSIS_DIR}/Core/Include")
  set(BOARD_CMSIS_CORE_INC "${BOARD_CMSIS_DIR}/Core/Include")
endif()
file(GLOB BOARD_CMSIS_DEVICE_DIRS LIST_DIRECTORIES true "${BOARD_CMSIS_DIR}/Device/ST/*/Include")
list(LENGTH BOARD_CMSIS_DEVICE_DIRS BOARD_CMSIS_DEVICE_DIRS_COUNT)
if(BOARD_CMSIS_DEVICE_DIRS_COUNT LESS 1)
  message(FATAL_ERROR "No CMSIS device include folder found in ${BOARD_CMSIS_DIR}/Device/ST")
endif()
list(GET BOARD_CMSIS_DEVICE_DIRS 0 BOARD_CMSIS_DEVICE_INC)

if(BOARD STREQUAL "STM32H743ZITX")
  set(BOARD_DEVICE_DEFINE STM32H743xx)
  set(MCU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
  set(BOARD_FREERTOS_PORT ARM_CM7)
  set(BOARD_LINKER_SCRIPT_FLASH "${BOARD_ROOT}/STM32H743ZITX_FLASH.ld")
  set(BOARD_LINKER_SCRIPT_RAM "${BOARD_ROOT}/STM32H743ZITX_RAM.ld")
elseif(BOARD STREQUAL "STM32G474RET6")
  set(BOARD_DEVICE_DEFINE STM32G474xx)
  set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
  set(BOARD_FREERTOS_PORT ARM_CM4F)
  set(BOARD_LINKER_SCRIPT_FLASH "${BOARD_ROOT}/STM32G474RETX_FLASH.ld")
  set(BOARD_LINKER_SCRIPT_RAM "${BOARD_ROOT}/STM32G474RETX_RAM.ld")
elseif(BOARD STREQUAL "STM32U5A9NJH6Q")
  set(BOARD_DEVICE_DEFINE STM32U5A9xx)
  set(MCU_FLAGS -mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard)
  set(BOARD_FREERTOS_PORT ARM_CM33)
  set(BOARD_LINKER_SCRIPT_FLASH "${BOARD_ROOT}/STM32U5A9NJHXQ_FLASH.ld")
  set(BOARD_LINKER_SCRIPT_RAM "${BOARD_ROOT}/STM32U5A9NJHXQ_RAM.ld")
else()
  message(FATAL_ERROR "Unknown BOARD '${BOARD}'. Add its configuration to cmake/board.cmake.")
endif()

set(FREERTOS_PORT "${BOARD_FREERTOS_PORT}" CACHE STRING "FreeRTOS GCC port directory name")

if(NOT DEFINED LINKER_SCRIPT)
  set(LINKER_SCRIPT "${BOARD_LINKER_SCRIPT_FLASH}" CACHE FILEPATH "LD script")
endif()

set(BOARD_DEFINES USE_HAL_DRIVER ${BOARD_DEVICE_DEFINE})
set(BOARD_COMPILE_OPTIONS)
if(DEFINED MCU_FLAGS)
  list(APPEND BOARD_COMPILE_OPTIONS ${MCU_FLAGS} ${OPT_FLAGS} ${WARN_FLAGS})
endif()
set(BOARD_LINK_OPTIONS
  ${MCU_FLAGS}
  -T ${LINKER_SCRIPT}
  -Wl,--gc-sections
  -static
  --specs=nano.specs
  -Wl,--start-group
  -lc -lm -lstdc++
  -lsupc++
  -Wl,--end-group
)

set(BOARD_INCLUDE_DIRS
  ${BOARD_SETUP_INC}
  ${CMAKE_SOURCE_DIR}/Common/Libs/Inc
  ${BOARD_HAL_INC}
  ${BOARD_HAL_LEGACY_INC}
  ${BOARD_CMSIS_DEVICE_INC}
  ${BOARD_CMSIS_INC}
  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
)

function(add_board BOARD_NAME)
  file(GLOB BOARD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp
  )
  if(NOT BOARD_SOURCES)
    message(FATAL_ERROR "No sources found for ${BOARD_NAME} in ${CMAKE_CURRENT_SOURCE_DIR}/Src")
  endif()

  set(BOARD_LIB_TARGET ${BOARD_NAME}_app)
  add_library(${BOARD_LIB_TARGET} OBJECT ${BOARD_SOURCES})
  target_include_directories(${BOARD_LIB_TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${BOARD_INCLUDE_DIRS}
  )
  target_compile_options(${BOARD_LIB_TARGET} PRIVATE ${BOARD_COMPILE_OPTIONS})
  target_compile_definitions(${BOARD_LIB_TARGET} PUBLIC
    ${BOARD_DEFINES}
  )
  target_link_libraries(${BOARD_LIB_TARGET} PUBLIC
    common
    freertos_kernel
  )

  set(BOARD_EXE_TARGET ${BOARD_NAME}.elf)
  add_executable(${BOARD_EXE_TARGET}
    ${STARTUP_S}
    ${APP_C_SOURCES}
    ${APP_CPP_SOURCES}
    ${HAL_SRCS}
    $<TARGET_OBJECTS:${BOARD_LIB_TARGET}>
  )
  set_target_properties(${BOARD_EXE_TARGET} PROPERTIES
    OUTPUT_NAME ${BOARD_NAME}.elf
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
  )
  target_include_directories(${BOARD_EXE_TARGET} PRIVATE
    ${BOARD_INCLUDE_DIRS}
  )
  target_compile_definitions(${BOARD_EXE_TARGET} PRIVATE
    DEBUG
    ${BOARD_DEFINES}
  )
  target_compile_options(${BOARD_EXE_TARGET} PRIVATE ${BOARD_COMPILE_OPTIONS} -fstack-usage)
  target_link_options(${BOARD_EXE_TARGET} PRIVATE ${BOARD_LINK_OPTIONS} -Wl,-Map=${BOARD_NAME}.map)
  target_link_libraries(${BOARD_EXE_TARGET}
    common
    freertos_kernel
    m c gcc
  )
endfunction()

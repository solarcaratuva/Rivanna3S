# Shared board target helper.

set(BOARD_DEFINES USE_HAL_DRIVER STM32H743xx)
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
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H743ZITX_Setup/Inc
  ${CMAKE_SOURCE_DIR}/Common/Libs/Inc
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
  ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
  ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
)

set(STARTUP_S ${CMAKE_SOURCE_DIR}/Drivers/STM32H743ZITX_Setup/Src/startup_stm32h743zitx.s)
file(GLOB APP_C_SOURCES
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H743ZITX_Setup/Src/*.c
)
file(GLOB APP_CPP_SOURCES
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H743ZITX_Setup/Src/*.cpp
)
file(GLOB HAL_SRCS
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal*.c
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_*.c
)
set_source_files_properties(${APP_CPP_SOURCES} PROPERTIES COMPILE_FLAGS "-std=gnu++14 -fno-exceptions -fno-rtti -fno-use-cxa-atexit")
set_source_files_properties(${APP_C_SOURCES} PROPERTIES COMPILE_FLAGS "-std=gnu11")

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

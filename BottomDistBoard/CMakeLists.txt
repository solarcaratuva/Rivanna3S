## Build TelemetryBoard sources as a static library for app_main

file(GLOB TELEMETRY_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp
)

add_library(telemetry_board STATIC ${TELEMETRY_SOURCES})

target_include_directories(telemetry_board PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc
  ${CMAKE_SOURCE_DIR}/Core/Inc
  ${CMAKE_SOURCE_DIR}/Drivers/STM32H743ZITX_Setup/Inc
  ${CMAKE_SOURCE_DIR}/Common/Drivers/Inc
  ${CMAKE_SOURCE_DIR}/Common/Libs/Inc
  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
  ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
  ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
)

# Inherit MCU/optimization flags from parent if present
if(DEFINED MCU_FLAGS)
  target_compile_options(telemetry_board PRIVATE ${MCU_FLAGS} ${OPT_FLAGS} ${WARN_FLAGS})
endif()

target_compile_definitions(telemetry_board PUBLIC
  USE_HAL_DRIVER
  STM32H743xx
)

target_link_libraries(telemetry_board PUBLIC
  common
  freertos_kernel
)

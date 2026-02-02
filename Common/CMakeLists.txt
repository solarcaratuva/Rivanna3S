## Build Common sources (Drivers + Libs) as a static library

file(GLOB COMMON_DRIVERS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Src/*.cpp
)

file(GLOB COMMON_LIBS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Src/*.cpp
)

file(GLOB COMMON_STM_LIBS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/STM_Libs/*.c
)

set(COMMON_SOURCES
  ${COMMON_DRIVERS_SOURCES}
  ${COMMON_LIBS_SOURCES}
  ${COMMON_STM_LIBS_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/startup.cpp
)

add_library(common STATIC ${COMMON_SOURCES})

target_include_directories(common PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/Inc
  ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Inc
  ${BOARD_SETUP_INC}
  ${CMAKE_SOURCE_DIR}/Core/Inc
  ${BOARD_HAL_INC}
  ${BOARD_HAL_LEGACY_INC}
  ${BOARD_CMSIS_DEVICE_INC}
  ${BOARD_CMSIS_INC}
  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
  ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
)

# Inherit MCU/optimization flags from parent if present
if(DEFINED MCU_FLAGS)
  target_compile_options(common PRIVATE ${MCU_FLAGS} ${OPT_FLAGS} ${WARN_FLAGS})
endif()

# Ensure common sees HAL/device/RTOS macros
target_compile_definitions(common PUBLIC
  USE_HAL_DRIVER
  ${BOARD_DEVICE_DEFINE}
)

set_target_properties(common PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
)

message(STATUS "Common: built ${COMMON_SOURCES}")

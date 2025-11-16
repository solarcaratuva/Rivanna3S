## Common utility/driver library
# Builds a static `common` target that can be consumed by other modules.

cmake_minimum_required(VERSION 3.0)

project(Common LANGUAGES CXX C)

set(COMMON_DRIVERS_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/Drivers/Inc)
set(COMMON_DRIVERS_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/Drivers/Src)
set(COMMON_MODULES_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/Modules/Inc)
set(COMMON_MODULES_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/Modules/Src)

file(GLOB COMMON_DRIVER_SOURCES CONFIGURE_DEPENDS
  "${COMMON_DRIVERS_SRC_DIR}/*.cpp"
)

file(GLOB COMMON_MODULE_SOURCES CONFIGURE_DEPENDS
  "${COMMON_MODULES_SRC_DIR}/*.cpp"
)

add_library(common STATIC
  ${COMMON_DRIVER_SOURCES}
  ${COMMON_MODULE_SOURCES}
)

target_include_directories(common
  PUBLIC
    ${COMMON_DRIVERS_INC_DIR}
    ${COMMON_MODULES_INC_DIR}
)

# Reuse the MCU compile/link flags defined at the top level
if(TARGET mcu_flags)
  target_link_libraries(common PUBLIC mcu_flags)
endif()

# Common drivers wrap HAL APIs, so inherit the HAL driver's PUBLIC includes/options.
if(TARGET STM32H7xx_HAL_DRIVER)
  target_link_libraries(common PUBLIC STM32H7xx_HAL_DRIVER)
endif()

set_target_properties(common PROPERTIES
  VERSION 0.1
  SOVERSION 0
)

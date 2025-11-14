## CMake for the PowerBoard module
# Builds a static library `powerboard` from the sources in this directory.

cmake_minimum_required(VERSION 3.0)

project(PowerBoard LANGUAGES CXX C)

# Sources and headers in this module
set(POWERBOARD_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/Inc)
set(POWERBOARD_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/Src)

file(GLOB POWERBOARD_SOURCES CONFIGURE_DEPENDS
	"${POWERBOARD_SRC_DIR}/*.cpp"
)

add_library(powerboard STATIC ${POWERBOARD_SOURCES})

# Export the module's public include dir
target_include_directories(powerboard
	PUBLIC
		${POWERBOARD_INC_DIR}
		${CMAKE_CURRENT_LIST_DIR}/../Common/Drivers/Inc  # for DigitalOut.h etc.
)

# If a global mcu_flags interface exists, inherit cpu/compiler flags
if(TARGET mcu_flags)
	target_link_libraries(powerboard PUBLIC mcu_flags)
endif()

# The PowerBoard code includes STM HAL headers (via Common/Drivers/Inc/DigitalOut.h),
# so link the HAL driver target so the HAL public include directories are available
# to consumers of `powerboard`.
if(TARGET STM32H7xx_HAL_DRIVER)
	target_link_libraries(powerboard PUBLIC STM32H7xx_HAL_DRIVER)
endif()

set_target_properties(powerboard PROPERTIES
	VERSION 0.1
	SOVERSION 0
)

## Usage: other targets should `target_link_libraries(<target> PRIVATE powerboard)`

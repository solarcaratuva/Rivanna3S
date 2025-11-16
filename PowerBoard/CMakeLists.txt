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
)

# PowerBoard depends on common drivers/utilities.
target_link_libraries(powerboard PUBLIC common)

set_target_properties(powerboard PROPERTIES
	VERSION 0.1
	SOVERSION 0
)

## Usage: other targets should `target_link_libraries(<target> PRIVATE powerboard)`

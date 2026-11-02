# Load board helper macros from the root CMake module folder.
include(${CMAKE_SOURCE_DIR}/cmake/board.cmake)
# Declare the TopDistBoard firmware target using shared board conventions.
add_board(TopDistBoard)

option(UART_UPLOAD_MOCK "Enable #129 UART upload with a streaming mock consumer" OFF)
if(UART_UPLOAD_MOCK)
  target_compile_definitions(TopDistBoard_app PRIVATE UART_UPLOAD_MOCK)
endif()

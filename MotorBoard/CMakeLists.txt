# Load board helper macros from the root CMake module folder.
include(${CMAKE_SOURCE_DIR}/cmake/board.cmake)
# Declare the MotorBoard firmware target using shared board conventions.
add_board(MotorBoard)

option(UART_UPLOAD_MOCK "Enable #129 UART upload with a streaming mock consumer on MotorBoard" ON)
if(UART_UPLOAD_MOCK)
  target_compile_definitions(MotorBoard_app PRIVATE UART_UPLOAD_MOCK)
endif()

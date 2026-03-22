# Load board helper macros from the root CMake module folder.
include(${CMAKE_SOURCE_DIR}/cmake/board.cmake)
# Declare the MotorBoard firmware target using shared board conventions.
add_board(MotorBoard)

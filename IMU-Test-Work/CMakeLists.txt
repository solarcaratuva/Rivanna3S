cmake_minimum_required(VERSION 3.10)
project(VN200_Serial_Test)

set(CMAKE_CXX_STANDARD 11)

# Include the current directory for headers
include_directories(.)

# Define the executable and the files it needs
add_executable(vn_test 
    main.cpp 
    vn200_linux.cpp 
    vn200_linux.h
)
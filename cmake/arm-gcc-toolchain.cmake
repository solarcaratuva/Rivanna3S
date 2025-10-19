# Generic GCC ARM embedded toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

find_program(ARM_GCC arm-none-eabi-gcc)
if(NOT ARM_GCC)
  message(FATAL_ERROR "arm-none-eabi-gcc not found in PATH")
endif()
get_filename_component(ARM_GCC_BIN_DIR "${ARM_GCC}" DIRECTORY)
set(TOOLCHAIN_PREFIX arm-none-eabi)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER   ${ARM_GCC_BIN_DIR}/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${ARM_GCC_BIN_DIR}/${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${ARM_GCC_BIN_DIR}/${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_AR           ${ARM_GCC_BIN_DIR}/${TOOLCHAIN_PREFIX}-ar)
set(CMAKE_OBJCOPY      ${ARM_GCC_BIN_DIR}/${TOOLCHAIN_PREFIX}-objcopy)
set(CMAKE_OBJDUMP      ${ARM_GCC_BIN_DIR}/${TOOLCHAIN_PREFIX}-objdump)
set(CMAKE_SIZE         ${ARM_GCC_BIN_DIR}/${TOOLCHAIN_PREFIX}-size)

# Match CubeIDE’s link model
set(CMAKE_EXE_LINKER_FLAGS_INIT "--specs=nano.specs --specs=nosys.specs")


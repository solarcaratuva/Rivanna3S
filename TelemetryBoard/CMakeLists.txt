include(${CMAKE_SOURCE_DIR}/cmake/board.cmake)
add_board(TelemetryBoard)

# FATFS middleware + SD card driver
add_library(fatfs STATIC
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FatFs/src/ff.c
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FatFs/src/ff_gen_drv.c
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FatFs/src/diskio.c
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FatFs/src/option/syscall.c
    ${CMAKE_SOURCE_DIR}/FATFS/App/fatfs.c
    ${CMAKE_SOURCE_DIR}/FATFS/Target/fatfs_sd.cpp
    ${CMAKE_SOURCE_DIR}/FATFS/Target/user_diskio.cpp
)
target_include_directories(fatfs PUBLIC
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/FatFs/src
    ${CMAKE_SOURCE_DIR}/FATFS/App
    ${CMAKE_SOURCE_DIR}/FATFS/Target
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
)
target_compile_options(fatfs PRIVATE ${BOARD_COMPILE_OPTIONS})
target_compile_definitions(fatfs PUBLIC ${BOARD_DEFINES})
target_link_libraries(fatfs PUBLIC common)

# Expose FATFS headers to the board app object library (needed by main.cpp)
target_link_libraries(TelemetryBoard_app PUBLIC fatfs)

# Link FATFS into the final ELF
target_link_libraries(TelemetryBoard.elf fatfs)

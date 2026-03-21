## Build STM HAL as a static library and export include paths.
# Use a canonical target name so the root and other consumers can link to it
# as `STM32G4xx_HAL_DRIVER`.

file(GLOB HAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.c"
)

# Build vendor HAL as a static archive to avoid relinking every consumer target.
add_library(STM32G4xx_HAL_DRIVER STATIC ${HAL_SOURCES})

target_include_directories(STM32G4xx_HAL_DRIVER PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${BOARD_SETUP_INC}
    ${BOARD_CMSIS_DEVICE_INC}
    ${BOARD_CMSIS_INC}
)

if(TARGET mcu_flags)
    target_link_libraries(STM32G4xx_HAL_DRIVER PUBLIC mcu_flags)
endif()

target_compile_definitions(STM32G4xx_HAL_DRIVER PUBLIC
    USE_HAL_DRIVER
    ${BOARD_DEVICE_DEFINE}
)

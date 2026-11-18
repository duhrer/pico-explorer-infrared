cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)
include(pimoroni_pico_import.cmake)

# Initialize the SDK
pico_sdk_init()

# Include required libraries
include(common/pimoroni_i2c)

include(common/pimoroni_bus)

include(libraries/pico_explorer/pico_explorer)

set(INFRARED pico-explorer-infrared)

project(${INFRARED} C CXX ASM)

# Add your source files
add_executable(${INFRARED}
    src/infrared.cpp
)

# Link required libraries
target_link_libraries(${INFRARED} LINK_PUBLIC
    pico_stdlib
    pico_explorer
    pico_graphics
    st7789
    hardware_pio
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${INFRARED})

# Disable USB serial and enable UART serial
pico_enable_stdio_usb(${INFRARED} 0)
pico_enable_stdio_uart(${INFRARED} 1)

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${INFRARED}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)



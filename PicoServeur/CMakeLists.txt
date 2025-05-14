# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(PicoServeur C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(PicoServeur PicoServeur.c )

pico_set_program_name(PicoServeur "PicoServeur")
pico_set_program_version(PicoServeur "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(PicoServeur 1)
pico_enable_stdio_usb(PicoServeur 0)

# Add the standard library to the build
target_link_libraries(PicoServeur
        pico_stdlib
        hardware_pwm
        hardware_gpio
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip
)

# Add the standard include files to the build
target_include_directories(PicoServeur PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${PICO_SDK_PATH}/lib/lwip/src/include
        ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
)

# Copier lwipopts.h dans le répertoire de build
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lwipopts.h ${CMAKE_CURRENT_BINARY_DIR}/lwipopts.h COPYONLY)

# Add any user requested libraries
target_link_libraries(PicoServeur 
        
        )

pico_add_extra_outputs(PicoServeur)

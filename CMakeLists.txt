# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# GaltonBoard - Simulação de Galton Board no display OLED da BitDogLab

cmake_minimum_required(VERSION 3.13)

# Configurações padrão
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configuração para o Raspberry Pi Pico
set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../pico-sdk CACHE PATH "Path to the Raspberry Pi Pico SDK")
include(pico_sdk_import.cmake)

project(galton_board C CXX ASM)

# Inicializa o SDK do Pico
pico_sdk_init()

# Adiciona os arquivos do display OLED
add_library(ssd1306_display STATIC
    include/ssd1306_i2c.c
)
target_include_directories(ssd1306_display PUBLIC inc)
target_link_libraries(ssd1306_display pico_stdlib hardware_i2c)

# Adiciona o executável principal
add_executable(galton_board src/lab-01-galton-board.c)

# Configura a saída para USB
pico_enable_stdio_uart(galton_board 0)
pico_enable_stdio_usb(galton_board 1)

# Adiciona as bibliotecas necessárias
target_link_libraries(galton_board
    ssd1306_display
    pico_stdlib
    hardware_i2c
)

# Gera o arquivo UF2 para carregar na BitDogLab
pico_add_extra_outputs(galton_board)

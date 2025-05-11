
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(galton-board C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(galton-board
  ./src/lab-01-galton-board.c
  ./src/ssd1306_i2c.c
)

# Nome e versão do programa
pico_set_program_name(galton-board "Galton Board")
pico_set_program_version(galton-board "1.0")

# Habilita saída USB (console via USB)
pico_enable_stdio_uart(galton-board 0)
pico_enable_stdio_usb(galton-board 1)

# Diretórios de include
target_include_directories(galton-board PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include
)

# Bibliotecas necessárias
target_link_libraries(galton-board
  pico_stdlib
  hardware_i2c
)

# Geração dos arquivos .uf2 etc.
pico_add_extra_outputs(galton-board)

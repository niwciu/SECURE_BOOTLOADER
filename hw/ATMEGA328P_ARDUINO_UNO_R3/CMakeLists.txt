############################################################################################################################
# file:  CMakeLists.txt
# brief: Bootloader build for ATmega328P / Arduino Uno R3
#
# usage:
#   cmake -S ./ -B Build -G"Unix Makefiles"
#   make -C Build all
#
# Optional variables:
#   DEVICE_ID   : 64-bit device identifier (default 0x0000000000000000)
#   ENCR_KEY    : 16-byte AES key in C initializer format
#############################################################################################################################
cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE ../config/AVR/Toolchain-AVR-gcc.cmake)

enable_language(ASM)

project(AVR_ATMEGA328P_BL)

#--------------------------------------------------------------------------------------------------------
# AVR chip configuration
#--------------------------------------------------------------------------------------------------------

set(F_CPU 16000000UL)
set(MMCU  atmega328p)

# Programmer
set(PROG_TYPE usbasp)

# Fuses:
#   L_FUSE 0xfd -> external crystal 8–16 MHz, full swing
#   H_FUSE 0xd8 -> BOOTSZ=00 (4 KB bootloader at 0x7000), BOOTRST=0 (boot from bootloader)
#   E_FUSE 0xfd -> BOD 2.7 V
#   LOCK   0xff -> no lock
set(L_FUSE  0xfd)
set(H_FUSE  0xd8)
set(E_FUSE  0xfd)
set(LOCK_BIT 0xcf)

# Start address for the linker (32KB - 4KB = 0x7000)
set(BOOT_START_ADDR 0x7000)

#--------------------------------------------------------------------------------------------------------
# Build options
#--------------------------------------------------------------------------------------------------------

if(NOT DEFINED DEVICE_ID)
    set(DEVICE_ID 0x0000000000000000)
endif()

if(NOT DEFINED ENCR_KEY)
    set(ENCR_KEY "{0x21,0xBB,0xA1,0x8D,0xF4,0x9B,0x1E,0x77,0x26,0x6F,0x80,0x92,0x4C,0x35,0xE6,0xB8}")
endif() 
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/parse_encr_key.cmake)
parse_encr_key("${ENCR_KEY}" ENCR_KEY)

#--------------------------------------------------------------------------------------------------------
# Project info
#--------------------------------------------------------------------------------------------------------

message(STATUS "")
message(STATUS "===========================================================================================")
message(STATUS "              ATmega328P Arduino Uno R3 — Bootloader configuration")
message(STATUS "-------------------------------------------------------------------------------------------")
message(STATUS "   DEVICE_ID = ${DEVICE_ID}")
message(STATUS "   ENCR_KEY  = ${ENCR_KEY}")
message(STATUS "   BOOT START= ${BOOT_START_ADDR}")
message(STATUS "===========================================================================================")
message(STATUS "")

#--------------------------------------------------------------------------------------------------------
# Include directories
#--------------------------------------------------------------------------------------------------------

set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/CRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/tiny-AES-c
)

#--------------------------------------------------------------------------------------------------------
# Sources
#--------------------------------------------------------------------------------------------------------

set(C_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main_app.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_key_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_status_led_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_usart_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/core_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_flash_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/CRC/crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/tiny-AES-c/aes.c
)

set(ASM_SRCS)

#--------------------------------------------------------------------------------------------------------
# Build
#--------------------------------------------------------------------------------------------------------

include_directories(${INCLUDE_DIRS})
add_definitions(-DF_CPU=${F_CPU} -DDEVICE_ID=${DEVICE_ID} -DENCR_KEY=${ENCR_KEY})

# Include compiler and linker flags from external file
include(../config/AVR/AVR_compiler_flags.cmake)
add_compile_options(${COMPILE_OPTIONS})

# Create the executable using the provided macro
add_AVR_executable(${CMAKE_PROJECT_NAME} ${C_SRCS} ${ASM_SRCS})

# FIX: Linker relocation must target the .elf file created by the macro
target_link_options(${CMAKE_PROJECT_NAME}.elf PRIVATE "-Wl,--section-start=.text=${BOOT_START_ADDR}")

AVR_link_libraries(${CMAKE_PROJECT_NAME})
#############################################################################################################################
# file:  CMakeLists.txt
# brief: Template for a new STM32 bootloader hardware target.
#
# usage: Copy this directory to hw/<YOUR_TARGET> and follow every TODO: comment.
#
#        For debug build using Unix Makefiles:
#          cmake -S ./ -B Debug -G"Unix Makefiles" -DBUILD_TYPE=Debug
#          make -C Debug all (optional with VERBOSE=1  and -jxx ->xx number of cores)
#        For debug build using Ninja:
#          cmake -S ./ -B Debug -G"Ninja" -DBUILD_TYPE=Debug
#          ninja -C Debug (optional with -V  and -jxx ->xx number of cores)
#
#        For release build Unix Makefiles:
#          cmake -S ./ -B Release -G"Unix Makefiles" -DBUILD_TYPE=Release
#          make all -C Release VERBOSE=1 -j12
#        For release build using Ninja:
#          cmake -S ./ -B Release -G"Ninja" -DBUILD_TYPE=Release
#          ninja -C Release (optional -V  and -jxx ->xx number of cores)
#############################################################################################################################

cmake_minimum_required(VERSION 3.20)

#--------------------------------------------------------------------------------------------------------
# TOOLCHAIN

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../config/STM32/Toolchain-arm-gcc.cmake)

#--------------------------------------------------------------------------------------------------------
# PROJECT
#--------------------------------------------------------------------------------------------------------

# TODO: rename to match your target (e.g. STM32L432KC, STM32H743ZI)
project(STM32_TEMPLATE C ASM)

set(CMAKE_COLOR_DIAGNOSTIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

#--------------------------------------------------------------------------------------------------------
# BUILD OPTIONS
#--------------------------------------------------------------------------------------------------------

if(NOT DEFINED BUILD_TYPE)
    set(BUILD_TYPE Debug)
endif()

if(NOT DEFINED CRC_MODULE)
    set(CRC_MODULE SW)
endif()

if(NOT DEFINED DEVICE_ID)
    set(DEVICE_ID 0x0000000000000000)
endif()

# Default encryption key (C initializer format)
if(NOT DEFINED ENCR_KEY)
    set(ENCR_KEY "{0x21,0xBB,0xA1,0x8D,0xF4,0x9B,0x1E,0x77,0x26,0x6F,0x80,0x92,0x4C,0x35,0xE6,0xB8}")
endif()

#--------------------------------------------------------------------------------------------------------
# ENCR KEY PARSING
#--------------------------------------------------------------------------------------------------------

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/parse_encr_key.cmake)
parse_encr_key("${ENCR_KEY}" ENCR_KEY)

#--------------------------------------------------------------------------------------------------------
# PROJECT CONFIG INFO
#--------------------------------------------------------------------------------------------------------

message(STATUS "")
message(STATUS "===========================================================================================")
# TODO: update the label below to match your target name
message(STATUS "                    HW_STM32_TEMPLATE project configuration")
message(STATUS "-------------------------------------------------------------------------------------------")
message(STATUS " Available options:")
message(STATUS "   BUILD_TYPE  : <Debug, Release> (default: Debug)")
message(STATUS "   CRC_MODULE  : <HW, SW> (default: SW)")
message(STATUS "   DEVICE_ID   : <64-bit hex value>")
message(STATUS "                 example: -DDEVICE_ID=0x00A0000BC22510E1")
message(STATUS "                 default: 0x0000000000000000")
message(STATUS "   ENCR_KEY    : <16 bytes key>")
message(STATUS "                 supported formats:")
message(STATUS "                   - \"{0xDE,0xAD,...}\"")
message(STATUS "                   - \"DE AD BE EF ...\"")
message(STATUS "                   - \"DEADBEEF...\"")
message(STATUS "")
message(STATUS " Current configuration:")
message(STATUS "   BUILD_TYPE  = ${BUILD_TYPE}")
message(STATUS "   CRC_MODULE  = ${CRC_MODULE}")
message(STATUS "   DEVICE_ID   = ${DEVICE_ID}")
message(STATUS "   ENCR_KEY    = ${ENCR_KEY}")
message(STATUS "===========================================================================================")
message(STATUS "")

#--------------------------------------------------------------------------------------------------------
# USER CONFIGURATION
#--------------------------------------------------------------------------------------------------------

# ========================= INCLUDES =========================
set(APP_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup

    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    # TODO: add vendor-specific CMSIS driver folder if needed
    # ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/STM32XX_Drivers

    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface

    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/CRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/tiny-AES-c
)

# ========================= SOURCES =========================
set(APP_C_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/sysmem.c

    # system_stm32xxxx.c — BOOTLOADER APPROACH: keep commented out.
    #   SystemInit() call is commented out in the startup file, so this file
    #   is not needed. The MCU boots at its HSI reset-default clock (8–16 MHz
    #   depending on family), which is sufficient for 115200 baud UART.
    #   SystemInit is declared .weak in the startup so the linker is happy.
    #   See Core/Startup/PLACEHOLDER.md for the required startup modification.
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/system_stm32xxxx.c

    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main_app.c

    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_key_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_status_led_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_usart_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/core_drv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_flash_drv.c

    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/tiny-AES-c/aes.c
)

# ========================= CRC MODULE =========================
if(CRC_MODULE STREQUAL "SW")
    list(APPEND APP_C_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/CRC/crc.c
    )
else()
    list(APPEND APP_C_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_crc_drv.c
    )
endif()

# ========================= ASM SOURCES =========================
set(APP_ASM_SOURCES
    # TODO: replace with the correct startup file for your MCU
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32xxxxxx.s
)

# ========================= DEFINES =========================
set(APP_DEFINES
    # TODO: replace with the correct CMSIS device define for your MCU
    # Examples: STM32L432xx  STM32H743xx  STM32F446xx
    STM32XXXXXX
)

# ========================= LIBRARIES =========================
set(APP_LIBS
    # tiny_AES_c
)

#--------------------------------------------------------------------------------------------------------
# COMPILER FLAGS
#--------------------------------------------------------------------------------------------------------

# TODO: set the correct Cortex-M core for your MCU
# Examples:
#   -mcpu=cortex-m0plus   (STM32G0, STM32L0)
#   -mcpu=cortex-m4       (STM32G4, STM32F4, STM32L4)
#   -mcpu=cortex-m7       (STM32H7, STM32F7)
#   -mcpu=cortex-m33      (STM32U5, STM32WL5)
set(MCPU "-mcpu=cortex-m0plus")

# TODO: rename to match your linker script file if it differs from linker.ld
set(LD_FILE "linker.ld")

include(${CMAKE_CURRENT_SOURCE_DIR}/../config/STM32/compiler_flags.cmake)

#--------------------------------------------------------------------------------------------------------
# TARGET
#--------------------------------------------------------------------------------------------------------

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        ${APP_C_SOURCES}
        ${APP_ASM_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${APP_INCLUDE_DIRS}
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ${APP_DEFINES}
        BUILD_TYPE=${BUILD_TYPE}
        DEVICE_ID=${DEVICE_ID}
        ENCR_KEY=${ENCR_KEY}
)

#--------------------------------------------------------------------------------------------------------
# EXTERNAL LIBRARIES
#--------------------------------------------------------------------------------------------------------

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/tiny-AES-c tiny_AES_c)

#--------------------------------------------------------------------------------------------------------
# STM32CubeMX (optional)
#--------------------------------------------------------------------------------------------------------

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cmake/stm32cubemx)

#--------------------------------------------------------------------------------------------------------
# LINKING
#--------------------------------------------------------------------------------------------------------

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${APP_LIBS}
)

#--------------------------------------------------------------------------------------------------------
# POST BUILD
#--------------------------------------------------------------------------------------------------------

add_arm_executable(${PROJECT_NAME})

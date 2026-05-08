#############################################################################################################################
# file:  CMakeLists.txt
# brief: Modernized STM32 template (external toolchain + flags preserved)

# usage: Edit MCPU-Variable to suit STM32 project requirements.
#        For debug build using Unix Makefiles:
#          cmake -S ./ -B Debug -G"Unix Makefiles" -DBUILD_TYPE=Debug
#          make -C Debug all (optional with VERBOSE=1  and -jxx ->xx numnber of cores)
#        For debug build using Ninja:
#          cmake -S ./ -B Debug -G"Ninja" -DBUILD_TYPE=Debug
#          ninja -C Debug (optional with -V  and -jxx ->xx numnber of cores)
# 
#        For release build Unix Makefiles:
#          cmake -S ./ -B Release -G"Unix Makefiles" -DBUILD_TYPE=Release
#          make all -C Release VERBOSE=1 -j12 (optional with VERBOSE=1  and -jxx ->xx numnber of cores)
#        For Release build using Ninja:
#          cmake -S ./ -B Release -G"Ninja" -DBUILD_TYPE=Release
#          ninja -C Release (optional -V  and -jxx ->xx numnber of cores)
#############################################################################################################################

cmake_minimum_required(VERSION 3.20)

#--------------------------------------------------------------------------------------------------------
# TOOLCHAIN

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../config/STM32/Toolchain-arm-gcc.cmake)

#--------------------------------------------------------------------------------------------------------
# PROJECT
#--------------------------------------------------------------------------------------------------------

project(STM32GO71RB C ASM)

set(CMAKE_COLOR_DIAGNOSTIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

#--------------------------------------------------------------------------------------------------------
# BUILD TYPE 
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

# Include shared parser (located next to config directory)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/parse_encr_key.cmake)

# Normalize ENCR_KEY to C initializer format
parse_encr_key("${ENCR_KEY}" ENCR_KEY)


#--------------------------------------------------------------------------------------------------------
# PROJECT CONFIG USER INFO
#--------------------------------------------------------------------------------------------------------

message(STATUS "")
message(STATUS "===========================================================================================")
message(STATUS "                    HW_STM32G071RB project configuration")
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
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/STM32G0_Drivers
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
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/CRC/crc_sw.c
    )
else()
    list(APPEND APP_C_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Bl_drv_interface/bl_crc_drv.c
    )
endif()

# ========================= ASM SOURCES =========================
set(APP_ASM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Startup/startup_stm32g070rbtx.s
)

# ========================= DEFINES =========================
set(APP_DEFINES
    STM32G071xx
)

# ========================= LIBRARIES =========================
set(APP_LIBS
    # tiny_AES_c
)

#--------------------------------------------------------------------------------------------------------
# COMPILER FLAGS 
#--------------------------------------------------------------------------------------------------------

set(MCPU "-mcpu=cortex-m0plus")
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
# STM32CubeMX (optional if cubemx is used for generate hw config as cmake project)
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

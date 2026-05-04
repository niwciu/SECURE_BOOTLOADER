#############################################################################################################################
# file:  CMakeLists.txt
# brief: Template "CMakeLists.txt" for building of executables and static libraries.
#
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
cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE ../config/STM32/Toolchain-arm-gcc.cmake)

project(NUCLEO_GO71RB)
# Allow assembler
enable_language(ASM)

set(CMAKE_COLOR_DIAGNOSTIC ON)
# json file compilation base generation -> usefull for cppcheck
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

# project specific define 
if(NOT DEFINED BUILD_TYPE)
    set(BUILD_TYPE Debug)
endif()


# Informational messages for the user
message(STATUS "")
message(STATUS "================================================================")
message(STATUS " STM32G071RB_TEMPLATE project configuration")
message(STATUS "----------------------------------------------------------------")
message(STATUS " Available options:")
message(STATUS "   BUILD_TYPE              : <Debug, Release> (default: Debug)")
message(STATUS "")
message(STATUS " Current configuration:")
message(STATUS "   BUILD_TYPE              = ${BUILD_TYPE}")
message(STATUS "===============================================================")

set(INCLUDE_DIRS
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/led
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modbus_slave
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/keypad
)

set(C_SRCS
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main_app.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/led/led.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modbus_slave/modbus_slave_manager.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/keypad/keypad.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/hal_utils.c

    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/driver_interfaces/pushbutton_GPIO_interface.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/driver_interfaces/led_driver_interface.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/driver_interfaces/modbus_slave_driver_interface.c
)

set(ASM_SRCS
    # startup_stm32g071xx.s
)

set(GLOBAL_DEFINES
    -DBUILD_TYPE=${BUILD_TYPE}
)

set(LINKED_LIBS
    # modbus_lib
    # pushbutton_lib
)

# link lib directory - complied library *.a files dir 
link_directories(

)

include_directories(${INCLUDE_DIRS})
add_definitions(${GLOBAL_DEFINES})

#############################################################################################################################
# COMPILER FLAGS
#############################################################################################################################

set(MCPU "-mcpu=cortex-m0plus")
set(LD_FILE "STM32G071XX_FLASH.ld")
include(    ../config/STM32/compiler_flags.cmake)

message(STATUS "C flags: ${CMAKE_C_FLAGS}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")


# add lib directory - lib to compile
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/MODBUS modbus_build)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/PUSHBUTTON pushbutton_build)


# generate elf file
add_executable(${CMAKE_PROJECT_NAME} ${C_SRCS} ${ASM_SRCS})

# add STM32CUBEMX SOURCES
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cmake/stm32cubemx)

# POSTPROCESS FIRMWARE (HEX, BIN, LSS, DMP)
add_arm_executable(${CMAKE_PROJECT_NAME})

# link libraries
arm_link_libraries(${CMAKE_PROJECT_NAME} ${LINKED_LIBS})


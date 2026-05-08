```cmake
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

project(NUCLEO_GO71RB C ASM)

set(CMAKE_COLOR_DIAGNOSTIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

#--------------------------------------------------------------------------------------------------------
# BUILD TYPE 
#--------------------------------------------------------------------------------------------------------

if(NOT DEFINED BUILD_TYPE) 
    set(BUILD_TYPE Debug) 
endif()

#--------------------------------------------------------------------------------------------------------
# PROJECT CONFIG USER INFO
#--------------------------------------------------------------------------------------------------------

message(STATUS "")
message(STATUS "===========================================================================================")
message(STATUS "                    STM32G070RB_TEMPLATE project configuration")
message(STATUS "-------------------------------------------------------------------------------------------")
message(STATUS " Available options:")
message(STATUS "   BUILD_TYPE  : <Debug, Release> (default: Debug)")
message(STATUS "")
message(STATUS " Current configuration:")
message(STATUS "   BUILD_TYPE  = ${BUILD_TYPE}")
message(STATUS "===========================================================================================")
message(STATUS "")

#--------------------------------------------------------------------------------------------------------
# USER CONFIGURATION
#--------------------------------------------------------------------------------------------------------

# ========================= INCLUDES =========================
set(APP_INCLUDE_DIRS
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/led
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modbus_slave
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/keypad
)

# ========================= SOURCES =========================
set(APP_C_SOURCES
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main_app.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/led/led.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modbus_slave/modbus_slave_manager.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/keypad/keypad.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils/hal_utils.c

    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/driver_interfaces/pushbutton_GPIO_interface.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/driver_interfaces/led_driver_interface.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Core/driver_interfaces/modbus_slave_driver_interface.c
)

# ========================= ASM SOURCES =========================
set(APP_ASM_SOURCES
    # ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32g071xx.s
)

# ========================= DEFINES =========================
set(APP_DEFINES
    # USE_FULL_LL_DRIVER
    # STM32G071xx
)

# ========================= LIBRARIES =========================
set(APP_LIBS
    # modbus_lib
    # pushbutton_lib
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
        BUILD_TYPE=$<CONFIG>
)

#--------------------------------------------------------------------------------------------------------
# EXTERNAL LIBRARIES
#--------------------------------------------------------------------------------------------------------

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/MODBUS modbus_build)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/PUSHBUTTON pushbutton_build)

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

#--------------------------------------------------------------------------------------------------------
# STATUS OUTPUT
#--------------------------------------------------------------------------------------------------------

# message(STATUS "")
# message(STATUS "================================================================")
# message(STATUS " STM32G071RB_TEMPLATE project configuration")
# message(STATUS "----------------------------------------------------------------")
# message(STATUS " Available options:")
# message(STATUS "   CMAKE_BUILD_TYPE       : <Debug, Release> (default: Debug)")
# message(STATUS "")
# message(STATUS " Current configuration:")
# message(STATUS "   CMAKE_BUILD_TYPE       = ${CMAKE_BUILD_TYPE}")
# message(STATUS "   CPU                    = ${MCPU}")
# message(STATUS "   LINKER FILE            = ${LD_FILE}")
# message(STATUS "")
# message(STATUS " Sources:")
# message(STATUS "   C                      = ${APP_C_SOURCES}")
# message(STATUS "   ASM                    = ${APP_ASM_SOURCES}")
# message(STATUS "")
# message(STATUS " Include dirs             = ${APP_INCLUDE_DIRS}")
# message(STATUS " Defines                  = ${APP_DEFINES}")
# message(STATUS " Linked libs              = ${APP_LIBS}")
# message(STATUS "===============================================================")

# # debug flags (from external file)
# message(STATUS "C flags: ${CMAKE_C_FLAGS}")
# message(STATUS "CXX flags: ${CMAKE_CXX_FLAGS}")



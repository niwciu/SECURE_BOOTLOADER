##############################################################################
# FILE:  CMakeLists.txt
# BRIEF: MAIN_APP unit-test build using Unity + CMock.
#
# USAGE:
#       1a. cmake -S ./ -B out -G "Unix Makefiles"
#           or
#       1b. cmake -S ./ -B out -G "Ninja"
#       2. cd out
#       3. cmake --build . --parallel (optional --verbose)
#
# ADDITIONAL CUSTOM TARGETS (defined in custom_targets.cmake):
#   - make/ninja ccm   -> code complexity metrics (console)
#   - make/ninja ccmr  -> code complexity report
#   - make/ninja cppcheck -> static analysis (src + test)
#   - make/ninja ccc   -> code coverage check
#   - make/ninja ccr   -> code coverage report
##############################################################################

cmake_minimum_required(VERSION 3.20)
project(MAIN_APP_test LANGUAGES C)

# --- UNITY FRAMEWORK ---
add_subdirectory(../unity unity)

# --- CMOCK FRAMEWORK ---
add_subdirectory(../cmock/src cmock)

# --- CUSTOM TARGETS FILE (MANDATORY, FAIL IF MISSING) ---
set(CUSTOM_TARGETS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/custom_targets.cmake")
if(EXISTS "${CUSTOM_TARGETS_FILE}")
    include("${CUSTOM_TARGETS_FILE}")
else()
    message(FATAL_ERROR
        "Missing required file: ${CUSTOM_TARGETS_FILE}\n"
        "This file defines custom build targets (cppcheck, lizard, gcovr, etc.) "
        "and must be present in the repository.")
endif()

# --- MOCK FUNCTION GENERATOR INCLUDE ---
include(${CMAKE_CURRENT_SOURCE_DIR}/generate_mock.cmake)

# --- DETECT LIBM ---
include(CheckLibraryExists)
check_library_exists(m sin "" HAVE_LIB_M)

# --- INCLUDE DIRECTORIES ---
# Test directory listed FIRST so test-specific bl_hw_config.h shadows the real one.
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/CRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/tiny-AES-c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hw/STM32G070RB/Core/Bl_drv_interface
)

# --- INCLUDE TEST DIRECTORIES ---
set(TEST_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# --- MOCK GENERATION SETTINGS ---
set(MOCK_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hw/STM32G070RB/Core/Bl_drv_interface/bl_usart_drv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hw/STM32G070RB/Core/Bl_drv_interface/bl_flash_drv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hw/STM32G070RB/Core/Bl_drv_interface/core_drv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hw/STM32G070RB/Core/Bl_drv_interface/bl_key_drv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hw/STM32G070RB/Core/Bl_drv_interface/bl_status_led_drv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/CRC/crc_api.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/tiny-AES-c/aes.h
)
set(MOCK_TARGET_NAME generate_mocks)
set(MOCK_OUTPUT_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/cmocks)

# --- MOCK GENERATION ---
generate_mock(
    "${MOCK_HEADERS}"
    "${MOCK_TARGET_NAME}"
    "${MOCK_OUTPUT_DIR}"
    MOCK_C_FILES
)

# --- SOURCES UNDER TEST ---
set(SRCS
    ../../src/main_app.c
)

# --- TEST SOURCES ---
set(TEST_SRCS
    main_app_test_main.c
    main_app_test_runner.c
    main_app_init_test.c
    main_app_test.c
    main_app_rx_fsm_test.c
    main_app_com_fsm_test.c
    main_app_start_cmd_test.c
    main_app_page_cmd_test.c
    main_app_exit_test.c
    ${MOCK_C_FILES}
)

# --- CREATE TEST EXECUTABLE ---
add_executable(${PROJECT_NAME}
    ${SRCS}
    ${TEST_SRCS}
)

# --- ENSURE MOCKS ARE GENERATED FIRST ---
add_dependencies(${PROJECT_NAME} ${MOCK_TARGET_NAME})

# --- DEFINES ---
# DEVICE_ID and ENCR_KEY mirror the defaults from hw/cmake/parse_encr_key.cmake.
set(TEST_ENCR_KEY "{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F}")
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        UNIT_TESTS
        DEVICE_ID=0x0000000000000001
        ENCR_KEY=${TEST_ENCR_KEY}
)

# --- COMPILER OPTIONS ---
target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -g3
        $<$<C_COMPILER_ID:GNU>:-fdiagnostics-color=always>
        $<$<C_COMPILER_ID:Clang>:-fcolor-diagnostics>
)

# --- INCLUDE DIRECTORIES ---
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${INCLUDE_DIRS}
        ${TEST_INCLUDE_DIRS}
        ${MOCK_OUTPUT_DIR}
)

# --- LINKED LIBRARIES ---
set(LINKED_LIBS
    unity
    cmock
)
if(HAVE_LIB_M)
    list(APPEND LINKED_LIBS m)
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${LINKED_LIBS}
)

# --- CODE COVERAGE OPTIONS ---
target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage -O0)
target_link_options(${PROJECT_NAME}    PRIVATE -fprofile-arcs -O0)

##############################################################################
# --- ENABLE TESTING ---
##############################################################################
enable_testing()
add_test(NAME ${PROJECT_NAME}
    COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME} -v
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

##############################################################################
# END OF FILE
##############################################################################

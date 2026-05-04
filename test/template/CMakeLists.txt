#############################################################################################################################
# FILE:  CMakeLists.txt
# BRIEF: Template "CMakeLists.txt" for building Unit test modules using Unity + CMock.
#
# USAGE:
#       1a. cmake -S ./ -B out -G "Unix Makefiles"   
#			or
#		1b. cmake -S ./ -B out -G "Ninja"
#       2. cd out
#       3. cmake --build . --parallel (optional --verbose)
#
# ADDITIONAL CUSTOM TARGETS (defined in custom_targets.cmake):
#   - make/ninja ccm   -> code complexity metrics (console)
#   - make/ninja ccmr  -> code complexity report
#   - make/ninja cppcheck -> static analysis (src + test)
#   - make/ninja ccc   -> code coverage check
#   - make/ninja ccr   -> code coverage report
#############################################################################################################################

cmake_minimum_required(VERSION 3.20)
project(template_test LANGUAGES C)

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

# --- INCLUDE SRC DIRECTORIES ---
set(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/template
)

# --- INCLUDE TEST DIRECTORIES ---
set(TEST_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# --- MOCK GENERATION SETTINGS ---
set(MOCK_HEADERS 
    # Add here paths to headers for which mocks should be generated
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../../src/MODULE/module.h
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
    # ${CMAKE_CURRENT_SOURCE_DIR}/../../src/tested_module_folder/tested_module.c
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/template/template.c
)

# --- TEST SOURCES ---
set(TEST_SRCS
    template_test_main.c
    template_test_runner.c
    template_test.c
    # mock_module.c
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
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        UNIT_TESTS
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

# --- CODE COVERAGE OPTIONS (ALWAYS ENABLED FOR TEST BUILDS) ---
# Unit tests are always built with debug info and coverage instrumentation.
# No need to depend on CMAKE_BUILD_TYPE (embedded tests never use -O2/-O3 optimizations).
target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage -O0)
target_link_options(${PROJECT_NAME}  PRIVATE -fprofile-arcs -O0)

#############################################################################################################################
# --- ENABLE TESTING (OPTIONAL) ---
#############################################################################################################################
# Enables CTest integration.
# After building, run:
#   ctest --output-on-failure --verbose
# Full logs are available in: build/Testing/Temporary/LastTest.log
#############################################################################################################################

enable_testing()
add_test(NAME ${PROJECT_NAME}
    COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME} -v
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

#############################################################################################################################
# END OF FILE
#############################################################################################################################

# System Generic - no OS bare-metal application
set(CMAKE_SYSTEM_NAME Generic)

# Setup arm processor and bleeding edge toolchain
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_NM arm-none-eabi-nm)
set(CMAKE_STRIP arm-none-eabi-strip)
set(CMAKE_RANLIB arm-none-eabi-ranlib)
set(CMAKE_SIZE arm-none-eabi-size)

# When trying to link cross compiled test program, error occurs, so setting test compilation to static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Don't know if following setting works also for Ninja
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Remove default static libraries for win32
set(CMAKE_C_STANDARD_LIBRARIES "")


function(add_arm_executable TARGET)

    # Optional custom ELF filename
    if (ARGC GREATER 1)
        set(ELF_NAME ${ARGV1})
    else()
        set(ELF_NAME "${TARGET}.elf")
    endif()

    #
    # 1. Set ELF output filename
    #
    set_target_properties(${TARGET}
        PROPERTIES
            OUTPUT_NAME ${ELF_NAME}
            SUFFIX ""        # Prevent double extensions
    )

    # Full ELF path as known by CMake
    set(ELF_PATH $<TARGET_FILE:${TARGET}>)

    #
    # 2. Artifact filenames (based on TARGET, not ELF_NAME)
    #
    set(HEX_FILE ${TARGET}.hex)
    set(BIN_FILE ${TARGET}.bin)
    set(LSS_FILE ${TARGET}.lss)
    set(DMP_FILE ${TARGET}.dmp)

    #
    # 3. Generate artifacts
    #
    add_custom_command(
        OUTPUT ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${ELF_PATH} ${HEX_FILE}
        DEPENDS ${TARGET}
        COMMENT "Generating HEX for ${TARGET}"
    )

    add_custom_command(
        OUTPUT ${BIN_FILE}
        COMMAND ${CMAKE_OBJCOPY} -O binary ${ELF_PATH} ${BIN_FILE}
        DEPENDS ${TARGET}
        COMMENT "Generating BIN for ${TARGET}"
    )

    add_custom_command(
        OUTPUT ${LSS_FILE}
        COMMAND ${CMAKE_OBJDUMP} -h -S ${ELF_PATH} > ${LSS_FILE}
        DEPENDS ${TARGET}
        COMMENT "Generating LSS for ${TARGET}"
    )

    add_custom_command(
        OUTPUT ${DMP_FILE}
        COMMAND ${CMAKE_OBJDUMP} -x --syms ${ELF_PATH} > ${DMP_FILE}
        DEPENDS ${TARGET}
        COMMENT "Generating DMP for ${TARGET}"
    )

    #
    # 4. Size print command
    #    This runs AFTER artifacts are created.
    #
    add_custom_command(
        OUTPUT ${TARGET}.sizeinfo
        COMMAND ${CMAKE_SIZE} --format=berkeley ${ELF_NAME} ${HEX_FILE} 
        DEPENDS
            ${HEX_FILE}
            ${BIN_FILE}
            ${LSS_FILE}
            ${DMP_FILE}
        COMMENT "Printing size of ELF and HEX for ${TARGET}"
    )

    #
    # 5. Global target — executed in "make all"
    #
    add_custom_target(${TARGET}_artifacts ALL
        DEPENDS
            ${HEX_FILE}
            ${BIN_FILE}
            ${LSS_FILE}
            ${DMP_FILE}
            ${TARGET}.sizeinfo
    )

	# Check if programmer software is available 
    find_program(STM32_Programmer STM32_Programmer_CLI)
    if(STM32_Programmer)
        message(STATUS "STM32 Programmer was found, you can work with your device using targets: \r\n\t flash,\r\n\t erase,\r\n\t reset.")

        # add "flash" command for programming the uC
        add_custom_target(flash COMMAND ${STM32_Programmer} --connect port=SWD --write ${ELF_NAME} --verify -rst DEPENDS ${TARGET_NAME})
        add_custom_target(erase COMMAND ${STM32_Programmer} -c port=SWD -e all)
        add_custom_target(reset COMMAND ${STM32_Programmer} -c port=SWD -rst)
    else()
        message(STATUS "STM32 Programmer was not found.")  
    endif()

endfunction(add_arm_executable)

macro(arm_link_libraries TARGET)

target_link_libraries(${TARGET} ${ARGN})

endmacro(arm_link_libraries)

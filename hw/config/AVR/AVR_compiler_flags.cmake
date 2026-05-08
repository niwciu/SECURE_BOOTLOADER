#--------------------------------------------------------------------------------------------------------
# AVR Compiler & Linker Options
#--------------------------------------------------------------------------------------------------------

set(COMPILE_OPTIONS
    -mmcu=${MMCU} 
    -std=gnu99 
    -Os 
    -Wall
    -Wno-main
    -Wundef
    -Wstrict-prototypes
    -Werror
    -Wfatal-errors
    -g
    -gdwarf-2
    -funsigned-char
    -funsigned-bitfields
    -fpack-struct
    -fshort-enums
    -flto
    -ffunction-sections
    -fdata-sections
    -fno-exceptions
    -fno-split-wide-types
    -fno-tree-scev-cprop
    -ffreestanding # flag required by util/delay.h 
)

# Standard linker flags - use spaces instead of semicolons
set(LD_FLAGS "-mmcu=${MMCU} -Wl,--relax -Wl,--gc-sections -flto -Wl,-Map=${CMAKE_PROJECT_NAME}.map")

# Apply global linker flags
set(CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS}")


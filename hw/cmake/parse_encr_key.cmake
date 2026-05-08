# parse_encr_key.cmake
# Function converts ENCR_KEY into C initializer format {0x..,...}
# Supports:
#   - "{0x..,...}"
#   - "AA BB CC ..."
#   - "AABBCC..."

function(parse_encr_key INPUT_KEY OUTPUT_VAR)

    # If already in C initializer format -> just return
    if(INPUT_KEY MATCHES "^\\{.*\\}$")
        set(${OUTPUT_VAR} "${INPUT_KEY}" PARENT_SCOPE)
        return()
    endif()

    set(_key_raw "${INPUT_KEY}")

    # Remove spaces
    string(REPLACE " " "" _key_compact "${_key_raw}")

    # Case 1: continuous hex string
    if(_key_compact MATCHES "^[0-9A-Fa-f]+$")

        string(LENGTH "${_key_compact}" _len)
        if(NOT _len EQUAL 32)
            message(FATAL_ERROR "ENCR_KEY must contain exactly 16 bytes (32 hex chars)")
        endif()

        set(_key_bytes "")
        foreach(i RANGE 0 15)
            math(EXPR idx "2*${i}")
            string(SUBSTRING "${_key_compact}" ${idx} 2 _byte)
            string(APPEND _key_bytes "0x${_byte},")
        endforeach()

    # Case 2: space-separated
    elseif(_key_raw MATCHES "^[0-9A-Fa-f ]+$")

        string(REPLACE " " ";" _key_list "${_key_raw}")

        list(LENGTH _key_list _len)
        if(NOT _len EQUAL 16)
            message(FATAL_ERROR "ENCR_KEY must contain exactly 16 bytes")
        endif()

        set(_key_bytes "")
        foreach(byte ${_key_list})
            string(APPEND _key_bytes "0x${byte},")
        endforeach()

    else()
        message(FATAL_ERROR "Unsupported ENCR_KEY format")
    endif()

    # Remove trailing comma
    string(REGEX REPLACE ",$" "" _key_bytes "${_key_bytes}")

    # Wrap into initializer
    set(${OUTPUT_VAR} "{${_key_bytes}}" PARENT_SCOPE)

endfunction()
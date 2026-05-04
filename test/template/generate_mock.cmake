# --- GENERATE_MOCK FUNCTION (MODERN CMAKE STYLE) ---
function(generate_mock HEADERS TARGET_NAME OUTPUT_DIR OUT_VAR)
    # --- ENSURE OUTPUT DIRECTORY EXISTS ---
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    set(GENERATED_MOCKS)
    set(ALL_COMMANDS)
    set(ALL_DEPENDS)

    foreach(HEADER ${HEADERS})
        get_filename_component(HEADER_ABS "${HEADER}" ABSOLUTE)
        get_filename_component(HEADER_NAME "${HEADER_ABS}" NAME_WE)
        set(MOCK_FILE "${OUTPUT_DIR}/mock_${HEADER_NAME}.c")
        set(YAML_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmock_config/${HEADER_NAME}.yml")

        if(NOT EXISTS ${YAML_FILE})
            file(WRITE ${YAML_FILE}
                ":cmock:
  :mock_prefix: \"mock_\"
  :callback: true
  :plugins:
    - expect
    - expect_any_args
    - ignore
    - return_thru_ptr
    - callback")
        endif()

        # Add commands to the list
        list(APPEND ALL_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E echo "Generating mock for ${HEADER_NAME}.h"
            COMMAND ${RUBY_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../cmock/lib/cmock.rb
                    ${HEADER_ABS} --mock_path=${OUTPUT_DIR} -o${YAML_FILE}
        )
        
        # Collect all dependencies
        list(APPEND ALL_DEPENDS ${HEADER_ABS} ${YAML_FILE})
        
        list(APPEND GENERATED_MOCKS ${MOCK_FILE})
    endforeach()

    # Add cmock.rb as dependency
    list(APPEND ALL_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../cmock/lib/cmock.rb)

    # --- CREATE SINGLE TARGET FOR ALL MOCKS ---
    add_custom_target(${TARGET_NAME}
        ${ALL_COMMANDS}
        DEPENDS ${ALL_DEPENDS}
        BYPRODUCTS ${GENERATED_MOCKS}
        COMMENT "Generating all mock files"
        VERBATIM
    )

    # --- CLEAN GENERATED MOCKS ON 'MAKE CLEAN' ---
    set_directory_properties(PROPERTIES
        ADDITIONAL_MAKE_CLEAN_FILES "${OUTPUT_DIR}"
    )

    # --- RETURN GENERATED FILE LIST TO CALLER ---
    set(${OUT_VAR} ${GENERATED_MOCKS} PARENT_SCOPE)
endfunction()
function(compile_shaders TARGET_NAME)
  set(SHADER_SOURCE_FILES ${ARGN})
  set(SHADER_TARGETS)

  foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
    cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
    cmake_path(GET SHADER_SOURCE FILENAME SHADER_FILENAME)
    string(REPLACE "." "_" SHADER_NAME "${SHADER_FILENAME}.spv")
    
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${SHADER_NAME}.dir")
    # Build command
    list(APPEND SHADER_COMMAND COMMAND)
    list(APPEND SHADER_COMMAND Vulkan::glslc)
    list(APPEND SHADER_COMMAND "${SHADER_SOURCE}")
    list(APPEND SHADER_COMMAND "-o")
    set(OUTPUT_FILENAME "${OUTPUT_DIR}/${SHADER_FILENAME}.spv")
    list(APPEND SHADER_COMMAND "${OUTPUT_FILENAME}")
    add_custom_command(
        OUTPUT ${OUTPUT_FILENAME}
        COMMAND Vulkan::glslc "${SHADER_SOURCE}" -o "${OUTPUT_FILENAME}"
        DEPENDS ${SHADER_SOURCE}
    )
    add_custom_target(
        ${SHADER_NAME} ALL
        DEPENDS ${OUTPUT_FILENAME}
    )
    set_target_properties(${SHADER_NAME} PROPERTIES OUTPUT_NAME ${OUTPUT_FILENAME})
    list(APPEND SHADER_TARGETS ${SHADER_NAME})

  endforeach()
  set(compile_shaders_RETURN ${SHADER_TARGETS} PARENT_SCOPE)
endfunction()

function(generate_obj_assembly INPUT_FILE_BASE INPUT_FILE_NAME)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(SEGMENT_PREFIX "_")
        set(START_STRING ".section __DATA, __const")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(START_STRING ".section .note.GNU-stack, \"\", %progbits")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set(START_STRING ".section .rdata")
    else()
        set(START_STRING ".section .rodata, \"a\", %progbits")
    endif()
    set(generate_obj_assembly_RETURN "\
${START_STRING}\n\
\t.global ${SEGMENT_PREFIX}data_start_${INPUT_FILE_BASE}\n\
\t.global ${SEGMENT_PREFIX}data_end_${INPUT_FILE_BASE}\n\
${SEGMENT_PREFIX}data_start_${INPUT_FILE_BASE}:\n\
\t.incbin \"${INPUT_FILE_NAME}\"\n\
${SEGMENT_PREFIX}data_end_${INPUT_FILE_BASE}:\n\
" PARENT_SCOPE)
endfunction()
function(binary_files_to_object_files TARGET_NAME)
  set(INPUT_TARGETS ${ARGN})
  set(PREPARE_TARGETS)
  foreach(SOURCE_TARGET IN LISTS INPUT_TARGETS)
    get_target_property(SOURCE_TARGET_LOCATION ${SOURCE_TARGET} OUTPUT_NAME)
    generate_obj_assembly(${SOURCE_TARGET} ${SOURCE_TARGET_LOCATION})
    set(NEW_TARGET_NAME "${SOURCE_TARGET}-object")
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${NEW_TARGET_NAME}.dir")
    set(ASM_PATH "${OUTPUT_DIR}/${SOURCE_TARGET}.s")
    write_file(${ASM_PATH} ${generate_obj_assembly_RETURN})
    add_library(
        ${NEW_TARGET_NAME}
        OBJECT "${ASM_PATH}"
    )
    SET_SOURCE_FILES_PROPERTIES(
        "${ASM_PATH}"
        PROPERTIES
        OBJECT_DEPENDS
        ${SOURCE_TARGET_LOCATION}
    )
    add_dependencies("${NEW_TARGET_NAME}" "${SOURCE_TARGET}")
    list(APPEND PREPARE_TARGETS "${NEW_TARGET_NAME}")
  endforeach()
  set(binary_files_to_object_files_RETURN ${PREPARE_TARGETS} PARENT_SCOPE)
endfunction()

function (cmake_print_all_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()


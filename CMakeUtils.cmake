function(compile_shaders TARGET_NAME)
  set(SHADER_SOURCE_FILES ${ARGN})
  set(SHADER_COMMANDS)
  set(SHADER_PRODUCTS)
  set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir")

  foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
    cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
    cmake_path(GET SHADER_SOURCE FILENAME SHADER_NAME)
    
    # Build command
    list(APPEND SHADER_COMMAND COMMAND)
    list(APPEND SHADER_COMMAND Vulkan::glslc)
    list(APPEND SHADER_COMMAND "${SHADER_SOURCE}")
    list(APPEND SHADER_COMMAND "-o")
    list(APPEND SHADER_COMMAND "${OUTPUT_DIR}/${SHADER_NAME}.spv")

    # Add product
    list(APPEND SHADER_PRODUCTS "${OUTPUT_DIR}/${SHADER_NAME}.spv")
    list(APPEND SHADER_COMMANDS "${SHADER_COMMAND}")

  endforeach()
  add_custom_command(
    OUTPUT ${SHADER_PRODUCTS}
    ${SHADER_COMMANDS}
    DEPENDS ${SHADER_SOURCE_FILES}
    COMMENT "Compiling Shaders [${TARGET_NAME}]"
  )
  add_custom_target(
      ${TARGET_NAME} ALL
      DEPENDS ${SHADER_PRODUCTS}
    )
  set(compile_shaders_RETURN ${SHADER_PRODUCTS} PARENT_SCOPE)
endfunction()

function(generate_obj_assembly INPUT_FILE_BASE INPUT_FILE_NAME)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(SEGMENT_PREFIX "_")
    endif()
    set(generate_obj_assembly_RETURN "\
#if defined(__linux__) && defined(__ELF__)\n\
.section .note.GNU-stack, \"\", %progbits\n\
#endif\n\
\n\
#if defined(__APPLE__)\n\
.section __DATA, __const\n\
#elif defined(_WIN32)\n\
.section .rdata\n\
#else\n\
.section .rodata, \"a\", %progbits\n\
#endif\n\
\t.global ${SEGMENT_PREFIX}data_start_${INPUT_FILE_BASE}\n\
\t.global ${SEGMENT_PREFIX}data_end_${INPUT_FILE_BASE}\n\
${SEGMENT_PREFIX}data_start_${INPUT_FILE_BASE}:\n\
\t.incbin \"${INPUT_FILE_NAME}\"\n\
${SEGMENT_PREFIX}data_end_${INPUT_FILE_BASE}:\n\
" PARENT_SCOPE)
endfunction()
function(binary_files_to_object_files TARGET_NAME)
  set(INPUT_FILES ${ARGN})
  set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir")
  set(PREPARE_COMMANDS)
  set(PREPARE_PRODUCTS)

  foreach(SOURCE_FILE IN LISTS INPUT_FILES)
    cmake_path(ABSOLUTE_PATH SOURCE_FILE NORMALIZE)
    cmake_path(GET SOURCE_FILE FILENAME BINARY_FILENAME)
    string(REPLACE "." "_" VAR_NAME ${BINARY_FILENAME})
    
    generate_obj_assembly(${VAR_NAME} ${SOURCE_FILE})
    set(ASM_TEMPLATE_PATH "${OUTPUT_DIR}/${BINARY_FILENAME}.template.s")
    set(ASM_PATH "${OUTPUT_DIR}/${BINARY_FILENAME}.s")
    set(OBJ_PATH "${OUTPUT_DIR}/${BINARY_FILENAME}.o")
    write_file(${ASM_TEMPLATE_PATH} ${generate_obj_assembly_RETURN})
    if (CMAKE_C_COMPILER EQUAL "cl")
        list(APPEND SHADER_COMMAND COMMAND)
        list(APPEND SHADER_COMMAND "cl")
        list(APPEND SHADER_COMMAND "/P")
        list(APPEND SHADER_COMMAND "${ASM_TEMPLATE_PATH}")
        list(APPEND SHADER_COMMAND "/Fi")
        list(APPEND SHADER_COMMAND "${ASM_PATH}")
        list(APPEND SHADER_COMMAND "&&")
        list(APPEND SHADER_COMMAND as)
        list(APPEND SHADER_COMMAND "${ASM_PATH}")
        list(APPEND SHADER_COMMAND "-o")
        list(APPEND SHADER_COMMAND "${OBJ_PATH}")
    else()
    list(APPEND SHADER_COMMAND COMMAND)
    list(APPEND SHADER_COMMAND ${CMAKE_C_COMPILER})
    list(APPEND SHADER_COMMAND "-E")
    list(APPEND SHADER_COMMAND "${ASM_TEMPLATE_PATH}")
    list(APPEND SHADER_COMMAND "-o")
    list(APPEND SHADER_COMMAND "${ASM_PATH}")
    list(APPEND SHADER_COMMAND "&&")
    list(APPEND SHADER_COMMAND as)
    list(APPEND SHADER_COMMAND "${ASM_PATH}")
    list(APPEND SHADER_COMMAND "-o")
    list(APPEND SHADER_COMMAND "${OBJ_PATH}")
    endif()
    # Add product
    list(APPEND PREPARE_PRODUCTS "${OBJ_PATH}")
    list(APPEND PREPARE_COMMANDS "${SHADER_COMMAND}")

  endforeach()
  add_custom_command(
    OUTPUT ${PREPARE_PRODUCTS}
    ${PREPARE_COMMANDS}
    DEPENDS ${INPUT_FILES}
    COMMENT "Transforming binary files into object files [${TARGET_NAME}]"
  )
  add_custom_target(
      ${TARGET_NAME} ALL
      DEPENDS ${PREPARE_PRODUCTS}
    )
  set(binary_files_to_object_files_RETURN ${PREPARE_PRODUCTS} PARENT_SCOPE)
endfunction()

function (cmake_print_all_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()


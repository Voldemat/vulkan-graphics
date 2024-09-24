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

function(binary_files_to_object_files TARGET_NAME)
  set(INPUT_TARGETS ${ARGN})
  set(PREPARE_TARGETS)
  foreach(SOURCE_TARGET IN LISTS INPUT_TARGETS)
    get_target_property(SOURCE_TARGET_LOCATION ${SOURCE_TARGET} OUTPUT_NAME)
    set(INPUT_FILE_BASE ${SOURCE_TARGET})
    set(INPUT_FILE_NAME ${SOURCE_TARGET_LOCATION})
    set(NEW_TARGET_NAME "${SOURCE_TARGET}-object")
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${NEW_TARGET_NAME}.dir")
    set(ASM_PATH "${OUTPUT_DIR}/${SOURCE_TARGET}.S")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/binary_embed.template.S
        ${ASM_PATH}
    )
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


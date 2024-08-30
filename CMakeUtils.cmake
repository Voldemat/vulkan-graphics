function(add_shaders TARGET_NAME)
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
  add_custom_target(${TARGET_NAME} ALL
    ${SHADER_COMMANDS}
    COMMENT "Compiling Shaders [${TARGET_NAME}]"
    SOURCES ${SHADER_SOURCE_FILES}
    BYPRODUCTS ${SHADER_PRODUCTS}
  )
  set(add_shaders_RETURN ${SHADER_PRODUCTS} PARENT_SCOPE)
endfunction()

function(generate_shader_assembly INPUT_FILE_BASE INPUT_FILE_NAME)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(SEGMENT_PREFIX "_")
    endif()
    set(generate_shader_assembly_RETURN "\
    \t.global ${SEGMENT_PREFIX}data_start_${INPUT_FILE_BASE}\n\
    \t.global ${SEGMENT_PREFIX}data_end_${INPUT_FILE_BASE}\n\
    ${SEGMENT_PREFIX}data_start_${INPUT_FILE_BASE}:\n\
    \t.incbin \"${INPUT_FILE_NAME}\"\n\
    ${SEGMENT_PREFIX}data_end_${INPUT_FILE_BASE}:\n\
    " PARENT_SCOPE)
endfunction()
function(prepare_shaders TARGET_NAME)
  set(SPV_SHADER_FILES ${ARGN})
  set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir")
  set(PREPARE_COMMANDS)
  set(PREPARE_PRODUCTS)

  foreach(SOURCE_FILE IN LISTS SPV_SHADER_FILES)
    cmake_path(ABSOLUTE_PATH SOURCE_FILE NORMALIZE)
    cmake_path(GET SOURCE_FILE FILENAME SHADER_NAME)
    string(REPLACE "." "_" SHADER_ASM_NAME ${SHADER_NAME})
    
    generate_shader_assembly(${SHADER_ASM_NAME} ${SOURCE_FILE})
    set(SHADER_ASM_PATH "${OUTPUT_DIR}/${SHADER_NAME}.s")
    set(SHADER_OBJ_PATH "${OUTPUT_DIR}/${SHADER_NAME}.o")
    write_file(${SHADER_ASM_PATH} ${generate_shader_assembly_RETURN})
    list(APPEND SHADER_COMMAND COMMAND)
    list(APPEND SHADER_COMMAND as)
    list(APPEND SHADER_COMMAND "${SHADER_ASM_PATH}")
    list(APPEND SHADER_COMMAND "-o")
    list(APPEND SHADER_COMMAND "${SHADER_OBJ_PATH}")
    # Add product
    list(APPEND PREPARE_PRODUCTS "${SHADER_OBJ_PATH}")
    list(APPEND PREPARE_COMMANDS "${SHADER_COMMAND}")

  endforeach()
  add_custom_target(${TARGET_NAME} ALL
      ${PREPARE_COMMANDS}
    COMMENT "Transforming shaders into object files [${TARGET_NAME}]"
    SOURCES ${SPV_SHADER_FILES}
    BYPRODUCTS ${PREPARE_PRODUCTS}
  )
  set(prepare_shaders_RETURN ${PREPARE_PRODUCTS} PARENT_SCOPE)
endfunction()

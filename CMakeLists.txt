cmake_minimum_required(VERSION 3.28.1)
project(vulcan-graphics VERSION 0.0.1 LANGUAGES ASM C CXX)
mark_as_advanced(CMAKE_MAKE_PROGRAM)
include(GNUInstallDirs)
include(CMakePrintHelpers)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(Vulkan 1.3.250 REQUIRED)
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/easyloggingpp.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/magic_enum.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/libjpeg.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/zlib.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/libpng.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils.cmake")
file(GLOB_RECURSE GRAPHICS_HEADERS src/*.hpp src/*.h)
file(GLOB_RECURSE GRAPHICS_SOURCES src/*.cpp src/*.c)
file(GLOB_RECURSE GLSL_SHADERS src/shaders/*.frag src/shaders/*.vert)
compile_shaders(glsl-shaders ${GLSL_SHADERS})
binary_files_to_object_files(shaders-embedded ${compile_shaders_RETURN})
set(EMBEDDED_SHADERS ${binary_files_to_object_files_RETURN})
file(GLOB_RECURSE ASSETS src/assets/*)
set(ASSETS_TARGETS)
foreach(ASSET_PATH IN LISTS ASSETS)
    cmake_path(GET ASSET_PATH FILENAME ASSET_NAME)
    string(REPLACE "." "_" ASSET_TARGET_NAME ${ASSET_NAME})
    add_custom_target(${ASSET_TARGET_NAME} ALL DEPENDS ${ASSET_PATH})
    set_target_properties(${ASSET_TARGET_NAME} PROPERTIES OUTPUT_NAME ${ASSET_PATH})
    list(APPEND ASSETS_TARGETS ${ASSET_TARGET_NAME})
endforeach()
binary_files_to_object_files(assets-embedded ${ASSETS_TARGETS})
set(EMBEDDED_ASSETS ${binary_files_to_object_files_RETURN})
add_executable(
    main
    ${GRAPHICS_HEADERS}
    ${GRAPHICS_SOURCES}
)
if (WIN32)
    target_compile_options(main PRIVATE -D NOMINMAX=1)
endif()
target_include_directories(
    main
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/
    PRIVATE ${GLFW_INCLUDE_DIRS}
    PRIVATE ${EASYLOGGINGPP_INCLUDE_DIRECTORIES}
    PRIVATE ${Vulkan_INCLUDE_DIRS}
    PRIVATE ${turbojpeg_INCLUDE_DIRS}
    PRIVATE ${png_static_INCLUDE_DIRS}
)
target_link_libraries(
    main
    glfw
    Vulkan::Vulkan
    glm
    easyloggingpp
    magic_enum::magic_enum
    turbojpeg
    png_static
    ${EMBEDDED_SHADERS}
    ${EMBEDDED_ASSETS}
)

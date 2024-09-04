cmake_minimum_required(VERSION 3.28.1)
project(vulcan-graphics VERSION 0.0.1)
mark_as_advanced(CMAKE_MAKE_PROGRAM)
include(GNUInstallDirs)
include(CMakePrintHelpers)
include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(Vulkan REQUIRED)
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/easyloggingpp.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/magic_enum.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vendor/libjpeg.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils.cmake")
file(GLOB_RECURSE GRAPHICS_HEADERS src/*.hpp src/*.h)
file(GLOB_RECURSE GRAPHICS_SOURCES src/*.cpp src/*.c)
file(GLOB_RECURSE GLSL_SHADERS src/shaders/*.frag src/shaders/*.vert)
add_shaders(glsl-shaders ${GLSL_SHADERS})
set(GLSL_COMPILED_SHADERS ${add_shaders_RETURN})
prepare_shaders(glsl-shaders-object-files ${GLSL_COMPILED_SHADERS})
add_executable(
    main
    ${GRAPHICS_HEADERS}
    ${GRAPHICS_SOURCES}
)
target_include_directories(
    main
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/
    PRIVATE ${GLFW_INCLUDE_DIRS}
    PRIVATE ${EASYLOGGINGPP_INCLUDE_DIRECTORIES}
    PRIVATE ${Vulkan_INCLUDE_DIRS}
    PRIVATE ${turbojpeg_INCLUDE_DIRS}
)
target_link_libraries(
    main
    glfw Vulkan::Vulkan glm easyloggingpp magic_enum::magic_enum turbojpeg-static
    ${prepare_shaders_RETURN}
)

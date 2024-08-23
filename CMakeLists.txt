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
file(GLOB_RECURSE GRAPHICS_HEADERS src/*.hpp src/*.h)
file(GLOB_RECURSE GRAPHICS_SOURCES src/*.cpp src/*.c)
add_executable(
    main
    ${GRAPHICS_HEADERS}
    ${GRAPHICS_SOURCES}
)
cmake_print_variables(Vulkan_INCLUDE_DIRS)
target_include_directories(
    main
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/
    PRIVATE ${GLFW_INCLUDE_DIRS}
    PRIVATE ${EASYLOGGINGPP_INCLUDE_DIRECTORIES}
    PRIVATE ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(main glfw Vulkan::Vulkan glm easyloggingpp magic_enum::magic_enum)

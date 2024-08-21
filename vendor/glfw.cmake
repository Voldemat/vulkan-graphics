FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
)

set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
FetchContent_MakeAvailable(glfw)
set(GLFW_INCLUDE_DIRS ${glfw_SOURCE_DIR}/include/)

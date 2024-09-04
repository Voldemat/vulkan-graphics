FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    CMAKE_ARGS
    -DGLFW_BUILD_EXAMPLES=OFF
    -DGLFW_BUILD_TESTS=OFF
    -DGLFW_BUILD_DOCS=OFF
    -DGLFW_INSTALL=OFF
)
FetchContent_MakeAvailable(glfw)
set(GLFW_INCLUDE_DIRS ${glfw_SOURCE_DIR}/include/)

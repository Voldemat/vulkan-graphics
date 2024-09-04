include(FetchContent)
FetchContent_Declare(
    libpng
    GIT_REPOSITORY https://github.com/pnggroup/libpng
    GIT_TAG v1.6.43
    CMAKE_ARGS
    -DPNG_SHARED=OFF
    -DPNG_TESTS=OFF
    -DPNG_TOOLS=OFF
)
FetchContent_MakeAvailable(libpng)
set(PNG_FRAMEWORK OFF CACHE BOOL "")
get_target_property(png_static_INCLUDE_DIRS png_static INCLUDE_DIRECTORIES)

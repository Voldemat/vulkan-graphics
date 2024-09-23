include(FetchContent)
FetchContent_Declare(
    libpng
    GIT_REPOSITORY https://github.com/pnggroup/libpng
    GIT_TAG v1.6.44
)
set(PNG_FRAMEWORK OFF CACHE BOOL "")
set(PNG_SHARED OFF CACHE BOOL "")
set(PNG_TESTS OFF CACHE BOOL "")
set(PNG_TOOLS OFF CACHE BOOL "")
include_directories(
    BEFORE ${ZLIB_INCLUDE_DIRS}
)
set(PNG_ZLIB_VERNUM 0 CACHE BOOL "")
FetchContent_MakeAvailable(libpng)
get_target_property(png_static_INCLUDE_DIRS png_static INCLUDE_DIRECTORIES)

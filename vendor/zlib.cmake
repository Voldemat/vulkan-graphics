include(FetchContent)
FetchContent_Declare(
    ZLIB
    GIT_REPOSITORY https://github.com/madler/zlib
    GIT_TAG v1.3.1
    OVERRIDE_FIND_PACKAGE
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(SKIP_INSTALL_ALL ON CACHE INTERNAL "")
FetchContent_MakeAvailable(ZLIB)
set_target_properties(zlib PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
target_compile_options(zlibstatic PRIVATE -Wno-deprecated-non-prototype -Wno-deprecated-declarations)
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(zlibstatic PROPERTIES OUTPUT_NAME zd)
endif()
FetchContent_GetProperties(ZLIB)
add_library(ZLIB::ZLIB ALIAS zlibstatic)
set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}")

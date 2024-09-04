include(FetchContent)
FetchContent_Declare(
    libjpeg
    GIT_REPOSITORY https://github.com/winlibs/libjpeg
    GIT_TAG libjpeg-turbo-2.1.0
)
FetchContent_MakeAvailable(libjpeg)
set(ENABLE_SHARED OFF CACHE INTERNAL "DISABLE LIBJPEG SHARED LIBRARIES")
set(REQUIRE_SIMD ON CACHE INTERNAL "REQUIRE LIBJPEG SIMD EXTENSION")
set_target_properties(cjpeg-static PROPERTIES EXCLUDE_FROM_ALL 1)
set_target_properties(tjunittest-static PROPERTIES EXCLUDE_FROM_ALL 1)
set_target_properties(djpeg-static PROPERTIES EXCLUDE_FROM_ALL 1)
set_target_properties(tjbench-static PROPERTIES EXCLUDE_FROM_ALL 1)
set_target_properties(jpegtran-static PROPERTIES EXCLUDE_FROM_ALL 1)
set_target_properties(rdjpgcom PROPERTIES EXCLUDE_FROM_ALL 1)
set_target_properties(wrjpgcom PROPERTIES EXCLUDE_FROM_ALL 1)
set_target_properties(md5cmp PROPERTIES EXCLUDE_FROM_ALL 1)
set_target_properties(jpeg-static PROPERTIES EXCLUDE_FROM_ALL 1)
get_target_property(turbojpeg_INCLUDE_DIRS turbojpeg-static INCLUDE_DIRECTORIES)

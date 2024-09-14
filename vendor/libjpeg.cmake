include(ExternalProject)
ExternalProject_Add(
    libjpeg
    GIT_REPOSITORY https://github.com/libjpeg-turbo/libjpeg-turbo
    GIT_TAG 3.0.3
    CMAKE_ARGS -DENABLE_SHARED=OFF
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target turbojpeg-static
)
ExternalProject_Get_Property(libjpeg SOURCE_DIR)
set(turbojpeg_INCLUDE_DIRS ${SOURCE_DIR})
ExternalProject_Get_Property(libjpeg BINARY_DIR)
set(turbojpeg_BUILD_DIR ${BINARY_DIR})
add_library(turbojpeg STATIC IMPORTED)
set_target_properties(
    turbojpeg PROPERTIES IMPORTED_LOCATION ${turbojpeg_BUILD_DIR}/libturbojpeg.a
)

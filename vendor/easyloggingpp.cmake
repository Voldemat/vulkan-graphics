include(FetchContent)
FetchContent_Declare(
    easyloggingpp
    GIT_REPOSITORY https://github.com/abumq/easyloggingpp
    GIT_TAG v9.97.1
    CMAKE_ARGS
    -Dbuild_static_lib=ON
)
FetchContent_MakeAvailable(easyloggingpp)
set(EASYLOGGINGPP_INCLUDE_DIRECTORIES ${easyloggingpp_SOURCE_DIR}/src/)

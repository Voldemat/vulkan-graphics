include(FetchContent)
FetchContent_Declare(
    easyloggingpp
    GIT_REPOSITORY https://github.com/abumq/easyloggingpp
    GIT_TAG v9.97.1
)
set(build_static_lib ON CACHE INTERNAL "Build easyloggingpp static library")
FetchContent_MakeAvailable(easyloggingpp)
set(EASYLOGGINGPP_INCLUDE_DIRECTORIES ${easyloggingpp_SOURCE_DIR}/src/)

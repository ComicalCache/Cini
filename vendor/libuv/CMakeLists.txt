include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(LIBUV_BUILD_TESTS OFF CACHE BOOL "Build libuv tests" FORCE)

FetchContent_Declare(
        libuv
        GIT_REPOSITORY https://github.com/libuv/libuv.git
        GIT_TAG v1.51.0
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(libuv)

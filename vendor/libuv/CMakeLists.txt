include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(LIBUV_BUILD_TESTS OFF CACHE BOOL "Build libuv tests" FORCE)

FetchContent_Declare(
  libuv
  GIT_REPOSITORY https://github.com/libuv/libuv.git
  GIT_TAG b556c0629d0fd250b5feefbc91b84c6f5e3dae48
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(libuv)

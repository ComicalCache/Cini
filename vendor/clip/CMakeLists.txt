include(FetchContent)

set(CLIP_ENABLE_IMAGE OFF CACHE BOOL "Compile with support to copy/paste images" FORCE)
set(CLIP_ENABLE_LIST_FORMATS OFF CACHE BOOL "Compile with support to list clipboard formats" FORCE)
set(CLIP_EXAMPLES OFF CACHE BOOL "Compile clip examples" FORCE)
set(CLIP_TESTS OFF CACHE BOOL "Compile clip tests" FORCE)
set(CLIP_INSTALL OFF CACHE BOOL "Enable clip installation" FORCE)
set(CLIP_X11_WITH_PNG OFF CACHE BOOL "Compile with libpng to support copy/paste image in png format" FORCE)

FetchContent_Declare(
  clip
  GIT_REPOSITORY https://github.com/dacap/clip.git
  GIT_TAG v1.15
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(clip)

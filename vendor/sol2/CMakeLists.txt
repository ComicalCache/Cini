include(FetchContent)

FetchContent_Declare(
        sol2
        GIT_REPOSITORY https://github.com/ThePhD/sol2.git
        GIT_TAG develop
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(sol2)

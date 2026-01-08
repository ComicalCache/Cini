include(FetchContent)

FetchContent_Declare(
        lua
        URL https://www.lua.org/ftp/lua-5.4.7.tar.gz
)
FetchContent_MakeAvailable(lua)
FetchContent_GetProperties(lua SOURCE_DIR LUA_SOURCE_DIR)

# Glob all sources besides the CLI ones.
file(GLOB LUA_SOURCES "${LUA_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM LUA_SOURCES "${LUA_SOURCE_DIR}/src/lua.c" "${LUA_SOURCE_DIR}/src/luac.c")

add_library(lua_lib STATIC ${LUA_SOURCES})

set_source_files_properties(${LUA_SOURCES} PROPERTIES LANGUAGE CXX)

target_compile_definitions(lua_lib PRIVATE LUA_USE_LINUX)

target_include_directories(lua_lib PUBLIC "${LUA_SOURCE_DIR}/src")

if (UNIX)
    target_link_libraries(lua_lib PRIVATE m dl)
endif ()

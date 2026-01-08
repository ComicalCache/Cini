include(${CMAKE_SOURCE_DIR}/vendor/lua_defaults/lua_defaults.cmake)
include(${CMAKE_SOURCE_DIR}/vendor/lua/lua.cmake)
include(${CMAKE_SOURCE_DIR}/vendor/version/version.cmake)

set(LUA_DEFAULTS_CPP "${CMAKE_BINARY_DIR}/lua_defaults/lua_defaults.cpp")
set_source_files_properties(${LUA_DEFAULTS_CPP} PROPERTIES GENERATED TRUE)
set(VERSION_CPP "${CMAKE_BINARY_DIR}/version/version.cpp")
set_source_files_properties(${VERSION_CPP} PROPERTIES GENERATED TRUE)

add_executable(cini
        bindings/cursor.cpp
        bindings/document.cpp
        bindings/editor.cpp
        bindings/face.cpp
        bindings/key.cpp
        bindings/mode.cpp
        bindings/rgb.cpp
        bindings/viewport.cpp
        bindings/window.cpp
        typedef/key_hash.cpp
        typedef/string_hash.cpp
        util/ansi.cpp
        util/fs.cpp
        util/log.cpp
        util/utf8.cpp
        cell.cpp
        cursor.cpp
        display.cpp
        document.cpp
        editor.cpp
        face.cpp
        key.cpp
        main.cpp
        mini_buffer.cpp
        mode.cpp
        regex.cpp
        viewport.cpp
        window.cpp

        ${LUA_DEFAULTS_CPP}
        ${VERSION_CPP}
)
add_dependencies(cini lua_lib)
add_dependencies(cini lua_defaults)
add_dependencies(cini version)

target_compile_options(cini PRIVATE -Wall -Wextra -Werror)
target_compile_definitions(cini PRIVATE SOL_USING_CXX_LUA=1)
target_compile_definitions(cini PRIVATE SOL_ALL_SAFETIES_ON=1)

target_link_libraries(cini PRIVATE lua_lib sol2 uv_a pcre2-8)

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT OUT)
if (IPO_SUPPORTED AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(cini PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_target_properties(uv_a PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_target_properties(lua_lib PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif (NOT IPO_SUPPORTED)
    message(WARNING "LTO is not supported: ${OUT}")
endif ()

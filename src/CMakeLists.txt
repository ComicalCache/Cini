include(${CMAKE_SOURCE_DIR}/vendor/lua_defaults/lua_defaults.cmake)
include(${CMAKE_SOURCE_DIR}/vendor/luajit/luajit.cmake)
include(${CMAKE_SOURCE_DIR}/vendor/version/version.cmake)

set(LUA_DEFAULTS_CPP "${CMAKE_BINARY_DIR}/lua_defaults/lua_defaults.cpp")
set_source_files_properties(${LUA_DEFAULTS_CPP} PROPERTIES GENERATED TRUE)
set(VERSION_CPP "${CMAKE_BINARY_DIR}/version/version.cpp")
set_source_files_properties(${VERSION_CPP} PROPERTIES GENERATED TRUE)

add_executable(cini
        ansi.cpp
        cell.cpp
        cursor.cpp
        display.cpp
        document.cpp
        editor.cpp
        face.cpp
        key.cpp
        key_hash.cpp
        main.cpp
        mini_buffer.cpp
        mode.cpp
        regex.cpp
        rgb.cpp
        string_hash.cpp
        util.cpp
        viewport.cpp
        window.cpp

        ${LUA_DEFAULTS_CPP}
        ${VERSION_CPP}
)
add_dependencies(cini luajit_script)
add_dependencies(cini lua_defaults)
add_dependencies(cini version)

target_compile_options(cini PRIVATE -Wall -Wextra -Werror -march=native)
target_compile_definitions(cini PRIVATE SOL_LUAJIT=1)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(cini PRIVATE SOL_ALL_SAFETIES_ON=1)
    target_compile_options(cini PRIVATE -ggdb3 -O0)
endif ()

target_include_directories(cini PRIVATE ${LUAJIT_INC})
target_link_libraries(cini PRIVATE ${LUAJIT_LIB} sol2 uv_a pcre2-8)

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT OUT)
if (IPO_SUPPORTED AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(cini PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_target_properties(uv_a PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif (NOT IPO_SUPPORTED)
    message(WARNING "LTO is not supported: ${OUT}")
endif ()

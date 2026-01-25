include(${CMAKE_SOURCE_DIR}/vendor/lua_defaults/lua_defaults.cmake)
include(${CMAKE_SOURCE_DIR}/vendor/lua/lua.cmake)
include(${CMAKE_SOURCE_DIR}/vendor/version/version.cmake)

set(LUA_DEFAULTS_CPP "${CMAKE_BINARY_DIR}/lua_defaults/lua_defaults.cpp")
set_source_files_properties(${LUA_DEFAULTS_CPP} PROPERTIES GENERATED TRUE)
set(VERSION_CPP "${CMAKE_BINARY_DIR}/version/version.cpp")
set_source_files_properties(${VERSION_CPP} PROPERTIES GENERATED TRUE)

add_executable(cini
        bindings/cursor.cpp
        bindings/direction.cpp
        bindings/document.cpp
        bindings/editor.cpp
        bindings/face.cpp
        bindings/key.cpp
        bindings/regex.cpp
        bindings/regex_match.cpp
        bindings/rgb.cpp
        bindings/viewport.cpp

        rendering/cell.cpp
        rendering/display.cpp
        rendering/window.cpp
        rendering/window_manager.cpp

        types/face.cpp
        types/property.cpp
        types/rgb.cpp

        util/ansi.cpp
        util/fs.cpp
        util/utf8.cpp

        cursor.cpp
        document.cpp
        editor.cpp
        face_cache.cpp
        key.cpp
        main.cpp
        mini_buffer.cpp
        property_map.cpp
        regex.cpp
        viewport.cpp

        ${LUA_DEFAULTS_CPP}
        ${VERSION_CPP}
)
add_dependencies(cini lua_lib)
add_dependencies(cini lua_defaults)
add_dependencies(cini version)

target_compile_options(cini PRIVATE -Wall -Wextra -Werror)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(cini PRIVATE -fsanitize=address,undefined)
    target_link_options(cini PRIVATE -fsanitize=address,undefined)
endif ()

# Treat dependency includes as system for clang-tidy to ignore them.
foreach (TARGET lua_lib sol2 uv_a pcre2-8)
    if (TARGET ${TARGET})
        get_target_property(INCLUDES ${TARGET} INTERFACE_INCLUDE_DIRECTORIES)

        if (INCLUDES)
            target_include_directories(cini SYSTEM PRIVATE ${INCLUDES})
        endif ()
    endif ()
endforeach ()

target_link_libraries(cini PRIVATE lua_lib sol2 uv_a pcre2-8)

target_compile_definitions(cini PRIVATE SOL_USING_CXX_LUA=1)
target_compile_definitions(cini PRIVATE SOL_ALL_SAFETIES_ON=1)

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT OUT)
if (IPO_SUPPORTED AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(cini PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_target_properties(uv_a PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_target_properties(lua_lib PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif (NOT IPO_SUPPORTED)
    message(WARNING "LTO is not supported: ${OUT}")
endif ()

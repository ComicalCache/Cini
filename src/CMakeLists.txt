include(${CMAKE_SOURCE_DIR}/vendor/lua_defaults/lua_defaults.cmake)
include(${CMAKE_SOURCE_DIR}/vendor/luajit/luajit.cmake)

add_executable(cini
        ansi.cpp
        cell.cpp
        cursor.cpp
        display.cpp
        document.cpp
        editor.cpp
        key.cpp
        main.cpp
        rgb.cpp
        string_hash.cpp
        util.cpp
        viewport.cpp
)
add_dependencies(cini luajit_script)
add_dependencies(cini lua_defaults)

target_compile_options(cini PRIVATE -Wall -Wextra -Werror -march=native)
target_compile_definitions(cini PRIVATE SOL_LUAJIT=1)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(cini PRIVATE SOL_ALL_SAFETIES_ON=1)
endif ()

target_include_directories(cini PRIVATE ${LUAJIT_INC})
target_link_libraries(cini PRIVATE ${LUAJIT_LIB} sol2 uv_a)

target_include_directories(cini PRIVATE "${CMAKE_BINARY_DIR}/lua_defaults")

include(CheckIPOSupported)
check_ipo_supported(RESULT RES OUTPUT OUT)
if (RES)
    set_target_properties(cini PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_target_properties(uv_a PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
else ()
    message(WARNING "LTO is not supported: ${OUT}")
endif ()

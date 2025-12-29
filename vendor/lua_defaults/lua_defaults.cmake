set(LUA_DIR "${CMAKE_SOURCE_DIR}/lua")
set(EMBED_HEADER "${CMAKE_BINARY_DIR}/lua_defaults/lua_defaults.hpp")

add_custom_target(lua_defaults
        COMMAND ${CMAKE_COMMAND}
        -P "${CMAKE_SOURCE_DIR}/vendor/lua_defaults/generate.cmake"
        "${LUA_DIR}"
        "${EMBED_HEADER}"
        BYPRODUCTS "${EMBED_HEADER}"
        COMMENT "Packing Lua modules into C++ header..."
        VERBATIM
)

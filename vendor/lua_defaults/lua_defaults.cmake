set(LUA_DEFAULTS_SOURCE "${CMAKE_BINARY_DIR}/lua_defaults/lua_defaults.cpp")

add_custom_target(lua_defaults
        COMMAND ${CMAKE_COMMAND}
        -DINPUT_DIR="${CMAKE_SOURCE_DIR}/lua"
        -DLUA_DEFAULTS_SOURCE="${LUA_DEFAULTS_SOURCE}"
        -DLUA_DEFAULTS_TEMPLATE="${CMAKE_CURRENT_SOURCE_DIR}/lua_defaults.cpp.in"
        -DSRC="${CMAKE_CURRENT_SOURCE_DIR}"
        -P "${CMAKE_SOURCE_DIR}/vendor/lua_defaults/generate.cmake"
        BYPRODUCTS "${LUA_DEFAULTS_SOURCE}"
        COMMENT "Embedding Lua modules..."
)

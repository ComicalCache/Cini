set(LUA_SRC "${CMAKE_SOURCE_DIR}/lua/defaults.lua")
set(EMBED_HEADER "${CMAKE_BINARY_DIR}/lua_defaults/lua_defaults.hpp")

add_custom_command(
        OUTPUT ${EMBED_HEADER}
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/vendor/lua_defaults/generate.cmake" "${LUA_SRC}" "${EMBED_HEADER}"
        DEPENDS ${LUA_SRC} "${CMAKE_SOURCE_DIR}/vendor/lua_defaults/lua_defaults.cmake"
        COMMENT "Embedding defaults.lua into a C++ header"
)
add_custom_target(lua_defaults DEPENDS ${EMBED_HEADER})

#ifndef UTF8_BINDING_HPP_
#define UTF8_BINDING_HPP_

#include "sol/forward.hpp"

struct Utf8Binding {
public:
    /// Sets up the bridge to make this space's members and methods available in Lua.
    static void init_bridge(sol::table& core);
};

#endif

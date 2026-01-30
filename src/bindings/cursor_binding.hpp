#ifndef CURSOR_BINDING_HPP_
#define CURSOR_BINDING_HPP_

#include <sol/forward.hpp>

struct CursorBinding {
public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);
};

#endif

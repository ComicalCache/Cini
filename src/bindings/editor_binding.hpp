#ifndef EDITOR_BINDING_HPP_
#define EDITOR_BINDING_HPP_

#include <sol/forward.hpp>

struct EditorBinding {
public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);
};

#endif

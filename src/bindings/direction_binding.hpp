#ifndef DIRECTION_BINDING_HPP_
#define DIRECTION_BINDING_HPP_

#include <sol/forward.hpp>

struct DirectionBinding {
public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);
};

#endif

#ifndef DIRECTION_HPP_
#define DIRECTION_HPP_

#include <sol/sol.hpp>

/// A general direction enum.
enum struct Direction { LEFT, RIGHT, UP, DOWN };

namespace direction {
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    void init_bridge(sol::table& core);
} // namespace direction

#endif

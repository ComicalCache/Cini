#ifndef UTF8_BINDING_HPP_
#define UTF8_BINDING_HPP_

#include "sol/forward.hpp"

namespace utf8_binding {
    /// Sets up the bridge to make this space's members and methods available in Lua.
    void init_bridge(sol::table& core);
} // namespace utf8_binding

#endif

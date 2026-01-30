#ifndef REGEX_BINDING_HPP_
#define REGEX_BINDING_HPP_

#include <sol/forward.hpp>

struct RegexBinding {
public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);
};

#endif

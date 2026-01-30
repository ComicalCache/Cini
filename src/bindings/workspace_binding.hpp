#ifndef WORKSPACE_BINDING_HPP_
#define WORKSPACE_BINDING_HPP_

#include <sol/forward.hpp>

struct WorkspaceBinding {
public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);
};

#endif

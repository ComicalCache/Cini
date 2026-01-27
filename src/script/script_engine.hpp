#ifndef SCRIPT_ENGINE_HPP_
#define SCRIPT_ENGINE_HPP_

#include <memory>

#include <sol/protected_function.hpp>
#include <sol/state.hpp>

#include "../util/assert.hpp"

/// Manages the Lua runtime.
struct ScriptEngine {
public:
    /// Handle to Lua.
    std::unique_ptr<sol::state> lua_;

public:
    ScriptEngine();

    /// Initializes the Lua runtime.
    void init() const;
    /// Sets up the bridge to make structs and functions available in Lua.
    void init_bridge() const;

    /// Emits an event triggering Lua hooks listening for it.
    template<typename... Args>
    void emit_event(const std::string_view event, Args&&... args) {
        sol::protected_function run = (*this->lua_)["Core"]["Hooks"]["run"];
        ASSERT(run.valid(), "");

        const sol::protected_function_result result = run(event, std::forward<Args>(args)...);
        if (!result.valid()) { ScriptEngine::on_emit_event_error(event, result); }
    }

private:
    static void on_emit_event_error(std::string_view event, const sol::error& error);
};

#endif

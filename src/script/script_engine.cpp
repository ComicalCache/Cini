#include "script_engine.hpp"

#include "../cursor.hpp"
#include "../document.hpp"
#include "../editor.hpp"
#include "../gen/lua_defaults.hpp"
#include "../key.hpp"
#include "../regex.hpp"
#include "../types/direction.hpp"
#include "../types/face.hpp"
#include "../viewport.hpp"

ScriptEngine::ScriptEngine() : lua_{std::make_unique<sol::state>()} {}

void ScriptEngine::init() const {
    this->lua_->open_libraries(
        sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::os, sol::lib::io, sol::lib::table,
        sol::lib::debug);

    // Handle Lua panic.
    this->lua_->set_panic([](lua_State* L) -> int {
        std::string s{};

        ansi::main_screen(s);
        std::print("{}", s);
        std::fflush(stdout);
        std::cerr << lua_tostring(L, -1) << "\n";

        uv_tty_reset_mode();
        exit(1);
    });
    // Generate trace on Lua errors.
    this->lua_->set_function("__panic", [](const sol::this_state L, const std::string& msg) -> std::string {
        return sol::state_view{L}["debug"]["traceback"](msg, 2);
    });
    sol::protected_function::set_default_handler((*this->lua_)["__panic"]);

    // Add a loader for predefined defaults.
    sol::table loaders = (*this->lua_)["package"]["searchers"];
    loaders.add([](const sol::this_state L, const std::string& name) -> sol::optional<sol::function> {
        const auto it = lua_modules::files.find(name);
        if (it == lua_modules::files.end()) { return sol::nullopt; }

        const sol::load_result res = sol::state_view{L}.load(it->second, name);
        if (!res.valid()) { return sol::nullopt; }

        return res.get<sol::function>();
    });
}

void ScriptEngine::init_bridge() const {
    auto core = this->lua_->create_named_table("Core");

    Cursor::init_bridge(core);
    direction::init_bridge(core);
    Document::init_bridge(core);
    Editor::init_bridge(core);
    Face::init_bridge(core);
    Key::init_bridge(core);
    Regex::init_bridge(core);
    RegexMatch::init_bridge(core);
    Rgb::init_bridge(core);
    Viewport::init_bridge(core);
}

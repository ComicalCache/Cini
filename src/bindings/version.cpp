#include "bindings.hpp"

#include <sol/state.hpp>

#include "../gen/version.hpp"

void VersionBinding::init_bridge(sol::state& lua) {
    // clang-format off
    lua.new_usertype<VersionBinding>("Version",
        "name", sol::property([](const VersionBinding&) -> std::string_view { return version::NAME; }),
        "version", sol::property([](const VersionBinding&) -> std::string_view { return version::VERSION; }),
        "build_date", sol::property([](const VersionBinding&) -> std::string_view { return version::BUILD_DATE; }),
        "build_type", sol::property([](const VersionBinding&) -> std::string_view { return version::BUILD_TYPE; }));
    lua.set("Version", VersionBinding{});
    // clang-format on
}

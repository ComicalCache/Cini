#ifndef LUA_DEFAULTS_HPP_
#define LUA_DEFAULTS_HPP_

#include <map>
#include <string>
#include <string_view>

namespace lua_modules {
    /// Contains a map from lua module to content of the embedded default configuration.
    extern const std::map<std::string_view, std::string> files;
} // namespace lua_modules

#endif

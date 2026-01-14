#ifndef REGEX_MATCH_HPP_
#define REGEX_MATCH_HPP_

#include <cstddef>

#include <sol/sol.hpp>

/// Regex match range.
struct RegexMatch {
public:
    std::size_t start_;
    std::size_t end_;

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);
};

#endif

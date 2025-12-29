#ifndef RGB_HPP_
#define RGB_HPP_

#include <cstdint>

#include <sol/sol.hpp>

/// RGB colors.
struct Rgb {
public:
    uint8_t r_{0}, g_{0}, b_{0};

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    bool operator==(const Rgb& rhs) const;
    bool operator!=(const Rgb& rhs) const;
};

#endif

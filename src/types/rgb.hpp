#ifndef RGB_HPP_
#define RGB_HPP_

#include <sol/forward.hpp>

/// RGB colors.
struct Rgb {
public:
    uint8_t r_{0};
    uint8_t g_{0};
    uint8_t b_{0};

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    auto operator==(const Rgb& rhs) const -> bool;
    auto operator!=(const Rgb& rhs) const -> bool;
};

#endif

#ifndef RGB_HPP_
#define RGB_HPP_

#include <cstdint>

/// RGB colors.
struct Rgb {
public:
    uint8_t r_{0};
    uint8_t g_{0};
    uint8_t b_{0};

public:
    auto operator==(const Rgb& rhs) const -> bool;
    auto operator!=(const Rgb& rhs) const -> bool;
};

#endif

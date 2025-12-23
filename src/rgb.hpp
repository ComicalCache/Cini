#ifndef RGB_HPP_
#define RGB_HPP_

#include <cstdint>

/// RGB colors.
struct Rgb {
public:
    uint8_t r_{0}, g_{0}, b_{0};

public:
    bool operator==(const Rgb& rhs) const;
    bool operator!=(const Rgb& rhs) const;
};

#endif

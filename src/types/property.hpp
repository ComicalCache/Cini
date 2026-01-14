#ifndef TEXT_PROPERTY_HPP_
#define TEXT_PROPERTY_HPP_

#include <cstddef>
#include <string>

#include <sol/sol.hpp>

/// Lua properties tied to a text range.
struct Property {
public:
    std::size_t start_;
    std::size_t end_;

    std::string key_;
    sol::object value_;

public:
    [[nodiscard]]
    bool contains(std::size_t pos) const;
    [[nodiscard]]
    bool overlaps(std::size_t start, std::size_t end) const;
};

#endif

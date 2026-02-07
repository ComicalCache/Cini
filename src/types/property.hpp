#ifndef TEXT_PROPERTY_HPP_
#define TEXT_PROPERTY_HPP_

#include <cstddef>
#include <string>

#include <sol/object.hpp>

/// Properties are a key-value metadata storage attached to a text range in a Document. The bounds are interpreted as
/// byte indices.
///
/// A property must not start and end on the same byte. Failure to do so will result in Property::contains always
/// being false.
struct Property {
public:
    std::size_t start_;
    std::size_t end_;

    std::string key_;
    sol::object value_;

public:
    [[nodiscard]]
    auto contains(std::size_t pos) const -> bool;
    [[nodiscard]]
    auto overlaps(std::size_t start, std::size_t end) const -> bool;
};

#endif

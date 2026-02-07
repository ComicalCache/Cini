#ifndef TEXT_PROPERTY_MAP_HPP_
#define TEXT_PROPERTY_MAP_HPP_

#include <vector>

#include <sol/forward.hpp>

#include "../types/property.hpp"

/// The PropertyMap manages efficient storage and access to Properties.
///
/// Text Properties on Documents must be managed through the API of this class and never directly inserted. Failure to
/// do so can result in UB and possible slowdowns.
class PropertyMap {
public:
    std::vector<Property> properties_{};

public:
    /// Adds or updates a property on a text range.
    void add(std::size_t start, std::size_t end, std::string key, sol::object value);
    /// Removes all matching properties in the given range.
    void remove(std::size_t start, std::size_t end, std::string_view key);
    /// Removes all or matching properties.
    void clear(sol::optional<std::string_view> key);

    /// Gets the matching property at that position.
    [[nodiscard]]
    auto get_property(std::size_t pos, std::string_view key) const -> sol::object;
    /// Gets all properties at that position.
    [[nodiscard]]
    auto get_all_properties(std::size_t pos, sol::state& lua) const -> sol::table;
    /// Gets the matching raw Property object at that position.
    [[nodiscard]]
    auto get_raw_property(std::size_t pos, std::string_view key) const -> const Property*;

    /// Updates property ranges after insertion.
    void update_on_insert(std::size_t pos, std::size_t len);
    /// Updates property ranges after removal.
    void update_on_remove(std::size_t start, std::size_t end);
    /// Merges overlapping properties.
    void merge(std::string_view key);

    /// Count of properties set.
    [[nodiscard]]
    auto size() const -> std::size_t;
    [[nodiscard]]
    auto empty() const -> bool;
};

#endif

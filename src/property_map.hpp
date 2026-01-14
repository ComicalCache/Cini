#ifndef TEXT_PROPERTY_MAP_HPP_
#define TEXT_PROPERTY_MAP_HPP_

#include <vector>

#include "types/property.hpp"

/// Container for managing Lua properties on text ranges.
class PropertyMap {
private:
    std::vector<Property> properties_{};

public:
    /// Adds or updates a property on a text range.
    void add(std::size_t start, std::size_t end, std::string_view key, const sol::object& value);
    /// Removes all matching properties in the given range.
    void remove(std::size_t start, std::size_t end, std::string_view key);
    /// Removes all or matching properties.
    void clear(sol::optional<std::string_view> key);

    /// Gets the matching property at that position.
    [[nodiscard]]
    sol::object get_property(std::size_t pos, std::string_view key) const;
    /// Gets all properties at that position.
    [[nodiscard]]
    sol::table get_all_properties(std::size_t pos, lua_State* L) const;
    /// Gets the matching raw Property object at that position.
    [[nodiscard]]
    const Property* get_raw_property(std::size_t pos, std::string_view key) const;

    /// Updates property ranges after insertion.
    void update_on_insert(std::size_t pos, std::size_t len);
    /// Updates property ranges after removal.
    void update_on_remove(std::size_t start, std::size_t end);
    /// Merges overlapping properties.
    void merge(std::string_view key);

    /// Count of properties set.
    [[nodiscard]]
    std::size_t size() const {
        return this->properties_.size();
    }

    [[nodiscard]]
    bool empty() const {
        return this->properties_.empty();
    }

private:
    /// Finds the insertion point for a property with the given start position.
    [[nodiscard]]
    std::vector<Property>::iterator find_insertion_point(std::size_t start);
};

#endif

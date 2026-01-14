#include "property_map.hpp"

#include <algorithm>
#include <ranges>

#include "util/assert.hpp"

void PropertyMap::add(
    const std::size_t start, const std::size_t end, const std::string_view key, const sol::object& value) {
    ASSERT(start < end, "Start of property must be before end.");

    // Remove previously existing properties with the same key to replace them.
    this->remove(start, end, key);

    Property prop{start, end, std::string(key), value};
    this->properties_.insert(this->find_insertion_point(start), std::move(prop));
}

void PropertyMap::remove(const std::size_t start, const std::size_t end, const std::string_view key) {
    ASSERT(start < end, "Start of property must be before end.");

    auto property = this->properties_.begin();
    while (property != this->properties_.end()) {
        if (property->key_ == key && property->overlaps(start, end)) {
            property = this->properties_.erase(property);
        } else {
            ++property;
        }
    }
}

void PropertyMap::clear(sol::optional<std::string_view> key) {
    if (key) {
        auto property = this->properties_.begin();
        while (property != this->properties_.end()) {
            if (property->key_ == *key) {
                property = this->properties_.erase(property);
            } else {
                ++property;
            }
        }
    } else {
        this->properties_.clear();
    }
}

sol::object PropertyMap::get_property(const std::size_t pos, const std::string_view key) const {
    for (const auto& property: std::ranges::reverse_view(this->properties_)) {
        if (property.key_ == key && property.contains(pos)) { return property.value_; }
    }

    return sol::lua_nil;
}

sol::table PropertyMap::get_all_properties(const std::size_t pos, lua_State* L) const {
    sol::state_view lua{L};
    sol::table res = lua.create_table();

    for (const auto& property: this->properties_) {
        if (property.contains(pos)) { res[property.key_] = property.value_; }
    }

    return res;
}

const Property* PropertyMap::get_raw_property(const std::size_t pos, const std::string_view key) const {
    for (const auto& property: std::ranges::reverse_view(this->properties_)) {
        if (property.key_ == key && property.contains(pos)) { return &property; }
    }

    return nullptr;
}

void PropertyMap::update_on_insert(const std::size_t pos, const std::size_t len) {
    for (auto& property: this->properties_) {
        if (property.start_ >= pos) {
            property.start_ += len;
            property.end_ += len;
        } else if (property.end_ > pos) {
            property.end_ += len;
        }
    }
}

void PropertyMap::update_on_remove(const std::size_t start, const std::size_t end) {
    const std::size_t len = end - start;

    auto property = this->properties_.begin();
    while (property != this->properties_.end()) {
        if (property->end_ <= start) {
            ++property;
            continue;
        }

        if (property->start_ >= end) {
            property->start_ -= len;
            property->end_ -= len;
            ++property;
            continue;
        }

        // Property overlaps entirely.
        if (property->start_ >= start && property->end_ <= end) {
            property = this->properties_.erase(property);
            continue;
        }

        // Property contains removal.
        if (property->start_ < start && property->end_ > end) {
            property->end_ -= len;
        }
        // Property ends in removal.
        else if (property->start_ < start) {
            property->end_ = start;
        }
        // Property starts in removal.
        else {
            property->start_ = start;
            property->end_ -= len;
        }

        ++property;
    }
}

void PropertyMap::merge(const std::string_view key) {
    if (this->properties_.empty()) { return; }

    // Read head.
    auto read = this->properties_.begin();
    // Write head.
    auto write = this->properties_.begin();

    while (read != this->properties_.end()) {
        if (read->key_ != key) {
            // Sync heads and increment.
            *write++ = *read++;
            continue;
        }

        // Merge properties, desyncs heads.
        *write = *read++;
        while (read != this->properties_.end() && read->key_ == key && read->start_ <= write->end_) {
            // TODO: check if values are identical.
            write->end_ = std::max(write->end_, read->end_);
            ++read;
        }

        ++write;
    }

    this->properties_.erase(write, this->properties_.end());
}

std::vector<Property>::iterator PropertyMap::find_insertion_point(const std::size_t start) {
    return std::lower_bound(
        this->properties_.begin(), this->properties_.end(), start,
        [](const Property& prop, const std::size_t start) { return prop.start_ < start; });
}

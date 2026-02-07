#include "property_map.hpp"

#include <algorithm>
#include <ranges>

#include <sol/state.hpp>
#include <sol/table.hpp>

#include "../types/property.hpp"
#include "../util/assert.hpp"

void PropertyMap::add(const std::size_t start, const std::size_t end, std::string key, sol::object value) {
    ASSERT(start < end, "");

    // Remove previously existing properties with the same key to replace them.
    this->remove(start, end, key);

    Property prop{.start_ = start, .end_ = end, .key_ = std::move(key), .value_ = std::move(value)};
    this->properties_.insert(
        std::ranges::lower_bound(this->properties_, start, {}, &Property::start_), std::move(prop));
}

void PropertyMap::remove(const std::size_t start, const std::size_t end, const std::string_view key) {
    ASSERT(start < end, "");

    std::optional<Property> right{};

    // Read head.
    auto read = this->properties_.begin();
    // Write head.
    auto write = this->properties_.begin();

    while (read != this->properties_.end()) {
        if (read->key_ != key || !read->overlaps(start, end)) {
            // Sync heads and increment.
            if (read != write) { *write = std::move(*read); }
            ++write;
            ++read;

            continue;
        }

        auto keep{true};

        if (read->start_ >= start && read->end_ <= end) {
            keep = false;
        } else if (read->start_ < start && read->end_ > end) {
            Property tmp = *read;
            tmp.start_ = end;
            right = std::move(tmp);

            // Truncate this to be the left side split.
            read->end_ = start;
            keep = true;
        } else if (read->start_ < start) {
            read->end_ = start;
            keep = true;
        } else {
            read->start_ = end;
            keep = true;
        }

        if (keep) {
            if (read != write) { *write = std::move(*read); }
            ++write;
        }
    }

    this->properties_.erase(write, this->properties_.end());

    if (right) {
        this->properties_.insert(
            std::ranges::upper_bound(this->properties_, right->start_, {}, &Property::start_), std::move(*right));
    }
}

void PropertyMap::clear(sol::optional<std::string_view> key) {
    if (key) {
        std::erase_if(this->properties_, [&](const Property& property) -> bool { return property.key_ == *key; });
    } else {
        this->properties_.clear();
    }
}

auto PropertyMap::get_property(const std::size_t pos, const std::string_view key) const -> sol::object {
    auto end = std::ranges::upper_bound(this->properties_, pos, {}, &Property::start_);
    for (const auto& property: std::ranges::reverse_view(std::ranges::subrange(this->properties_.begin(), end))) {
        if (property.key_ == key) {
            if (property.contains(pos)) { return property.value_; }

            // Since properties for a single key are disjoint, if the closest property doesn't match, pos does not
            // contain the target.
            return sol::lua_nil;
        }
    }

    return sol::lua_nil;
}

auto PropertyMap::get_all_properties(const std::size_t pos, sol::state& lua) const -> sol::table {
    sol::table res = lua.create_table();

    auto end = std::ranges::upper_bound(this->properties_, pos, {}, &Property::start_);
    for (const auto& property: std::ranges::subrange(this->properties_.begin(), end)) {
        if (property.contains(pos)) { res[property.key_] = property.value_; }
    }

    return res;
}

auto PropertyMap::get_raw_property(const std::size_t pos, const std::string_view key) const -> const Property* {
    auto end = std::ranges::upper_bound(this->properties_, pos, {}, &Property::start_);
    for (const auto& property: std::ranges::reverse_view(std::ranges::subrange(this->properties_.begin(), end))) {
        if (property.key_ == key) {
            // FIXME: replace const Property* with std::optional<const Property&> when upgrading to C++26.
            if (property.contains(pos)) { return &property; }

            // Since properties for a single key are disjoint, if the closest property doesn't match, pos does not
            // contain the target.
            return nullptr;
        }
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
    const auto len = end - start;

    // Read head.
    auto read = this->properties_.begin();
    // Write head.
    auto write = this->properties_.begin();

    while (read != this->properties_.end()) {
        if (read->end_ <= start) {
            // Sync heads and increment.
            if (write != read) { *write = std::move(*read); }
            write++;
            read++;

            continue;
        }

        auto keep{true};
        if (read->start_ >= end) { // Entirely after: Shift left.
            read->start_ -= len;
            read->end_ -= len;
        } else if (read->start_ >= start && read->end_ <= end) { // Overlaps entirely: Remove.
            keep = false;
        } else if (read->start_ < start && read->end_ > end) { // Contains removal: Shrink.
            read->end_ -= len;
        } else if (read->start_ < start) { // Ends in removal: Truncate.
            read->end_ = start;
        } else { // Starts in removal: Truncate start.
            read->start_ = start;
            read->end_ -= len;
        }

        if (keep) {
            if (write != read) { *write = std::move(*read); }
            write++;
        }

        read++;
    }

    // Truncate old vector.
    this->properties_.erase(write, this->properties_.end());
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
            if (write != read) { *write = std::move(*read); }
            write++;
            read++;

            continue;
        }

        // Merge properties, desyncs heads.
        *write = std::move(*read);
        read++;
        while (read != this->properties_.end() && read->key_ == key && read->start_ <= write->end_) {
            // TODO: check if values are identical.
            write->end_ = std::max(write->end_, read->end_);
            read++;
        }

        write++;
    }

    this->properties_.erase(write, this->properties_.end());
}

auto PropertyMap::size() const -> std::size_t { return this->properties_.size(); }
auto PropertyMap::empty() const -> bool { return this->properties_.empty(); }

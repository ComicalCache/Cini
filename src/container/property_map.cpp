#include "property_map.hpp"

#include <algorithm>
#include <ranges>

#include <sol/state.hpp>
#include <sol/table.hpp>

#include "../types/property.hpp"
#include "../util/assert.hpp"

void PropertyMap::add(const std::size_t start, const std::size_t end, const std::string& key, sol::object value) {
    ASSERT(start <= end, "");

    // Remove previously existing properties with the same key to replace them.
    this->remove(start, end, key);

    Property prop{.start_ = start, .end_ = end, .key_ = key, .value_ = std::move(value)};
    this->properties_[key].insert(
        std::ranges::lower_bound(this->properties_[key], start, {}, &Property::start_), std::move(prop));
}

void PropertyMap::remove(const std::size_t start, const std::size_t end, const std::string_view key) {
    ASSERT(start <= end, "");

    auto it = this->properties_.find(std::string(key));
    if (it == this->properties_.end() || it->second.empty()) { return; }
    auto& vec = it->second;

    auto read = std::ranges::upper_bound(vec, start, {}, &Property::start_);
    if (read != vec.begin() && std::prev(read)->end_ > start) { read = std::prev(read); }
    auto write = read;
    std::optional<Property> right{};

    while (read != vec.end()) {
        if (!read->overlaps(start, end)) {
            if (read->start_ >= end) { break; }

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

        ++read;
    }

    if (read != write && read != vec.end()) { write = std::move(read, vec.end(), write); }
    vec.erase(write, vec.end());

    if (right) { vec.insert(std::ranges::upper_bound(vec, right->start_, {}, &Property::start_), std::move(*right)); }
}

void PropertyMap::clear(const sol::optional<std::string>& key) {
    if (key) {
        this->properties_.erase(*key);
    } else {
        this->properties_.clear();
    }
}

auto PropertyMap::get_property(const std::size_t pos, const std::string_view key) const -> sol::object {
    auto it = this->properties_.find(std::string(key));
    if (it == this->properties_.end() || it->second.empty()) { return sol::lua_nil; }

    auto end = std::ranges::upper_bound(it->second, pos, {}, &Property::start_);

    if (end != it->second.begin()) {
        const auto& prop = *std::prev(end);
        if (prop.contains(pos)) { return prop.value_; }
    }

    return sol::lua_nil;
}

auto PropertyMap::get_properties(const std::size_t pos, sol::state& lua) const -> sol::table {
    sol::table res = lua.create_table();

    for (const auto& [key, val]: this->properties_) {
        auto end = std::ranges::upper_bound(val, pos, {}, &Property::start_);
        for (const auto& property: std::ranges::subrange(val.begin(), end)) {
            if (property.contains(pos)) { res[key][property.key_] = property.value_; }
        }
    }

    return res;
}

auto PropertyMap::get_all_properties(const std::string_view key, sol::state& lua) const -> sol::table {
    auto res = lua.create_table();

    for (const auto& [pkey, val]: this->properties_) {
        auto idx = 1UZ;
        auto group = lua.create_table();
        for (const auto& prop: val) {
            if (prop.key_ == key) {
                auto item = lua.create_table();
                item["start"] = prop.start_;
                item["stop"] = prop.end_;
                item["value"] = prop.value_;
                group[idx++] = item;
            }
        }

        if (idx != 1) { res[pkey] = group; }
    }

    return res;
}

auto PropertyMap::get_raw_property(const std::size_t pos, const std::string_view key) const -> const Property* {
    auto it = this->properties_.find(std::string(key));
    if (it == this->properties_.end() || it->second.empty()) { return nullptr; }

    auto end = std::ranges::upper_bound(it->second, pos, {}, &Property::start_);

    if (it->second.begin() != end) {
        const auto& prop = *std::prev(end);
        if (prop.contains(pos)) { return &prop; }
    }
    return nullptr;
}

void PropertyMap::update_on_insert(const std::size_t pos, const std::size_t len) {
    for (auto& [_, val]: this->properties_) {
        if (val.empty()) { continue; }

        auto it = std::ranges::upper_bound(val, pos, {}, &Property::start_);
        if (it != val.begin() && std::prev(it)->end_ > pos) { it--; }

        for (; it != val.end(); it++) {
            if (it->start_ >= pos) {
                it->start_ += len;
                it->end_ += len;
            } else if (it->end_ > pos) {
                it->end_ += len;
            }
        }
    }
}

void PropertyMap::update_on_remove(const std::size_t start, const std::size_t end) {
    const auto len = end - start;

    for (auto& [_, val]: this->properties_) {
        if (val.empty()) { continue; }

        auto read = std::ranges::upper_bound(val, start, {}, &Property::start_);
        if (read != val.begin() && std::prev(read)->end_ > start) { read = std::prev(read); }
        auto write = read;

        while (read != val.end()) {
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
        val.erase(write, val.end());
    }
}

void PropertyMap::merge(const std::string_view key) {
    auto it = this->properties_.find(std::string(key));
    if (it == this->properties_.end() || it->second.empty()) { return; }

    auto& val = it->second;
    // Read head.
    auto read = val.begin();
    // Write head.
    auto write = val.begin();

    *write = std::move(*read);
    read++;

    while (read != val.end()) {
        if (read->start_ <= write->end_) {
            // Overlapping or adjacent, merge them
            write->end_ = std::max(write->end_, read->end_);
        } else {
            // Disjoint, advance write head
            write++;
            if (write != read) { *write = std::move(*read); }
        }
        read++;
    }

    write++;
    val.erase(write, val.end());
}

auto PropertyMap::size() const -> std::size_t { return this->properties_.size(); }
auto PropertyMap::empty() const -> bool { return this->properties_.empty(); }

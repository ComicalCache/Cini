#include "string_hash.hpp"

auto StringHash::operator()(const std::string_view sv) const -> std::size_t {
    return std::hash<std::string_view>{}(sv);
}

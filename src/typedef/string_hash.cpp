#include "string_hash.hpp"

std::size_t StringHash::operator()(const std::string_view sv) const { return std::hash<std::string_view>{}(sv); }

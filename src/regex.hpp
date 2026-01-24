#ifndef REGEX_HEADER_HPP_
#define REGEX_HEADER_HPP_

#define PCRE2_CODE_UNIT_WIDTH 8

#include <memory>
#include <string_view>
#include <vector>

#include <pcre2.h>

#include "types/regex_match.hpp"

/// Regex engine wrapper.
struct Regex {
private:
    std::shared_ptr<pcre2_code> code_{nullptr};
    std::shared_ptr<pcre2_match_data> match_data_{nullptr};

public:
    /// Sets up the bridge to make this struct's members and methods available in Lua.
    static void init_bridge(sol::table& core);

    explicit Regex(std::string_view pattern);

    /// Searches a text and returns all matches.
    [[nodiscard]]
    auto search(std::string_view text) const -> std::vector<RegexMatch>;
};

#endif

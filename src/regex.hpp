#ifndef REGEX_HEADER_HPP_
#define REGEX_HEADER_HPP_

#define PCRE2_CODE_UNIT_WIDTH 8

#include <string_view>
#include <vector>

#include <pcre2.h>

#include "types/regex_match.hpp"

/// Regex engine wrapper.
struct Regex {
private:
    pcre2_code* code_{nullptr};
    pcre2_match_data* match_data_{nullptr};

public:
    explicit Regex(std::string_view pattern);
    ~Regex();

    /// Searches a text and returns all matches.
    [[nodiscard]]
    std::vector<RegexMatch> search_all(std::string_view text) const;
};

#endif

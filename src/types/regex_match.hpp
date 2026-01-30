#ifndef REGEX_MATCH_HPP_
#define REGEX_MATCH_HPP_

#include <cstddef>

/// Regex match range.
struct RegexMatch {
public:
    std::size_t start_;
    std::size_t end_;
};

#endif

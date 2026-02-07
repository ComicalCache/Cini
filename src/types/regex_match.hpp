#ifndef REGEX_MATCH_HPP_
#define REGEX_MATCH_HPP_

#include <cstddef>

/// The Regex match specifies the start byte and end byte (exclusive) of a found match during a Regex query.
struct RegexMatch {
public:
    std::size_t start_;
    std::size_t end_;
};

#endif

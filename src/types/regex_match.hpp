#ifndef REGEX_MATCH_HPP_
#define REGEX_MATCH_HPP_

#include <cstddef>
#include <string>
#include <vector>

/// The Regex match specifies the start byte and end byte (exclusive) of a found match during a Regex query.
struct RegexMatch {
public:
    std::size_t start_;
    std::size_t end_;

    std::string match_;
    std::vector<std::string> captures_;
};

#endif

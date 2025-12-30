#ifndef SYNTAX_RULE_HPP_
#define SYNTAX_RULE_HPP_

#include <regex>

/// A rule specifying a regex pattern and the corresponding face.
struct SyntaxRule {
public:
    std::regex pattern;
    std::string face;
};

#endif

#ifndef SYNTAX_RULE_HPP_
#define SYNTAX_RULE_HPP_

#include <memory>
#include <string>

struct Regex;

/// Rule specifying a regex pattern and the corresponding face.
struct SyntaxRule {
public:
    std::shared_ptr<Regex> pattern_;
    std::string face_;
};

#endif

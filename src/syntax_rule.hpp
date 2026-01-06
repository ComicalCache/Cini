#ifndef SYNTAX_RULE_HPP_
#define SYNTAX_RULE_HPP_

struct Regex;

/// Rule specifying a regex pattern and the corresponding face.
struct SyntaxRule {
public:
    std::shared_ptr<Regex> pattern;
    std::string face;
};

#endif

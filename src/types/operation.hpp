#ifndef OPERATION_HPP_
#define OPERATION_HPP_

#include <cstddef>
#include <cstdint>
#include <string>

/// Operations are insertions or removals of text in a Document.
struct Operation {
public:
    enum struct Type : std::uint8_t { INSERT, REMOVE };

public:
    Type type_;
    std::size_t pos_;
    std::string data_;
};

#endif

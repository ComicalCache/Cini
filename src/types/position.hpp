#ifndef POSITION_HPP_
#define POSITION_HPP_

#include <cstddef>

/// A general position struct.
struct Position {
public:
    std::size_t row_{0};
    std::size_t col_{0};
};

#endif

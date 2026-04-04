#ifndef TRANSACTION_HPP_
#define TRANSACTION_HPP_

#include <vector>

#include "operation.hpp"

/// Transactions group Operations on a Document into a block.
struct Transaction {
public:
    std::vector<Operation> operations_{};
    std::size_t point_before_{0UZ};
    std::size_t point_after_{0UZ};
};

#endif

#ifndef VISITOR_HPP_
#define VISITOR_HPP_

/// Helper for the std::visit pattern.
template<typename... Ts>
struct Visitor : Ts... {
    using Ts::operator()...;
};

#endif

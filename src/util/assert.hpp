#ifndef ASSERT_HPP_
#define ASSERT_HPP_

#include <iostream>
#include <print>
#include <source_location>
#include <string>

#include <uv.h>

#include "ansi.hpp"

namespace util {
    inline void _assert(
        const char* condition, const char* message,
        const std::source_location& location = std::source_location::current()) {
        uv_tty_reset_mode();

        std::string s{};
        ansi::main_screen(s);
        std::print("{}", s);

        // clang-format off
        std::cerr << "[Assertion Failed]\n"
            << location.file_name() << "[" << location.line() << ":" << location.column() << "]: " << condition << "\n"
            << "  " << message << "\n";
        // clang-format on

        std::abort();
    }
} // namespace util

// Do while forces a semicolon.
/// Assert a condition in Debug and Release builds.
#define ASSERT(Cond, Msg)                           \
    do {                                            \
        if (!(Cond)) { util::_assert(#Cond, Msg); } \
    } while (0)

#ifdef NDEBUG
  // Forces a semicolon.
    /// Assert a condition in Debug builds.
    #define ASSERT_DEBUG(Cond, Msg) ((void)0)
#else
  // Forces a semicolon.
    /// Assert a condition in Debug builds.
    #define ASSERT_DEBUG(Cond, Msg) ASSERT(Cond, Msg)
#endif

#endif

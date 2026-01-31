#include "utf8_binding.hpp"

#include <sol/table.hpp>

#include "../util/utf8.hpp"

namespace utf8_binding {
    void init_bridge(sol::table& core) {
        // Phantom struct to declare Utf8 utility functions to Lua.
        struct Utf8 {};

        // clang-format off
        core.new_usertype<Utf8>("Utf8",
            "count", [](const std::string_view str) -> std::size_t {
                auto count = 0UL;
                for (auto idx = 0UL; idx < str.size(); ) {
                    idx += utf8::len(static_cast<unsigned char>(str[idx]));
                    count++;
                }
                return count;
            },
            "len", [](const std::string_view ch) -> std::size_t {
                return utf8::len(static_cast<unsigned char>(ch[0]));
            });
        // clang-format on
    }
} // namespace utf8_binding

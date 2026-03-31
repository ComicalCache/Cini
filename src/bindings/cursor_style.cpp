#include "bindings.hpp"

#include <sol/table.hpp>

#include "../util/ansi.hpp"

void CursorStyleBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_enum("CursorStyle",
        "Hidden", ansi::CursorStyle::HIDDEN,
        "BlinkingBlock", ansi::CursorStyle::BLINKING_BLOCK,
        "SteadyBlock", ansi::CursorStyle::STEADY_BLOCK,
        "BlinkingUnderline", ansi::CursorStyle::BLINKING_UNDERLINE,
        "SteadyUnderline", ansi::CursorStyle::STEADY_UNDERLINE,
        "BlinkingBar", ansi::CursorStyle::BLINKING_BAR,
        "SteadyBar", ansi::CursorStyle::STEADY_BAR);
    // clang-format on
}

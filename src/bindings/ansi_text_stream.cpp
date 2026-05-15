#include "bindings.hpp"

#include <sol/state.hpp>

#include "../input/ansi_text_stream.hpp"
// Include required because ansi_text_stream.hpp forward declares Document.
#include "../document.hpp" // IWYU pragma: keep.

void AnsiTextStreamBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<AnsiTextStream>("AnsiTextStream",
        /* Functions. */
        sol::call_constructor, sol::constructors<AnsiTextStream(std::shared_ptr<Document> doc)>(),
        "parse", &AnsiTextStream::parse,
        "flush", &AnsiTextStream::flush);
    // clang-format on
}

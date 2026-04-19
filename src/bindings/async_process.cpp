#include "bindings.hpp"

#include <sol/state.hpp>

#include "../async_process.hpp"
// Include required because async_process.hpp forward declares Document.
#include "../document.hpp" // IWYU pragma: keep.

void AsyncProcessBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<AsyncProcess>("AsyncProcess",
        /* Properties. */
        "command", sol::readonly(&AsyncProcess::command_),
        "args", sol::readonly(&AsyncProcess::args_),
        "doc", sol::readonly(&AsyncProcess::doc_),

        /* Functions. */
        "spawn", &AsyncProcess::spawn,
        "kill", &AsyncProcess::kill);
    // clang-format on
}

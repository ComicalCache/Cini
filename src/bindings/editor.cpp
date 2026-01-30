#include "editor_binding.hpp"

#include <sol/property.hpp>

#include "../document.hpp"
#include "../editor.hpp"
#include "../viewport.hpp"

void EditorBinding::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Editor>("Editor",
        /* Properties. */
        "documents", sol::readonly(&Editor::documents_),
        "workspace", sol::readonly(&Editor::workspace_),

        /* Functions. */
        "create_document", [](Editor& self, std::optional<std::string_view> path) -> std::shared_ptr<Document> {
            return self.create_document(path);
        },
        "destroy_document", &Editor::destroy_document,
        "set_status_message", &Editor::set_status_message);
    // clang-format on
}

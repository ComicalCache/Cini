#include "bindings.hpp"

#include <sol/property.hpp>

#include "../document.hpp"
#include "../document_view.hpp"
#include "../editor.hpp"
#include "../viewport.hpp"

void EditorBinding::init_bridge(sol::state& lua) {
    // clang-format off
    lua.new_usertype<Editor>("Cini",
        /* Properties. */
        "documents", sol::readonly(&Editor::documents_),
        "document_views", sol::readonly(&Editor::document_views_),
        "workspace", sol::readonly(&Editor::workspace_),
        "face_layers", &Editor::face_layers_,

        /* Functions. */
        "quit", [](Editor&) -> void { Editor::stop(); },
        "create_document", [](Editor& self, std::optional<std::string_view> path) -> std::shared_ptr<Document> {
            return self.create_document(path);
        },
        "destroy_document", &Editor::destroy_document,
        "create_document_view", &Editor::create_document_view,
        "destroy_document_view", &Editor::destroy_document_view,
        "set_status_message", &Editor::set_status_message,
        "clear_status_message", [](Editor& self) -> void { self.workspace_.mini_buffer_.clear_status_message(); },
        "debug_stats", [](Editor& self) -> sol::table {
            auto stats = self.lua_.create_table();

            stats["document_instances"] = Document::instances_.load();
            stats["document_view_instances"] = DocumentView::instances_.load();
            stats["viewport_instances"] = Viewport::instances_.load();

            stats["documents"] = self.documents_.size();
            stats["document_views"] = self.document_views_.size();

            return stats;
        });
    // clang-format on
}

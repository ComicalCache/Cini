#include "bindings.hpp"

#include <sol/property.hpp>

#include "../async_process.hpp"
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
        "processes", sol::readonly(&Editor::processes_),
        "workspace", sol::readonly(&Editor::workspace_),
        "face_layers", &Editor::face_layers_,
        "cli_args", sol::readonly(&Editor::cli_args_),

        /* Functions. */
        "quit", [](Editor&) -> void { Editor::stop(); },
        "create_document", [](Editor& self, std::optional<std::string_view> path) -> std::shared_ptr<Document> {
            return self.create_document(path);
        },
        "destroy_document", &Editor::destroy_document,
        "create_document_view", &Editor::create_document_view,
        "destroy_document_view", &Editor::destroy_document_view,
        "create_process", [](Editor& self, const std::string& command, const sol::table& lua_args,
            std::shared_ptr<Document> doc, std::optional<std::size_t> insert_pos) -> std::shared_ptr<AsyncProcess> {
            std::vector<std::string> args;
            for (const auto& kv : lua_args) { args.push_back(kv.second.as<std::string>()); }

            return self.create_process(command, args, std::move(doc), insert_pos);
        },
        "set_status_message", &Editor::set_status_message,
        "clear_status_message", [](Editor& self) -> void { self.workspace_.mini_buffer_.clear_status_message(); },
        "debug_stats", [](Editor& self) -> sol::table {
            auto stats = self.lua_.create_table();

            stats["document_instances"] = Document::instances_.load();
            stats["document_view_instances"] = DocumentView::instances_.load();
            stats["viewport_instances"] = Viewport::instances_.load();
            stats["process_instances"] = AsyncProcess::instances_.load();

            stats["documents"] = self.documents_.size();
            stats["document_views"] = self.document_views_.size();
            stats["processes"] = self.processes_.size();

            return stats;
        });
    // clang-format on
}

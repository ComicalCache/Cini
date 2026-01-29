#include "../editor.hpp"

#include <sol/property.hpp>

#include "../document.hpp"
#include "../viewport.hpp"

void Editor::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Editor>("Editor",
        /* Properties. */
        /// The opened Documents.
        "documents", sol::readonly(&Editor::documents_),
        /// The currently active viewport.
        "viewport", sol::property([](Editor& self) -> std::shared_ptr<Viewport> {
            return self.is_mini_buffer_ ? self.mini_buffer_.viewport_: self.window_manager_.active_viewport_;
        }),
        /// The Mini Buffer viewport.
        "mini_buffer", sol::property([](Editor& self) -> std::shared_ptr<Viewport> { return self.mini_buffer_.viewport_; }),

        /* Functions. */
        /// Creates a new Document.
        "create_document", [](Editor& self, std::optional<std::string_view> path) -> std::shared_ptr<Document> {
            return self.create_document(path);
        },
        /// Sets a status message.
        "set_status_message", &Editor::set_status_message,
        /// Enters the Mini Buffer.
        "enter_mini_buffer", &Editor::enter_mini_buffer,
        /// Exits the Mini Buffer.
        "exit_mini_buffer", &Editor::exit_mini_buffer,
        /// Splits the current Viewport vertically.
        "split_vertical", [](Editor& self, const float ratio) -> void { self.split_viewport(true, ratio); },
        /// Splits the current Viewport horizontally.
        "split_horizontal", [](Editor& self, const float ratio) -> void { self.split_viewport(false, ratio); },
        /// Resizes the current Viewport split.
        "resize_split", [](Editor& self, const float delta) -> void { self.resize_viewport(delta); },
        /// Changes the current Viewport.
        "navigate_splits", &Editor::navigate_window,
        /// Closes the current viewport.
        "close", [](Editor& self) -> void { self.close_viewport(); });
    // clang-format on
}

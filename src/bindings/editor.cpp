#include "../editor.hpp"

#include <sol/property.hpp>

#include "../document.hpp"
#include "../viewport.hpp"

void Editor::init_bridge(sol::table& core) {
    // clang-format off
    core.new_usertype<Editor>("Editor",
        /* Properties. */
        "documents", sol::readonly(&Editor::documents_),
        "viewport", sol::property([](Editor& self) -> std::shared_ptr<Viewport> {
            return self.workspace_.active_tree_viewport_;
        }),
        "mini_buffer", sol::property([](Editor& self) -> std::shared_ptr<Viewport> {
            return self.workspace_.mini_buffer_.viewport_;
        }),

        /* Functions. */
        "create_document", [](Editor& self, std::optional<std::string_view> path) -> std::shared_ptr<Document> {
            return self.create_document(path);
        },
        "destroy_document", &Editor::destroy_document,
        "set_status_message", &Editor::set_status_message,
        "enter_mini_buffer", [](Editor& self) -> void {
            self.workspace_.enter_mini_buffer(self.status_message_timer_);
        },
        "exit_mini_buffer", [](Editor& self) -> void { self.workspace_.exit_mini_buffer(); },
        "split_vertical", [](Editor& self, const float ratio) -> void {
            self.workspace_.split(true, ratio, self.create_viewport(self.workspace_.active_tree_viewport_));
        },
        "split_horizontal", [](Editor& self, const float ratio) -> void {
            self.workspace_.split(false, ratio, self.create_viewport(self.workspace_.active_tree_viewport_));
        },
        "resize_split", [](Editor& self, const float delta) -> void { self.workspace_.resize_split(delta); },
        "navigate_splits", [](Editor& self, Direction direction) -> void {
            self.workspace_.navigate_split(direction);
        },
        "close", [](Editor& self) -> void {
            // Stop event loop on last Viewport close.
            if (const auto ret = self.workspace_.close_split(); ret && !*ret) { uv_stop(self.loop_); }
        });
    // clang-format on
}

#include "workspace_binding.hpp"

#include <sol/table.hpp>

#include "../editor.hpp"
#include "../render/workspace.hpp"
#include "../viewport.hpp"

void WorkspaceBinding::init_bridge(sol::table& core) {
    // clang-format off
        core.new_usertype<Workspace>("Workspace",
            /* Properties */
            "is_mini_buffer", sol::property([](Workspace& self) -> bool { return self.is_mini_buffer_; }),
            "viewport", sol::property([](Workspace& self) -> std::shared_ptr<Viewport> {
                return self.active_tree_viewport_;
            }),
            "mini_buffer", sol::property([](Workspace& self) -> std::shared_ptr<Viewport> {
                return self.mini_buffer_.viewport_;
            }),

            /* Functions */
            "enter_mini_buffer", [](Workspace& self) -> void {
                self.enter_mini_buffer(Editor::instance()->status_message_timer_);
            },
            "exit_mini_buffer", &Workspace::exit_mini_buffer,
            "split_vertical", [](Workspace& self, const float ratio) -> void {
                self.split(true, ratio, Editor::instance()->create_viewport(self.active_tree_viewport_));
            },
            "split_horizontal", [](Workspace& self, const float ratio) -> void {
                self.split(false, ratio, Editor::instance()->create_viewport(self.active_tree_viewport_));
            },
            "resize_split", &Workspace::resize_split,
            "navigate_split", &Workspace::navigate_split,
            "close_split", [](Workspace& self) -> void {
                // Stop event loop on last Viewport close.
                if (const auto ret = self.close_split(); ret && !*ret) { Editor::stop(); }
            }
        );
    // clang-format on
}

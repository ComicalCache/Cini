#include "mini_buffer.hpp"

#include "../document.hpp"
#include "../viewport.hpp"

MiniBuffer::MiniBuffer(const std::size_t width, const std::size_t height, sol::state& lua)
    // These specifically do not call the factory functions as they are not regular.
    : viewport_(std::make_shared<Viewport>(width, height, std::make_shared<Document>(std::nullopt, lua))) {
    this->viewport_->gutter_ = false;
    this->viewport_->mode_line_ = false;
}

void MiniBuffer::set_status_message(const std::string_view message, const std::string_view mode) const {
    this->viewport_->doc_->properties_["minor_mode_override"] = mode;
    this->viewport_->move_cursor([](Cursor& c, const Document& d, std::size_t) -> void { c.point(d, 0); }, 0);
    this->viewport_->doc_->clear();
    this->viewport_->doc_->insert(0, message);
}

void MiniBuffer::clear_status_message() const {
    this->viewport_->move_cursor([](Cursor& c, const Document& d, std::size_t) -> void { c.point(d, 0); }, 0);
    this->viewport_->doc_->clear();
    this->viewport_->doc_->properties_["minor_mode_override"] = nullptr;
}

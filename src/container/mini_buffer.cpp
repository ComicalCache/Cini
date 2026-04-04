#include "mini_buffer.hpp"

#include "../document.hpp"
#include "../viewport.hpp"

MiniBuffer::MiniBuffer(const std::size_t width, const std::size_t height, sol::state& lua)
    // These purposefully do not call the factory functions in Editor as they are not a regular Viewport and Document.
    : viewport_(std::make_shared<Viewport>(width, height, std::make_shared<Document>(std::nullopt, lua))) {
    this->viewport_->gutter_ = false;
    this->viewport_->mode_line_ = false;
}

// Technically these could be const, however this feels semantically incorrect as it changes the semantical Mini Buffer.
// NOLINTBEGIN(readability-make-member-function-const)
void MiniBuffer::set_status_message(const std::string_view message, const std::string_view mode) {
    this->viewport_->doc_->properties_["minor_mode_override"] = mode;
    this->viewport_->reset_cursor();
    this->viewport_->doc_->clear();
    this->viewport_->doc_->insert(0, message);
}

void MiniBuffer::clear_status_message() {
    this->viewport_->reset_cursor();
    this->viewport_->doc_->clear();
    this->viewport_->doc_->properties_["minor_mode_override"] = nullptr;
}
// NOLINTEND(readability-make-member-function-const)

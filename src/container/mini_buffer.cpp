#include "mini_buffer.hpp"

#include "../document.hpp"
#include "../document_view.hpp"
#include "../viewport.hpp"

MiniBuffer::MiniBuffer(const std::size_t width, const std::size_t height, sol::state& lua)
    // These purposefully do not call the factory functions in Editor as they are not a regular Viewport and Document.
    : viewport_(
          std::make_shared<Viewport>(
              width, height, std::make_shared<DocumentView>(std::make_shared<Document>(std::nullopt, lua), lua))) {
    this->viewport_->view_->doc_->views_.push_back(this->viewport_->view_);

    this->viewport_->view_->gutter_ = false;
    this->viewport_->view_->mode_line_ = false;
}

// Technically these could be const, however this feels semantically incorrect as it changes the semantical Mini Buffer.
// NOLINTBEGIN(readability-make-member-function-const)
void MiniBuffer::set_status_message(const std::string_view message, const std::string_view mode) {
    this->viewport_->view_->properties_["minor_mode_override"] = mode;

    this->viewport_->view_->reset_cursor();
    this->viewport_->adjust_viewport();

    this->viewport_->view_->doc_->clear();
    this->viewport_->view_->doc_->insert(0, message);
    this->viewport_->view_->doc_->modified_ = false;
}

void MiniBuffer::clear_status_message() {
    this->viewport_->view_->reset_cursor();
    this->viewport_->adjust_viewport();

    this->viewport_->view_->doc_->clear();
    this->viewport_->view_->properties_["minor_mode_override"] = nullptr;
}
// NOLINTEND(readability-make-member-function-const)

#include "mini_buffer.hpp"

#include "document.hpp"
#include "editor.hpp"
#include "mode.hpp"
#include "viewport.hpp"

MiniBuffer::MiniBuffer(const std::size_t width, const std::size_t height)
    : viewport_(std::make_shared<Viewport>(width, height, std::make_shared<Document>(std::nullopt))) {
    this->viewport_->gutter_ = false;
    this->viewport_->mode_line_ = false;
}

void MiniBuffer::set_mode(const std::string_view mode, Editor& editor) {
    if (const auto major = this->viewport_->doc_->major_mode_; major && major->name_ == mode) { return; }

    this->viewport_->doc_->major_mode_ = editor.get_mode(mode);
    this->viewport_->doc_->clear();
}

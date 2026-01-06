#include "mini_buffer.hpp"

#include "document.hpp"
#include "editor.hpp"
#include "viewport.hpp"

MiniBuffer::MiniBuffer(const std::size_t width, const std::size_t height)
    : viewport_(std::make_shared<Viewport>(width, height, std::make_shared<Document>(std::nullopt))) {
    this->viewport_->gutter_ = false;
    this->viewport_->mode_line_ = false;
}

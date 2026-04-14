#include "face.hpp"

void Face::merge(const Face& other) {
    if (other.fg_) { this->fg_ = other.fg_; }
    if (other.bg_) { this->bg_ = other.bg_; }

    if (other.bold_) { this->bold_ = other.bold_; }
    if (other.italic_) { this->italic_ = other.italic_; }
    if (other.underline_) { this->underline_ = other.underline_; }
    if (other.strikethrough_) { this->strikethrough_ = other.strikethrough_; }
}

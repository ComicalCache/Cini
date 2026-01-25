#include "face_cache.hpp"

#include "../document.hpp"

FaceCache::FaceCache(const std::size_t idx, const Document& doc) : properties_{doc.text_properties_.properties_} {
    if (idx == 0) {
        this->curr_ = this->properties_.begin();
    } else {
        this->curr_ = std::ranges::lower_bound(this->properties_, idx, {}, &Property::start_);
    }
}

void FaceCache::update(const std::size_t idx, const std::function<sol::optional<Face>(std::string_view)>& get_face) {
    if (idx < this->curr_end_) { return; }

    // Idx moved past the end of the current property, cache next Face.
    this->face_ = sol::nullopt;
    while (this->curr_ != this->properties_.end()) {
        // Find next Face property.
        if (this->curr_->end_ <= idx) {
            this->curr_++;
            continue;
        }
        if (this->curr_->key_ != "face") {
            this->curr_++;
            continue;
        }

        // Inside property.
        if (this->curr_->start_ <= idx) {
            if (this->curr_->value_.is<Face>()) {
                this->face_ = this->curr_->value_.as<Face>();
            } else if (this->curr_->value_.is<std::string_view>()) {
                this->face_ = get_face(this->curr_->value_.as<std::string_view>());
            }

            this->curr_end_ = this->curr_->end_;
        } else {
            // Before property.
            this->curr_end_ = this->curr_->start_;
        }

        return;
    }

    // No more Properties left.
    this->curr_end_ = std::numeric_limits<std::size_t>::max();
}

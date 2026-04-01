#include "face_cache.hpp"
#include <algorithm>
#include <iterator>

#include "../document.hpp"

FaceCache::FaceCache(const std::size_t idx, const std::string& key, const Document& doc) {
    auto it = doc.text_properties_.properties_.find(key);
    if (it != doc.text_properties_.properties_.end() && !it->second.empty()) {
        this->properties_ = &it->second;
        if (idx == 0) {
            this->curr_ = this->properties_->begin();
        } else {
            this->curr_ = std::ranges::upper_bound(*this->properties_, idx, {}, &Property::start_);
            if (this->curr_ != this->properties_->begin()) { this->curr_ = std::prev(this->curr_); }
        }
    }
}

void FaceCache::update(const std::size_t idx, const std::function<sol::optional<Face>(std::string_view)>& get_face) {
    // Short circuit on existing match.
    if (idx < this->curr_end_) { return; }

    this->face_ = sol::nullopt;

    // No properties for this key exist.
    if (this->properties_ == nullptr) {
        this->curr_end_ = std::numeric_limits<std::size_t>::max();
        return;
    }

    while (this->curr_ != this->properties_->end()) {
        if (this->curr_->end_ <= idx) {
            this->curr_++;
            continue;
        }

        if (this->curr_->start_ <= idx) { // Inside property.
            if (this->curr_->value_.is<Face>()) {
                this->face_ = this->curr_->value_.as<Face>();
            } else if (this->curr_->value_.get_type() == sol::type::string) {
                this->face_ = get_face(this->curr_->value_.as<std::string_view>());
            }

            this->curr_end_ = this->curr_->end_;
        } else { // Before property.
            this->curr_end_ = this->curr_->start_;
        }

        return;
    }

    // No more Properties left.
    this->curr_end_ = std::numeric_limits<std::size_t>::max();
}

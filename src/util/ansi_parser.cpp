#include "ansi_parser.hpp"

void AnsiParser::parse(uint8_t ch) {
    // https://vt100.net/emu/dec_ansi_parser
    // Check the above for the implementation reference, including the pull requests.

    if (ch == 0x18 || ch == 0x1A) {
        this->state_ = State::Ground;
        if (this->execute_) { this->execute_(ch); }

        return;
    }
    if (ch == 0x1B) {
        this->clear();
        this->state_ = State::Escape;

        return;
    }

    switch (state_) {
        case State::Ground:
            if (ch <= 0x1F) {
                if (this->execute_) { this->execute_(ch); }
            } else if (ch >= 0x20) {
                if (this->print_) { this->print_(ch); }
            }
            break;

        case State::Escape:
            if (ch <= 0x1F) {
                if (this->execute_) { this->execute_(ch); }
            } else if (ch >= 0x20 && ch <= 0x2F) {
                this->collect(ch);
                this->state_ = State::EscapeIntermediate;
            } else if (ch == '[') {
                this->clear();
                this->state_ = State::CsiEntry;
            } else if (ch == ']') {
                this->osc_payload_.clear();
                this->state_ = State::OscString;
            } else if (ch >= 0x30 && ch <= 0x7E) {
                if (this->esc_dispatch_) { this->esc_dispatch_(ch, this->intermediates_); }
                this->state_ = State::Ground;
            } else if (ch >= 0x80) {
                if (this->print_) { this->print_(ch); };
                this->state_ = State::Ground;
            }
            break;

        case State::EscapeIntermediate:
            if (ch <= 0x1F) {
                if (this->execute_) { this->execute_(ch); };
            } else if (ch >= 0x20 && ch <= 0x2F) {
                this->collect(ch);
            } else if (ch >= 0x30 && ch <= 0x7E) {
                if (this->esc_dispatch_) { this->esc_dispatch_(ch, this->intermediates_); }
                this->state_ = State::Ground;
            }
            break;

        case State::CsiEntry:
            if (ch <= 0x1F) {
                if (this->execute_) { this->execute_(ch); }
            } else if (ch >= 0x20 && ch <= 0x2F) {
                this->collect(ch);
                this->state_ = State::CsiIntermediate;
            } else if (ch >= 0x30 && ch <= 0x39) { // NOLINT(bugprone-branch-clone)
                this->collect_param(ch);
                this->state_ = State::CsiParam;
            } else if (ch == ';' || (ch >= 0x3C && ch <= 0x3F)) {
                this->collect_param(ch);
                this->state_ = State::CsiParam;
            } else if (ch >= 0x40 && ch <= 0x7E) {
                if (this->csi_dispatch_) { this->csi_dispatch_(this->params_, ch, this->intermediates_); }

                this->state_ = State::Ground;
            }
            break;

        case State::CsiParam:
            if (ch <= 0x1F) {
                if (this->execute_) { this->execute_(ch); }
            } else if (ch >= 0x30 && ch <= 0x39) {
                collect_param(ch);
            } else if (ch == ';') {
                if (this->has_param_) {
                    this->params_.push_back(this->current_param_);
                } else {
                    this->params_.push_back(0);
                }

                this->current_param_ = 0;
                this->has_param_ = false;
            } else if (ch >= 0x20 && ch <= 0x2F) {
                if (this->has_param_) {
                    this->params_.push_back(this->current_param_);
                    this->has_param_ = false;
                }

                this->collect(ch);
                this->state_ = State::CsiIntermediate;
            } else if (ch >= 0x40 && ch <= 0x7E) {
                if (this->has_param_) { this->params_.push_back(this->current_param_); }
                if (this->csi_dispatch_) { this->csi_dispatch_(this->params_, ch, this->intermediates_); }

                this->state_ = State::Ground;
            } else if (ch >= 0x3A && ch <= 0x3F) {
                this->state_ = State::CsiIgnore;
            }
            break;

        case State::CsiIntermediate:
            if (ch <= 0x1F) {
                if (this->execute_) { this->execute_(ch); }
            } else if (ch >= 0x20 && ch <= 0x2F) {
                this->collect(ch);
            } else if (ch >= 0x40 && ch <= 0x7E) {
                if (this->csi_dispatch_) { this->csi_dispatch_(this->params_, ch, this->intermediates_); }

                this->state_ = State::Ground;
            } else if (ch >= 0x30 && ch <= 0x3F) {
                this->state_ = State::CsiIgnore;
            }
            break;

        case State::CsiIgnore:
            if (ch <= 0x1F) {
                if (this->execute_) { this->execute_(ch); }
            } else if (ch >= 0x40 && ch <= 0x7E) {
                this->state_ = State::Ground;
            }
            break;

        case State::OscString:
            if (ch == 0x07) {
                if (this->osc_dispatch_) { this->osc_dispatch_(this->osc_payload_); }

                this->state_ = State::Ground;
            } else if (ch == 0x1B) {
                if (!this->osc_payload_.empty() && this->osc_payload_.back() == '\\') {
                    if (this->osc_dispatch_) { this->osc_dispatch_(osc_payload_); }
                }

                this->clear();
                this->state_ = State::Escape;
            } else if (ch >= 0x20) {
                this->osc_payload_.push_back(static_cast<char>(ch));
            }
            break;
    }
}

void AnsiParser::parse(const uint8_t* data, std::size_t len) {
    for (std::size_t idx = 0; idx < len; idx += 1) { this->parse(data[idx]); }
}

void AnsiParser::clear() {
    this->params_.clear();
    this->intermediates_.clear();
    this->current_param_ = 0;
    this->has_param_ = false;
}

void AnsiParser::collect(uint8_t ch) { this->intermediates_.push_back(static_cast<char>(ch)); }

void AnsiParser::collect_param(uint8_t ch) {
    if (ch >= '0' && ch <= '9') {
        if (this->current_param_ <= 99999) { this->current_param_ = (this->current_param_ * 10) + (ch - '0'); }
        this->has_param_ = true;
    } else if (ch >= 0x3C && ch <= 0x3F) {
        this->intermediates_.push_back(static_cast<char>(ch));
    }
}

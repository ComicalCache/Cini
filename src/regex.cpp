#include "regex.hpp"

#include <stdexcept>

Regex::Regex(const std::string_view pattern) {
    auto err_code = 0;
    PCRE2_SIZE err_offset = 0;

    this->code_ = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(pattern.data()), pattern.size(), PCRE2_UTF | PCRE2_UCP, &err_code, &err_offset,
        nullptr);

    if (!this->code_) {
        std::vector<PCRE2_UCHAR8> buff(256);
        pcre2_get_error_message_8(err_code, buff.data(), 256);
        throw std::runtime_error(std::string(buff.begin(), buff.end()));
    }

    pcre2_jit_compile(this->code_, PCRE2_JIT_COMPLETE);
    this->match_data_ = pcre2_match_data_create_from_pattern(this->code_, nullptr);
}

Regex::~Regex() {
    if (this->match_data_) { pcre2_match_data_free(this->match_data_); }
    if (this->code_) { pcre2_code_free(this->code_); }
}

[[nodiscard]]
std::vector<RegexMatch> Regex::search_all(const std::string_view text) const {
    std::vector<RegexMatch> matches;

    const auto data = reinterpret_cast<PCRE2_SPTR>(text.data());
    const PCRE2_SIZE len = text.size();
    PCRE2_SIZE offset = 0;

    while (offset < len) {
        if (const auto rc = pcre2_match(this->code_, data, len, offset, 0, this->match_data_, nullptr); rc < 0) {
            break;
        }

        const PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(this->match_data_);
        const std::size_t start = ovector[0];
        const std::size_t end = ovector[1];

        if (start == end) {
            if (offset >= len) { break; }

            // PCRE2 handles UTF-8 state internally, increment is fine.
            offset += 1;
            continue;
        }

        matches.push_back({start, end});
        offset = end;
    }

    return matches;
}

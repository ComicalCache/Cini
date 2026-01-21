#include "regex.hpp"

#include <stdexcept>

#include "util/utf8.hpp"

Regex::Regex(const std::string_view pattern) {
    auto err_code = 0;
    PCRE2_SIZE err_offset = 0;

    const auto code = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(pattern.data()), pattern.size(), PCRE2_UTF | PCRE2_UCP, &err_code, &err_offset,
        nullptr);

    if (!code) {
        std::vector<PCRE2_UCHAR8> buff(256);
        pcre2_get_error_message_8(err_code, buff.data(), 256);
        throw std::runtime_error(std::string(buff.begin(), buff.end()));
    }

    this->code_ = std::shared_ptr<pcre2_code>(code, pcre2_code_free);

    pcre2_jit_compile(this->code_.get(), PCRE2_JIT_COMPLETE);
    const auto match_data = pcre2_match_data_create_from_pattern(this->code_.get(), nullptr);

    this->match_data_ = std::shared_ptr<pcre2_match_data>(match_data, pcre2_match_data_free);
}

[[nodiscard]]
std::vector<RegexMatch> Regex::search(const std::string_view text) const {
    std::vector<RegexMatch> matches;

    const auto data = reinterpret_cast<PCRE2_SPTR>(text.data());
    const PCRE2_SIZE len = text.size();
    PCRE2_SIZE offset = 0;

    while (offset < len) {
        if (const auto rc = pcre2_match(this->code_.get(), data, len, offset, 0, this->match_data_.get(), nullptr);
            rc < 0) {
            break;
        }

        const PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(this->match_data_.get());
        const std::size_t start = ovector[0];
        const std::size_t end = ovector[1];

        if (start == end) {
            if (offset >= len) { break; }

            if (const auto ch_len = utf8::len(text[offset]); ch_len == 0 || offset + ch_len <= len) {
                offset += ch_len;
            } else {
                offset += 1;
            }

            continue;
        }

        matches.push_back({start, end});
        offset = end;
    }

    return matches;
}

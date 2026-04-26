#ifndef ANSI_PARSER_HPP_
#define ANSI_PARSER_HPP_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

/// A stateful ANSI parser.
///
/// https://vt100.net/emu/dec_ansi_parser
/// https://github.com/haberman/vtparse
struct AnsiParser {
private:
    enum struct State : std::uint8_t {
        Ground,
        Escape,
        EscapeIntermediate,
        CsiEntry,
        CsiParam,
        CsiIntermediate,
        CsiIgnore,
        OscString
    };

public:
    /// Callback for a print action.
    std::function<void(uint8_t)> print_;
    /// Callback for an execute action.
    std::function<void(uint8_t)> execute_;
    /// Callback for a CSI dispatch action.
    std::function<void(const std::vector<int>&, uint8_t, const std::string&)> csi_dispatch_;
    /// Callback for an ESC dispatch action.
    std::function<void(uint8_t, const std::string&)> esc_dispatch_;
    /// Callback for an OSC dispatch action.
    std::function<void(const std::string&)> osc_dispatch_;

private:
    State state_{State::Ground};

    std::vector<int> params_;
    int current_param_ = 0;
    bool has_param_ = false;

    std::string intermediates_;

    std::string osc_payload_;

public:
    void parse(uint8_t ch);
    void parse(const uint8_t* data, std::size_t len);

private:
    void state_change(State next_state, uint8_t ch);
    void action(uint8_t ch);
    void clear();
    void collect(uint8_t ch);
    void collect_param(uint8_t ch);
};

#endif

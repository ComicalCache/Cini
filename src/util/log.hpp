#ifndef LOG_HPP_
#define LOG_HPP_

#include <functional>
#include <string_view>

namespace log {
    /// Function called when a status message is set.
    inline std::function<void(std::string_view)> status_massage_handler;

    /// Set a status message in the editor.
    void set_status_message(std::string_view msg);
}

#endif

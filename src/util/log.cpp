#include "log.hpp"

namespace log {
    void set_status_message(const std::string_view msg) {
        if (log::status_massage_handler) { log::status_massage_handler(msg); }
    }
}

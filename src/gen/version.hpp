#ifndef VERSION_HPP_
#define VERSION_HPP_

#include <string_view>

namespace version {
    extern const std::string_view NAME;
    extern const std::string_view VERSION;
    extern const std::string_view BUILD_DATE;
    extern const std::string_view BUILD_TYPE;
} // namespace version

#endif

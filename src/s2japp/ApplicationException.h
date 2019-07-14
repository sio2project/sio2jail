#pragma once

#include "common/Exception.h"

namespace s2j {
namespace app {

class InvalidConfigurationException : public Exception {
public:
    InvalidConfigurationException(const std::string& msg)
            : Exception("Invalid configration: " + msg) {}
};

} // namespace app
} // namespace s2j

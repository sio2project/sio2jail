#pragma once

#include "Logger.h"

namespace s2j {
namespace logger {

class VoidLogger : public Logger {
public:
    bool isLoggerFD(int /* fd */) const noexcept override { return false; }

protected:
    void write(const std::string&) noexcept override {}
};

}
}

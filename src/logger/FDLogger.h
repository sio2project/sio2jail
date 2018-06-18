#pragma once

#include "Logger.h"

#include "common/FD.h"

namespace s2j {
namespace logger {

class FDLogger : public Logger {
public:
    FDLogger(int fd, bool close = false);
    ~FDLogger();

    bool isLoggerFD(int fd) const noexcept override;

protected:
    void write(const std::string& string) noexcept;

private:
    int fd_;
    bool close_;
};

}
}

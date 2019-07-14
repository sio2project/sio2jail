#include "FDLogger.h"

namespace s2j {
namespace logger {

FDLogger::FDLogger(int fd, bool close) : fd_(fd), close_(close) {
    if (fd_ < 0) {
        throw SystemException("Invalid logger initialization");
    }
}

FDLogger::~FDLogger() {
    if (close_ && fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

bool FDLogger::isLoggerFD(int fd) const noexcept {
    return fd == fd_;
}

void FDLogger::write(const std::string& string) noexcept {
    for (size_t written = 0; written < string.size();) {
        ssize_t res =
                ::write(fd_, string.c_str() + written, string.size() - written);
        if (res < 0) {
            if (errno != EAGAIN && errno != EINTR) {
                break;
            }
        }
        else {
            written += res;
        }
    }
}

} // namespace logger
} // namespace s2j

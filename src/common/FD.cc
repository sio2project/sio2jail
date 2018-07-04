#include "FD.h"

#include "Exception.h"
#include "WithErrnoCheck.h"

#include <unistd.h>
#include <fcntl.h>

namespace s2j {

FD::FD(int fd, bool close)
    : close_(close) {
    if (fd < 0) {
        throw Exception("tried to wrap a negative fd");
    }
    fd_ = fd;
}

FD::~FD() {
    if (close_) {
        try {
            close();
        } catch (...) {}
    }
}

void FD::close() {
    if (fd_ >= 0) {
        s2j::withErrnoCheck("close", ::close, fd_);
        fd_ = -1;
    }
}

FD& FD::operator<<(const std::string& str) {
    write(str, false);
    return *this;
}

void FD::write(const std::string& str, bool allowPartialWrites) {
    for (size_t written = 0; written < str.size();) {
        ssize_t result = s2j::withErrnoCheck(
                "write", {EAGAIN, EINTR}, ::write, fd_, str.c_str() + written, str.size() - written);
        if (result > 0)
            written += result;
        if (!allowPartialWrites && written != str.size())
            throw SystemException("Partial write: " + std::to_string(written) + "/" + std::to_string(str.size()));
    }
}

FD FD::open(const std::string& path, int flags) {
    return FD::withErrnoCheck("open " + path, ::open, path.c_str(), flags);
}

FD FD::open(const std::string& path, int flags, mode_t mode) {
    return FD::withErrnoCheck("open " + path, ::open, path.c_str(), flags, mode);
}

FD::operator int() const {
    return fd_;
}

bool FD::good() const {
    int errnoCode = 0;
    do {
        errnoCode = s2j::withErrnoCheck(
                "fcntl", std::initializer_list<int>{EBADF, EINTR, EAGAIN}, ::fcntl, fd_, F_GETFD).getErrnoCode();
        if (errnoCode == EBADF)
            return false;
    } while (errnoCode < 0);
    return true;
}

}

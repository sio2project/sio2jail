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
    // Do _not_ handle partial writes here, to handle partial writes use
    // (and implement) version of write that does it.
    ssize_t written = s2j::withErrnoCheck("write", ::write, fd_, str.c_str(), str.size());
    if (written < 0 || static_cast<size_t>(written) != str.size())
        throw SystemException("Partial write: " + std::to_string(written) + "/" + std::to_string(str.size()));
    return *this;
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

}

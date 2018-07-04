#pragma once

#include "Exception.h"
#include "WithErrnoCheck.h"

#include <string>

namespace s2j {

class FD {
public:
    FD(int fd, bool close = false);
    FD(const FD&) = delete;
    FD(FD&&) = default;
    virtual ~FD();

    void close();

    void write(const std::string& str, bool allowPartialWrites = true);
    FD& operator<<(const std::string& str);

    operator int() const;

    bool good() const;

    static FD open(const std::string& path, int flags);
    static FD open(const std::string& path, int flags, mode_t mode);
private:
    template<typename Operation, typename ...Args>
    static FD withErrnoCheck(const std::string& description, Operation operation, Args ...args) {
        int fd = s2j::withErrnoCheck(description, operation, args...);
        return FD(fd, true);
    }

    int fd_;
    bool close_;
};

}

#pragma once

#include "common/Exception.h"

namespace s2j {
namespace seccomp {

class SeccompException : public Exception {
public:
    SeccompException(const std::string& msg)
        : Exception(msg) {}
};

class SeccompFilterAlreadyCompiledException : public SeccompException {
public:
    SeccompFilterAlreadyCompiledException()
        : SeccompException("Can't add new seccomp rule, seccomp filter already compiled") {}
};

class UnknownSyscallNameException : public SeccompException {
public:
    UnknownSyscallNameException(const std::string& syscallName)
        : SeccompException("Unknown syscall \"" + syscallName + "\"") {}
};

}
}

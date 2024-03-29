#pragma once

#include <cstdint>
#include <string>

namespace s2j {

class Exception : public std::exception {
public:
    Exception(const std::string& msg);

    const char* what() const noexcept;

private:
    const std::string msg_;
};

class SystemException : public Exception {
public:
    SystemException(const std::string& msg);
    SystemException(const std::string& msg, int errnoNumber);

    uint32_t getErrno() const;

private:
    const uint32_t errno_;
};

class NotImplementedException : public Exception {
public:
    NotImplementedException(const std::string& msg) : Exception(msg){};
};

} // namespace s2j

#define NOT_IMPLEMENTED()                                                      \
    throw s2j::NotImplementedException(                                        \
            "Not implemented: " __FILE__ ":" + std::to_string(__LINE__))

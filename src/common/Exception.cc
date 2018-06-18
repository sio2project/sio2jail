#include "Exception.h"

#include "logger/Logger.h"

#include <cstring>

namespace s2j {

Exception::Exception(const std::string& msg)
    : msg_(msg) {
    logger::error("Exception: ", msg);
}

const char* Exception::what() const noexcept {
    return msg_.c_str();
}

SystemException::SystemException(const std::string& msg)
    : SystemException(msg, errno) {}

SystemException::SystemException(const std::string& msg, int errnoNumber)
    : Exception("System error occured: " + msg +
                ": error " + std::to_string(errnoNumber) + ": " + strerror(errnoNumber))
    , errno_(errnoNumber) {}

uint32_t SystemException::getErrno() const {
    return errno_;
}

}

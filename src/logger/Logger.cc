#include "Logger.h"

#include "VoidLogger.h"

namespace s2j {
namespace logger {

std::shared_ptr<logger::Logger> Logger::logger_;

void Logger::setLogger(std::shared_ptr<Logger> logger) noexcept {
    if (logger != nullptr)
        logger_ = logger;
}

std::shared_ptr<Logger> Logger::getLogger() noexcept {
    if (logger_ == nullptr)
        logger_ = std::make_shared<VoidLogger>();
    return logger_;
}

}
}

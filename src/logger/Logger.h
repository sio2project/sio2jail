#pragma once
#pragma GCC system_header

#include "executor/ExecuteEventListener.h"
#include "tracer/TraceEventListener.h"

#include "common/TypeList.h"
#include "common/Exception.h"
#include "common/Preprocessor.h"

#include <sys/time.h>

#include <sstream>
#include <ostream>
#include <chrono>
#include <iomanip>
#include <memory>
#include <cstring>

#define VAR(v) #v, "=", v
#define TRACE(...) \
    s2j::logger::Logger::Trace trace_(__PRETTY_FUNCTION__ MAP(_TRACE_PARAM, __VA_ARGS__));
#define _TRACE_PARAM(p) \
    , ", ", VAR(p)

namespace s2j {
namespace logger {

#ifdef NTRACE
const constexpr bool enableTrace = false;
#else
const constexpr bool enableTrace = true;
#endif

template<typename ...Args>
void trace(Args&& ...args) noexcept;

template<typename ...Args>
void debug(Args&& ...args) noexcept;

template<typename ...Args>
void error(Args&& ...args) noexcept;

inline bool isLoggerFD(int fd) noexcept;

class Logger {
    friend class LogSource;
public:
    class Trace {
    public:
        template<typename ...Args>
        Trace(const char* functionName, Args&& ...args)
            : functionName_(functionName) {
            trace(functionName_, args...);
        }

        ~Trace() {
            try {
                trace(functionName_, " -");
            } catch (...) {}
        }

    private:
        const char* const functionName_;
    };

    virtual ~Logger() = default;

    template<typename ...Args>
    void log(const std::string& level, Args&& ...args) noexcept;

    virtual bool isLoggerFD(int fd) const noexcept = 0;

    /**
     * Singleton pattern for logger.
     */
    static void setLogger(std::shared_ptr<Logger> logger) noexcept;
    static std::shared_ptr<Logger> getLogger() noexcept;

protected:
    template<typename Arg, typename ...Args>
    void log(std::ostream& stream,
            const std::string& delimeter,
            Arg&& arg,
            Args&& ...args) noexcept;

    void log(std::ostream& stream, const std::string& delimeter) noexcept {}

    virtual void write(const std::string& string) noexcept = 0;

private:
    static std::shared_ptr<Logger> logger_;
};

template<typename ...Args>
void trace(Args&& ...args) noexcept {
    if (enableTrace) {
        Logger::getLogger()->log("TRACE", args...);
    }
}

template<typename ...Args>
void debug(Args&& ...args) noexcept {
    Logger::getLogger()->log("DEBUG", args...);
}

template<typename ...Args>
void info(Args&& ...args) noexcept {
    Logger::getLogger()->log("INFO", args...);
}

template<typename ...Args>
void warn(Args&& ...args) noexcept {
    Logger::getLogger()->log("WARN", args...);
}

template<typename ...Args>
void error(Args&& ...args) noexcept {
    Logger::getLogger()->log("ERROR", args...);
}

inline bool isLoggerFD(int fd) noexcept {
    return Logger::getLogger()->isLoggerFD(fd);
}

template<typename ...Args>
void Logger::log(const std::string& level, Args&& ...args) noexcept {
    std::stringstream stream;

    struct timeval tv;
    gettimeofday(&tv, nullptr);

    char strTimeBuff[64];
    if (std::strftime(strTimeBuff,
                sizeof(strTimeBuff), "%F %T", std::localtime(&tv.tv_sec)) == 0)
        ::strcpy(strTimeBuff, "0000-00-00 00:00:00.000000");

    stream
        << strTimeBuff
        << "."
        << std::setw(6) << std::setfill('0') << std::right << tv.tv_usec
        << "\t"
        << level
        << "\t";
    log(stream, "", args...);
    stream
        << std::endl;

    write(stream.str());
}

template<typename Arg, typename ...Args>
void Logger::log(std::ostream& stream,
        const std::string& delimeter,
        Arg&& arg,
        Args&& ...args) noexcept {
    stream << arg << delimeter;
    log(stream, delimeter, args...);
}

}
}

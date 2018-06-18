#pragma once

#include "ApplicationSettings.h"

#include "printer/OutputBuilder.h"
#include "logger/Logger.h"

namespace s2j {
namespace app {

class Application {
public:
    enum ExitCode : int {
        OK = 0,
        PARSE_ERROR = 1,
        FATAL_ERROR = 2,
    };

    Application(int argc, const char* argv[]);
    Application(const ApplicationSettings& settings);

    ExitCode main();

private:
    ExitCode handleHelp();
    ExitCode handleVersion();
    ExitCode handleRun();

    const ApplicationSettings settings_;
    std::shared_ptr<logger::Logger> logger_;

    void initializeLogger();

    template<typename Listener, typename ...Args>
    std::shared_ptr<Listener> createListener(Args ...args);

    template<typename ListenerType, typename Operation, typename ...Listeners>
    void forEachListener(Operation operation, Listeners ...listeners);
};

}
}

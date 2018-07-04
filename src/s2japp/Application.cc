#include "Application.h"
#include "ApplicationException.h"

#include "executor/Executor.h"
#include "tracer/TraceExecutor.h"
#include "perf/PerfListener.h"
#include "seccomp/SeccompListener.h"
#include "ns/PIDNamespaceListener.h"
#include "ns/NetNamespaceListener.h"
#include "ns/IPCNamespaceListener.h"
#include "ns/UTSNamespaceListener.h"
#include "ns/UserNamespaceListener.h"
#include "ns/MountNamespaceListener.h"
#include "limits/MemoryLimitListener.h"
#include "limits/OutputLimitListener.h"
#include "limits/TimeLimitListener.h"
#include "priv/PrivListener.h"
#include "files/FilesListener.h"
#include "logger/FDLogger.h"
#include "logger/FileLogger.h"
#include "logger/LoggerListener.h"

#include <list>
#include <iostream>

namespace s2j {
namespace app {

Application::Application(const ApplicationSettings& settings)
    : settings_(settings) {
    initializeLogger();
}

Application::Application(int argc, const char* argv[])
    : Application(ApplicationSettings(argc, argv)) {}

Application::ExitCode Application::main() {
    if (settings_.action == ApplicationSettings::Action::RUN) {
        return handleRun();
    }
    else if (settings_.action == ApplicationSettings::Action::PRINT_HELP) {
        return handleHelp();
    }
    else {
        return handleVersion();
    }
}

Application::ExitCode Application::handleHelp() {
    if (!settings_.parsingError.empty())
        std::cout << "Error:" << std::endl << "    " << settings_.parsingError << std::endl << std::endl;
    std::cout << settings_.helpMessage;
    return settings_.parsingError.empty() ? ExitCode::OK : ExitCode::PARSE_ERROR;
}

Application::ExitCode Application::handleVersion() {
    std::cout << settings_.versionMessage;
    return ExitCode::OK;
}

Application::ExitCode Application::handleRun() {
    TRACE();

    auto executor = std::make_shared<s2j::executor::Executor>(settings_.programName, settings_.programArgv);

    auto traceExecutor          = createListener<tracer::TraceExecutor>();
    auto perfListener           = createListener<perf::PerfListener>(settings_.instructionCountLimit);
    auto userNsListener         = createListener<ns::UserNamespaceListener>();
    auto utsNsListener          = createListener<ns::UTSNamespaceListener>();
    auto ipcNsListener          = createListener<ns::IPCNamespaceListener>();
    auto netNsListener          = createListener<ns::NetNamespaceListener>();
    auto pidNsListener          = createListener<ns::PIDNamespaceListener>();
    auto mountNsListener        = createListener<ns::MountNamespaceListener>(settings_,
                                                                             settings_.programName,
                                                                             settings_.features.count(Feature::MOUNT_PROCFS) > 0);
    auto privListener           = createListener<priv::PrivListener>();
    auto seccompListener        = createListener<seccomp::SeccompListener>();
    auto memoryLimitListener    = std::make_shared<limits::MemoryLimitListener>(settings_.memoryLimitKb);
    auto outputLimitListener    = std::make_shared<limits::OutputLimitListener>(settings_.outputLimitB);
    auto timeLimitListener      = std::make_shared<limits::TimeLimitListener>(settings_.rTimelimitUs,
                                                                              settings_.uTimelimitUs,
                                                                              settings_.sTimelimitUs,
                                                                              settings_.usTimelimitUs);
    auto filesListener          = std::make_shared<files::FilesListener>(settings_.suppressStderr);
    auto loggerListener         = std::make_shared<logger::LoggerListener>();

    auto resultsFD = FD(settings_.resultsFD, false);
    if (!resultsFD.good())
        throw InvalidConfigurationException("invalid results file descriptor");

    // Some listeners can return output
    auto outputBuilder = settings_.createOutputBuilder();
    forEachListener<s2j::printer::OutputSource>(
            [outputBuilder](auto listener) { listener->setOutputBuilder(outputBuilder); },
            executor, traceExecutor, perfListener, seccompListener, memoryLimitListener, outputLimitListener, timeLimitListener);

    // Add listeners to executor, *in order*
    forEachListener<executor::ExecuteEventListener>(
            [executor](auto listener) { executor->addEventListener(listener); },
            loggerListener,
            memoryLimitListener,
            outputLimitListener,
            timeLimitListener,
            traceExecutor,
            perfListener,
            userNsListener, pidNsListener, utsNsListener, ipcNsListener, netNsListener, mountNsListener,
            privListener, filesListener, seccompListener
            );

    // Additional trace event listeners
    if (traceExecutor != nullptr) {
        forEachListener<tracer::TraceEventListener>(
                [traceExecutor](auto listener) { traceExecutor->addEventListener(listener); },
                loggerListener,
                memoryLimitListener,
                seccompListener
                );
    }

    // Additional syscall policies
    if (seccompListener != nullptr) {
        forEachListener<seccomp::policy::SyscallPolicy>(
                [seccompListener](auto policy) { seccompListener->addPolicy(*policy); },
                memoryLimitListener
                );
    }

    // Special configuration
    if (mountNsListener != nullptr) {
        mountNsListener->addEventListener(executor);
    }

    // Execute program...
    executor->execute();

    // ...and display output.
    resultsFD.write(outputBuilder->dump());
    return ExitCode::OK;
}

void Application::initializeLogger() {
    if (settings_.loggerPath == "-")
        logger_ = std::make_shared<s2j::logger::FDLogger>(2 /* stderr */);
    else if (!settings_.loggerPath.empty())
        logger_ = std::make_shared<s2j::logger::FileLogger>(settings_.loggerPath);

    logger::Logger::setLogger(logger_);
    logger::debug("Logger initialized");
}

template<typename Listener, typename ...Args>
std::shared_ptr<Listener> Application::createListener(Args ...args) {
    std::shared_ptr<Listener> listener;
    if (settings_.features.count(Listener::feature) > 0)
        listener = std::make_shared<Listener>(args...);
    return listener;
}

template<typename ListenerType, typename Operation, typename ...Listeners>
void Application::forEachListener(Operation operation, Listeners ...listeners) {
    for (auto listener: std::initializer_list<std::shared_ptr<ListenerType>>({listeners...})) {
        if (listener != nullptr) {
            operation(listener);
        }
    }
}

}
}

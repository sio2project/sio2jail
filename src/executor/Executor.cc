#include "Executor.h"

#include "common/Exception.h"
#include "common/Utils.h"
#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace {

volatile sig_atomic_t sigioOccurred = 0;
volatile sig_atomic_t sigalrmOccurred = 0;

void signalSigioHandler(int /*signum*/) {
    sigioOccurred = 1;
}

void signalSigalrmHandler(int /*signum*/) {
    sigalrmOccurred = 1;
}

} // namespace

namespace s2j {
namespace executor {

Executor::Executor(
        std::string childProgramName,
        std::vector<std::string> childProgramArgv)
        : childProgramName_(std::move(childProgramName))
        , childProgramArgv_(std::move(childProgramArgv))
        , childPid_(0) {}

void Executor::execute() {
    TRACE();

    for (auto& listener: eventListeners_) {
        listener->onPreFork();
    }

    childPid_ = withErrnoCheck("fork", fork);
    if (childPid_ == 0) {
        executeChild();
    }
    else {
        executeParent();
    }
}

void Executor::executeChild() {
    TRACE();

    for (auto& listener: eventListeners_) {
        listener->onPostForkChild();
    }

    // Create plain C arrays with program arguments
    char* programName = stringToCStr(childProgramName_);
    char** programArgv = new char*[childProgramArgv_.size() + 2];

    programArgv[0] = programName;
    for (size_t argIndex = 0; argIndex < childProgramArgv_.size(); ++argIndex) {
        programArgv[argIndex + 1] = stringToCStr(childProgramArgv_[argIndex]);
    }
    programArgv[childProgramArgv_.size() + 1] = nullptr;

    // And execute program!
    withErrnoCheck("execv", execv, programName, programArgv);

    std::stringstream ss;
    ss << "execve(" << programName << ", {";
    for (size_t argIndex = 0; argIndex < childProgramArgv_.size() + 2;
         ++argIndex) {
        if (programArgv[argIndex] != nullptr) {
            ss << programArgv[argIndex] << ", ";
            delete[] programArgv[argIndex];
        }
        else {
            ss << "nullptr})";
        }
    }
    throw SystemException(ss.str());
}

void Executor::executeParent() {
    TRACE();

    setupSignalHandling();

    for (auto& listener: eventListeners_) {
        listener->onPostForkParent(childPid_);
    }

    while (true) {
        ExecuteEvent event{};
        siginfo_t waitInfo;

        executor::ExecuteAction action = checkSignals();
        if (action == ExecuteAction::KILL) {
            killChild();
        }

        // Potential race condition here.
        int returnValue =
                waitid(P_ALL, -1, &waitInfo, WEXITED | WSTOPPED | WNOWAIT);
        if (returnValue == -1) {
            if (errno != EINTR) {
                throw SystemException(
                        std::string("waitid failed: ") + strerror(errno));
            }
            continue;
        }

        event.pid = waitInfo.si_pid;
        if (waitInfo.si_code == CLD_EXITED) {
            event.exited = true;
            event.exitStatus = waitInfo.si_status;
        }
        else if (
                waitInfo.si_code == CLD_KILLED ||
                waitInfo.si_code == CLD_DUMPED) {
            event.killed = true;
            event.signal = waitInfo.si_status;
        }
        else if (waitInfo.si_code == CLD_STOPPED) {
            event.stopped = true;
            event.signal = waitInfo.si_status;
        }
        else if (waitInfo.si_code == CLD_TRAPPED) {
            event.trapped = true;
            event.signal = waitInfo.si_status;
        }

        for (auto& listener: eventListeners_) {
            action = std::max(action, listener->onExecuteEvent(event));
        }

        if (event.exited || event.killed) {
            if (event.pid == childPid_) {
                if (event.exited) {
                    outputBuilder_->setExitStatus(event.exitStatus);
                }
                if (event.killed) {
                    outputBuilder_->setKillSignal(event.signal);
                }
                break;
            }
            else {
                // Wait to clear child's status so that future
                // wait calls will not return it.
                withErrnoCheck(
                        "waitid for exited child",
                        {ECHILD},
                        waitid,
                        P_PID,
                        event.pid,
                        nullptr,
                        WEXITED | WSTOPPED | WNOHANG);
            }
        }

        if (action == ExecuteAction::KILL) {
            killChild();
        }
    }
    for (auto& listener: eventListeners_) {
        listener->onPostExecute();
    }
}

void Executor::setupSignalHandling() {
    TRACE();

    sigioOccurred = 0;
    sigalrmOccurred = 0;

    struct sigaction signalAction {};
    signalAction.sa_flags = 0;
    withErrnoCheck("sigfillset", sigfillset, &signalAction.sa_mask);

    signalAction.sa_handler = signalSigioHandler;
    withErrnoCheck("sigaction", sigaction, SIGIO, &signalAction, nullptr);

    signalAction.sa_handler = signalSigalrmHandler;
    withErrnoCheck("sigaction", sigaction, SIGALRM, &signalAction, nullptr);
}

executor::ExecuteAction Executor::checkSignals() {
    TRACE();

    executor::ExecuteAction action = executor::ExecuteAction::CONTINUE;
    if (sigioOccurred != 0) {
        sigioOccurred = 0;
        for (auto& listener: eventListeners_) {
            action = std::max(action, listener->onSigioSignal());
        }
    }
    if (sigalrmOccurred != 0) {
        sigalrmOccurred = 0;
        for (auto& listener: eventListeners_) {
            action = std::max(action, listener->onSigalrmSignal());
        }
    }

    return action;
}

void Executor::onProgramNameChange(const std::string& newProgramName) {
    TRACE(newProgramName);

    setChildProgramName(newProgramName);
}

void Executor::killChild() {
    try {
        outputBuilder_->setKillSignal(SIGKILL);
        logger::debug("Killing trace after execute action kill");
        withErrnoCheck("kill child", kill, childPid_, SIGKILL);
    }
    catch (const s2j::SystemException& ex) {
        if (ex.getErrno() != ESRCH) {
            throw;
        }
    }
}

} // namespace executor
} // namespace s2j

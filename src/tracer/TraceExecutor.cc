#include "TraceExecutor.h"

#include "common/ProcFS.h"
#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <csignal>

namespace s2j {
namespace tracer {

const Feature TraceExecutor::feature = Feature::PTRACE;

void TraceExecutor::onPostForkChild() {
    TRACE();

    // Let's be a tracee.
    withErrnoCheck(
            "ptrace traceme", ptrace, PTRACE_TRACEME, 0, nullptr, nullptr);
    kill(getpid(), SIGTRAP);
}

void TraceExecutor::onPostForkParent(pid_t childPid) {
    TRACE();

    traceePid_ = childPid;

    // Wait for tracee to call PTRACE_TRACEME
    withErrnoCheck("initial wait", waitpid, traceePid_, nullptr, 0);

    // Let's be a tracer.
    withErrnoCheck(
            "ptrace setopts",
            ptrace,
            PTRACE_SETOPTIONS,
            traceePid_,
            nullptr,
            PTRACE_O_EXITKILL | PTRACE_O_TRACESECCOMP | PTRACE_O_TRACEEXEC);

    // Resume tracee.
    withErrnoCheck(
            "ptrace cont", ptrace, PTRACE_CONT, traceePid_, nullptr, nullptr);
}

executor::ExecuteAction TraceExecutor::onExecuteEvent(
        const executor::ExecuteEvent& executeEvent) {
    TRACE();

    static const uint64_t IGNORED_SIGNALS =
            (1 << SIGCHLD) | (1 << SIGCLD) | (1 << SIGURG) | (1 << SIGWINCH);

    TraceEvent event{executeEvent};
    Tracee tracee(traceePid_);

    if (!hasExecved_ && executeEvent.trapped &&
        executeEvent.signal == (SIGTRAP | (PTRACE_EVENT_EXEC << 8))) {
        hasExecved_ = true;
        for (auto& listener: eventListeners_) {
            listener->onPostExec(event);
        }
    }

    TraceAction action = TraceAction::CONTINUE;
    for (auto& listener: eventListeners_) {
        action = std::max(action, listener->onTraceEvent(event, tracee));
    }

    if (!executeEvent.exited && !executeEvent.killed && tracee.isAlive()) {
        int64_t signal = executeEvent.signal & 0xff;
        if (executeEvent.trapped && signal == SIGTRAP) {
            signal = 0;
        }

        if (signal > 0) {
            // Since our child is pid 1 we have to kill on delivery on uncaught
            // signal in favour of kernel.
            uint64_t caughtSignals =
                    procfs::readProcFS(traceePid_, procfs::Field::SIG_CGT);
            caughtSignals |= IGNORED_SIGNALS;
            if ((caughtSignals & (1 << signal)) == 0U) {
                outputBuilder_->setKillSignal(signal);
                logger::debug(
                        "Delivery of uncaught signal ",
                        signal,
                        " killing instead");
                signal = SIGKILL;
                action = TraceAction::KILL;
            }
        }

        if (signal != executeEvent.signal && signal > 0) {
            logger::debug("Continuing tracee with signal injection ", signal);
        }
        else {
            logger::debug("Continuing tracee with signal ", signal);
        }

        try {
            if (action == TraceAction::KILL) {
                // Kill tracee _before_ restarting it
                outputBuilder_->setKillSignal(SIGKILL);
                logger::debug("Killing tracee after trace action kill");
                withErrnoCheck("kill child", kill, traceePid_, SIGKILL);
            }
            withErrnoCheck(
                    "ptrace cont",
                    ptrace,
                    PTRACE_CONT,
                    traceePid_,
                    nullptr,
                    signal);
        }
        catch (const SystemException& ex) {
            if (ex.getErrno() != ESRCH) {
                throw;
            }
        }
    }

    if (action == TraceAction::KILL) {
        return executor::ExecuteAction::KILL;
    }
    return executor::ExecuteAction::CONTINUE;
}

} // namespace tracer
} // namespace s2j

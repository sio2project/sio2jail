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

const uint64_t TraceExecutor::PTRACE_OPTIONS =
        PTRACE_O_EXITKILL | PTRACE_O_TRACESECCOMP | PTRACE_O_TRACEEXEC;

void TraceExecutor::onPostForkChild() {
    TRACE();

    // Let's be a tracee.
    withErrnoCheck(
            "ptrace traceme", ptrace, PTRACE_TRACEME, 0, nullptr, nullptr);
    kill(getpid(), SIGTRAP);
}

void TraceExecutor::onPostForkParent(pid_t childPid) {
    TRACE();

    rootTraceePid_ = childPid;

    // Wait for tracee to call PTRACE_TRACEME
    withErrnoCheck("initial wait", waitpid, rootTraceePid_, nullptr, 0);

    // Let's be a tracer.
    withErrnoCheck(
            "ptrace setopts",
            ptrace,
            PTRACE_SETOPTIONS,
            rootTraceePid_,
            nullptr,
            PTRACE_OPTIONS);

    // Resume tracee.
    withErrnoCheck(
            "ptrace cont",
            ptrace,
            PTRACE_CONT,
            rootTraceePid_,
            nullptr,
            nullptr);
}

executor::ExecuteAction TraceExecutor::onExecuteEvent(
        const executor::ExecuteEvent& executeEvent) {
    TRACE();

    TraceEvent event{executeEvent};
    Tracee tracee(executeEvent.pid);

    TraceAction action = TraceAction::CONTINUE;
    if (!hasExecved_ && executeEvent.trapped &&
        executeEvent.signal == (SIGTRAP | (PTRACE_EVENT_EXEC << 8))) {
        action = std::max(action, onEventExec(event, tracee));
    }

    for (auto& listener: eventListeners_) {
        action = std::max(action, listener->onTraceEvent(event, tracee));
    }

    if (!executeEvent.exited && !executeEvent.killed && tracee.isAlive()) {
        auto handleSignalResult = handleTraceeSignal(event, tracee);
        action = std::max(action, std::get<0>(handleSignalResult));

        continueTracee(action, std::get<1>(handleSignalResult), event, tracee);
    }

    if (action == TraceAction::KILL) {
        return executor::ExecuteAction::KILL;
    }

    return executor::ExecuteAction::CONTINUE;
}

TraceAction TraceExecutor::onEventExec(
        const TraceEvent& event,
        Tracee& tracee) {
    TraceAction action = TraceAction::CONTINUE;

    hasExecved_ = true;
    for (auto& listener: eventListeners_) {
        action = std::max(action, listener->onPostExec(event, tracee));
    }

    return action;
}

std::tuple<TraceAction, int> TraceExecutor::handleTraceeSignal(
        const TraceEvent& event,
        Tracee& tracee) {
    static const uint64_t IGNORED_SIGNALS =
            (1 << SIGCHLD) | (1 << SIGCLD) | (1 << SIGURG) | (1 << SIGWINCH);

    TraceAction action = TraceAction::CONTINUE;

    int signal = event.executeEvent.signal & 0xff;
    if (event.executeEvent.trapped && signal == SIGTRAP) {
        signal = 0;
    }
    else if (signal > 0) {
        // Since our child is pid 1 we have to kill on delivery on uncaught
        // signal in favour of kernel.
        uint64_t caughtSignals =
                procfs::readProcFS(tracee.getPid(), procfs::Field::SIG_CGT);
        caughtSignals |= IGNORED_SIGNALS;
        if ((caughtSignals & (1 << signal)) == 0U) {
            outputBuilder_->setKillSignal(signal);
            logger::debug(
                    "Delivery of uncaught signal ", signal, " killing instead");
            signal = SIGKILL;
            action = TraceAction::KILL;
        }
    }

    if (signal != event.executeEvent.signal && signal > 0) {
        logger::debug("Handling tracee with signal injection ", signal);
    }
    else {
        logger::debug("Handling tracee with signal ", signal);
    }

    return {action, signal};
}

void TraceExecutor::continueTracee(
        TraceAction action,
        int injectedSignal,
        const TraceEvent& /* event */,
        Tracee& tracee) {
    try {
        if (action == TraceAction::KILL) {
            // Kill _root_ tracee
            outputBuilder_->setKillSignal(SIGKILL);
            logger::debug("Killing root tracee after trace action kill");
            withErrnoCheck("kill root child", kill, rootTraceePid_, SIGKILL);

            // Kill tracee _before_ restarting it
            if (rootTraceePid_ != tracee.getPid()) {
                logger::debug("Killing tracee before restarting it");
                withErrnoCheck("kill child", kill, tracee.getPid(), SIGKILL);
            }
        }

        withErrnoCheck(
                "ptrace cont",
                ptrace,
                PTRACE_CONT,
                tracee.getPid(),
                nullptr,
                injectedSignal);
    }
    catch (const SystemException& ex) {
        if (ex.getErrno() != ESRCH) {
            throw;
        }
    }
}

} // namespace tracer
} // namespace s2j

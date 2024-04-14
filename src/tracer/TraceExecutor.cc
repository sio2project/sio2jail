#include "TraceExecutor.h"

#include "common/ProcFS.h"
#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <csignal>
#include <cstdint>

namespace s2j {
namespace tracer {

const Feature TraceExecutor::feature = Feature::PTRACE;

const uint64_t TraceExecutor::PTRACE_OPTIONS =
        PTRACE_O_EXITKILL | PTRACE_O_TRACESECCOMP | PTRACE_O_TRACEEXEC |
        PTRACE_O_TRACECLONE;

void TraceExecutor::onPostForkChild() {
    TRACE();

    // Let's be a tracee.
    withErrnoCheck(
            "ptrace traceme", ptrace, PTRACE_TRACEME, 0, nullptr, nullptr);
    kill(getpid(), SIGTRAP);
}

void TraceExecutor::onPostForkParent(pid_t childPid) {
    TRACE();

    rootTraceeInfo_ = ProcessInfo::makeProcessInfo(childPid, nullptr);

    // Wait for tracee to call PTRACE_TRACEME
    withErrnoCheck(
            "initial wait", waitpid, rootTraceeInfo_->getPid(), nullptr, 0);

    // Let's be a tracer.
    withErrnoCheck(
            "ptrace setopts",
            ptrace,
            PTRACE_SETOPTIONS,
            rootTraceeInfo_->getPid(),
            nullptr,
            PTRACE_OPTIONS);

    // Resume tracee.
    withErrnoCheck(
            "ptrace cont",
            ptrace,
            PTRACE_CONT,
            rootTraceeInfo_->getPid(),
            nullptr,
            nullptr);
}

executor::ExecuteAction TraceExecutor::onExecuteEvent(
        const executor::ExecuteEvent& executeEvent) {
    TRACE();

    TraceEvent event{executeEvent};
    auto traceeInfo = rootTraceeInfo_->getProcess(event.executeEvent.pid);
    if (traceeInfo == nullptr) {
        logger::debug(
                "Got event for process ",
                VAR(executeEvent.pid),
                " which is not yet present in info tree, delaying");

        // Wait to clear child's status so that future
        // wait calls will not return it.
        withErrnoCheck(
                "waitid for stopped child",
                {ECHILD},
                waitid,
                P_PID,
                event.executeEvent.pid,
                nullptr,
                WEXITED | WSTOPPED | WNOHANG);

        return executor::ExecuteAction::CONTINUE;
    }

    Tracee tracee{traceeInfo};

    TraceAction action = TraceAction::CONTINUE;
    if (!hasExecved_ && executeEvent.trapped &&
        executeEvent.signal == (SIGTRAP | (PTRACE_EVENT_EXEC << 8))) {
        action = std::max(action, onEventExec(event, tracee));
    }

    if (executeEvent.signal == (SIGTRAP | (PTRACE_EVENT_CLONE << 8))) {
        action = std::max(action, onEventClone(event, tracee));
    }

    if (executeEvent.exited || executeEvent.killed) {
        auto parent = traceeInfo->getParent();
        if (parent != nullptr) {
            parent->delChild(traceeInfo->getPid());
        }
        return executor::ExecuteAction::CONTINUE;
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

TraceAction TraceExecutor::onEventClone(
        const TraceEvent& event,
        Tracee& tracee) {
    pid_t traceeChildPid{-1};
    withErrnoCheck(
            "ptrace get child pid",
            ptrace,
            PTRACE_GETEVENTMSG,
            event.executeEvent.pid,
            nullptr,
            &traceeChildPid);
    logger::debug(VAR(event.executeEvent.pid), VAR(traceeChildPid));

    withErrnoCheck(
            "ptrace setopts child",
            ptrace,
            PTRACE_SETOPTIONS,
            traceeChildPid,
            nullptr,
            PTRACE_OPTIONS);

    auto traceeChildInfo = tracee.getInfo()->addChild(traceeChildPid);
    Tracee traceeChild{traceeChildInfo};

    TraceAction action = TraceAction::CONTINUE;
    TraceAction childAction = TraceAction::CONTINUE;
    for (auto& listener: eventListeners_) {
        auto onCloneResult = listener->onPostClone(event, tracee, traceeChild);
        action = std::max(action, std::get<0>(onCloneResult));
        childAction = std::max(childAction, std::get<1>(onCloneResult));
    }

    int injectedSignal = 0;
    if (childAction == TraceAction::KILL) {
        injectedSignal = SIGKILL;

        logger::debug("Killing tracee child before restarting it");
        withErrnoCheck("kill child", kill, traceeChildPid, SIGKILL);
    }

    withErrnoCheck(
            "ptrace cont child",
            {ESRCH},
            ptrace,
            PTRACE_CONT,
            traceeChildPid,
            nullptr,
            injectedSignal);

    return action;
}

std::tuple<TraceAction, int> TraceExecutor::handleTraceeSignal(
        const TraceEvent& event,
        Tracee& tracee) {
    static const uint64_t IGNORED_SIGNALS = (1 << SIGCHLD) | (1 << SIGCLD) |
                                            (1 << SIGURG) | (1 << SIGWINCH) |
                                            (1 << SIGSTOP) | (1 << SIGCONT);

    TraceAction action = TraceAction::CONTINUE;

    int signal = event.executeEvent.signal & 0xff;
    if (event.executeEvent.trapped && signal == SIGTRAP) {
        signal = 0;
    }
    else if (signal > 0) {
        // Since our child is pid 1 we have to kill on delivery on uncaught
        // signal in favour of kernel.
        // Mask is shifted by one because the lowest bit of SigCgt mask
        // corresponds to signal 1, not 0.
        uint64_t caughtSignals =
                procfs::readProcFS(tracee.getPid(), procfs::Field::SIG_CGT)
                << 1;
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
    if (action == TraceAction::KILL) {
        // Kill _root_ tracee
        outputBuilder_->setKillSignal(SIGKILL);
        logger::debug("Killing root tracee after trace action kill");
        withErrnoCheck(
                "kill root child",
                {ESRCH},
                kill,
                rootTraceeInfo_->getPid(),
                SIGKILL);

        // Kill tracee _before_ restarting it
        if (rootTraceeInfo_->getPid() != tracee.getPid()) {
            logger::debug("Killing tracee before restarting it");
            withErrnoCheck(
                    "kill current tracee (a child)",
                    {ESRCH},
                    kill,
                    tracee.getPid(),
                    SIGKILL);
        }
    }

    withErrnoCheck(
            "ptrace cont",
            {ESRCH},
            ptrace,
            PTRACE_CONT,
            tracee.getPid(),
            nullptr,
            injectedSignal);
}

} // namespace tracer
} // namespace s2j

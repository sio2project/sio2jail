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
                "Got event for new process ",
                VAR(event.executeEvent.pid),
                " which is not yet present in info tree");

        auto stoppedPostCloneParent =
                stoppedPostCloneParentsByChild_.find(event.executeEvent.pid);
        if (stoppedPostCloneParent == stoppedPostCloneParentsByChild_.end()) {
            logger::debug("Parent not stopped yet, delaying");

            stoppedPostCloneChildren_.insert(event.executeEvent.pid);

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
        else {
            logger::debug("Parent already stopped, executing");
            tracer::TraceAction action =
                    onClone(stoppedPostCloneParent->second,
                            stoppedPostCloneParent->first);
            stoppedPostCloneParentsByChild_.erase(stoppedPostCloneParent);
            return asExecuteAction(action);
        }
    }

    Tracee tracee{traceeInfo};
    TraceAction action = TraceAction::CONTINUE;

    if (!hasExecved_ && executeEvent.trapped &&
        executeEvent.signal == (SIGTRAP | (PTRACE_EVENT_EXEC << 8))) {
        action = onEventExec(event, tracee);
    }
    else if (executeEvent.signal == (SIGTRAP | (PTRACE_EVENT_CLONE << 8))) {
        action = onEventClone(event, tracee);
    }
    else if (executeEvent.exited || executeEvent.killed) {
        action = onEventExit(event, tracee);
    }
    else {
        action = onRegularTrace(event, tracee);
    }
    return asExecuteAction(action);
}

TraceAction TraceExecutor::onEventExec(
        const TraceEvent& event,
        Tracee& tracee) {
    TraceAction action = TraceAction::CONTINUE;

    hasExecved_ = true;
    for (auto& listener: eventListeners_) {
        action = std::max(action, listener->onPostExec(event, tracee));
    }
    continueTracee(action, 0, tracee.getPid());
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
    logger::debug(VAR(event.executeEvent.pid), " ", VAR(traceeChildPid));

    auto stoppedPostCloneChild = stoppedPostCloneChildren_.find(traceeChildPid);
    if (stoppedPostCloneChild == stoppedPostCloneChildren_.end()) {
        logger::debug("Child not stopped yet, delaying");

        stoppedPostCloneParentsByChild_[traceeChildPid] = tracee.getPid();

        // Wait to clear parent's status so that future
        // wait calls will not return it.
        withErrnoCheck(
                "waitid for stopped parent",
                {ECHILD},
                waitid,
                P_PID,
                tracee.getPid(),
                nullptr,
                WEXITED | WSTOPPED | WNOHANG);

        return TraceAction::CONTINUE;
    }
    else {
        logger::debug("Child already stopped, executing");
        stoppedPostCloneChildren_.erase(stoppedPostCloneChild);
        return onClone(tracee.getPid(), traceeChildPid);
    }
}

TraceAction TraceExecutor::onEventExit(
        const TraceEvent& event,
        Tracee& tracee) {
    auto parent = tracee.getInfo()->getParent();
    if (parent != nullptr) {
        parent->delChild(tracee.getInfo()->getPid());
    }
    return TraceAction::CONTINUE;
}

TraceAction TraceExecutor::onRegularTrace(
        const TraceEvent& event,
        Tracee& tracee) {
    TraceAction action = TraceAction::CONTINUE;
    for (auto& listener: eventListeners_) {
        action = std::max(action, listener->onTraceEvent(event, tracee));
    }

    if (!event.executeEvent.exited && !event.executeEvent.killed &&
        tracee.isAlive()) {
        auto handleSignalResult = handleTraceeSignal(event, tracee);
        action = std::max(action, std::get<0>(handleSignalResult));

        continueTracee(
                action, std::get<1>(handleSignalResult), tracee.getPid());
    }
    return action;
}

TraceAction TraceExecutor::onClone(pid_t parentPid, pid_t childPid) {
    TraceAction action = TraceAction::CONTINUE;
    for (auto& listener: eventListeners_) {
        action = std::max(action, listener->onPostClone(parentPid, childPid));
    }

    withErrnoCheck(
            "ptrace setopts child",
            ptrace,
            PTRACE_SETOPTIONS,
            childPid,
            nullptr,
            PTRACE_OPTIONS);

    if (action == tracer::TraceAction::KILL) {
        logger::debug("Killing tracee parent and child");
        continueTracee(action, SIGKILL, childPid);
        continueTracee(action, SIGKILL, parentPid);
        return action;
    }

    auto parentInfo = rootTraceeInfo_->getProcess(parentPid);
    parentInfo->addChild(childPid);

    continueTracee(action, 0, childPid);
    continueTracee(action, 0, parentPid);
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
        pid_t traceePid) {
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
        if (rootTraceeInfo_->getPid() != traceePid) {
            logger::debug("Killing tracee before restarting it");
            withErrnoCheck(
                    "kill current tracee (a child)",
                    {ESRCH},
                    kill,
                    traceePid,
                    SIGKILL);
        }
    }

    withErrnoCheck(
            "ptrace cont",
            {ESRCH},
            ptrace,
            PTRACE_CONT,
            traceePid,
            nullptr,
            injectedSignal);
}

} // namespace tracer
} // namespace s2j

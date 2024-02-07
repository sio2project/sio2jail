#include "ThreadsLimitListener.h"

#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"
#include "seccomp/SeccompRule.h"
#include "seccomp/action/ActionAllow.h"
#include "seccomp/action/ActionKill.h"
#include "seccomp/filter/LibSeccompFilter.h"

#include <cstdint>

namespace s2j {
namespace limits {

ThreadsLimitListener::ThreadsLimitListener(int32_t threadsLimit)
        : threadsLimit_{threadsLimit} {
    TRACE(threadsLimit);

    syscallRules_.emplace_back(
            seccomp::SeccompRule("fork", seccomp::action::ActionKill{}));
    syscallRules_.emplace_back(
            seccomp::SeccompRule("vfork", seccomp::action::ActionKill{}));

    if (threadsLimit_ < 0) {
        // Disable threads support
        syscallRules_.emplace_back(
                seccomp::SeccompRule("clone", seccomp::action::ActionKill{}));
        syscallRules_.emplace_back(
                seccomp::SeccompRule("clone3", seccomp::action::ActionKill{})));
    }
    else {
        // Enable threads support
        using Arg = seccomp::filter::SyscallArg;
        syscallRules_.emplace_back(seccomp::SeccompRule(
                "clone",
                seccomp::action::ActionAllow{},
                (Arg(2) & CLONE_VM) == CLONE_VM));
        syscallRules_.emplace_back(seccomp::SeccompRule(
                "clone3",
                seccomp::action::ActionAllow{},
                (Arg(2) & CLONE_VM) == CLONE_VM));

        syscallRules_.emplace_back(seccomp::SeccompRule(
                "clone",
                seccomp::action::ActionKill(),
                (Arg(2) & CLONE_VM) == 0));
        syscallRules_.emplace_back(seccomp::SeccompRule(
                "clone3",
                seccomp::action::ActionKill(),
                (Arg(2) & CLONE_VM) == 0));

        // And various thread related
        syscallRules_.emplace_back(seccomp::SeccompRule(
                // TODO: allow sleep up to time limit
                "nanosleep",
                seccomp::action::ActionAllow{}));
    }
}

std::tuple<tracer::TraceAction, tracer::TraceAction>
ThreadsLimitListener::onPostClone(
        const tracer::TraceEvent& traceEvent,
        tracer::Tracee& tracee,
        tracer::Tracee& traceeChild) {
    TRACE(tracee.getPid(), traceeChild.getPid());
    if (threadsLimit_ < 0) {
        outputBuilder_->setKillReason(
                printer::OutputBuilder::KillReason::RV,
                "Threads are not allowed");
        return {tracer::TraceAction::KILL, tracer::TraceAction::KILL};
    }

    threadsPids_.insert(traceeChild.getPid());
    logger::debug(
            "Thread ",
            traceeChild.getPid(),
            " started, new thread count ",
            threadsPids_.size());
    if (threadsPids_.size() > static_cast<uint32_t>(threadsLimit_)) {
        outputBuilder_->setKillReason(
                printer::OutputBuilder::KillReason::RV,
                "threads limit exceeded");
        logger::info(
                "Threads limit ", threadsLimit_, " exceeded, killing tracee");
        return {tracer::TraceAction::KILL, tracer::TraceAction::KILL};
    }

    return {tracer::TraceAction::CONTINUE, tracer::TraceAction::CONTINUE};
}

executor::ExecuteAction ThreadsLimitListener::onExecuteEvent(
        const executor::ExecuteEvent& executeEvent) {
    TRACE(executeEvent.pid);
    if (threadsLimit_ < 0)
        return executor::ExecuteAction::CONTINUE;

    if (executeEvent.exited || executeEvent.killed) {
        threadsPids_.erase(executeEvent.pid);
        logger::debug(
                "Thread ",
                executeEvent.pid,
                " exited, new threads count ",
                threadsPids_.size());
    }
    return executor::ExecuteAction::CONTINUE;
}

} // namespace limits
} // namespace s2j

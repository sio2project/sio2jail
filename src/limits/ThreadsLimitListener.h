#pragma once

#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"
#include "seccomp/policy/SyscallPolicy.h"
#include "tracer/TraceEventListener.h"

#include <unordered_set>

namespace s2j {
namespace limits {

class ThreadsLimitListener
        : public executor::ExecuteEventListener
        , public tracer::TraceEventListener
        , public printer::OutputSource
        , public seccomp::policy::SyscallPolicy {
public:
    ThreadsLimitListener(int32_t threadsLimit);

    tracer::TraceAction onPostClone(pid_t traceePid, pid_t childPid) override;

    executor::ExecuteAction onExecuteEvent(
            const executor::ExecuteEvent& executeEvent) override;

    const std::vector<seccomp::SeccompRule>& getRules() const override {
        return syscallRules_;
    }

private:
    std::unordered_set<pid_t> threadsPids_;
    std::vector<seccomp::SeccompRule> syscallRules_;
    const int32_t threadsLimit_;
};

} // namespace limits
} // namespace s2j

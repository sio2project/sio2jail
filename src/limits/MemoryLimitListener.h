#pragma once

#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"
#include "seccomp/policy/SyscallPolicy.h"
#include "tracer/TraceEventListener.h"

#include <sys/mman.h>

#ifndef MREMAP_DONTUNMAP
//Due to this flag being introduced in Linux 5.7 there are many system which do not define it
#define MREMAP_DONTUNMAP 4
#endif

#include <cstdint>

namespace s2j {
namespace limits {

class MemoryLimitListener
        : public executor::ExecuteEventListener
        , public tracer::TraceEventListener
        , public printer::OutputSource
        , public seccomp::policy::SyscallPolicy {
public:
    MemoryLimitListener(uint64_t memoryLimitKb);

    void onPostForkChild() override;
    void onPostForkParent(pid_t childPid) override;
    tracer::TraceAction onPostExec(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee) override;
    executor::ExecuteAction onExecuteEvent(
            const executor::ExecuteEvent& executeEvent) override;

    const std::vector<seccomp::SeccompRule>& getRules() const;

private:
    static const uint64_t MEMORY_LIMIT_MARGIN;

    uint64_t getMemoryPeakKb();
    uint64_t getMemoryUsageKb();

    uint64_t memoryPeakKb_;
    uint64_t memoryLimitKb_;
    bool vmPeakValid_;
    pid_t childPid_;

    std::vector<seccomp::SeccompRule> syscallRules_;
    tracer::TraceAction handleMemoryAllocation(uint64_t allocatedMemoryKb);
};

} // namespace limits
} // namespace s2j

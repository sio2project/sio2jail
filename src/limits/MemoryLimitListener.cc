#include "MemoryLimitListener.h"

#include "common/ProcFS.h"
#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"
#include "seccomp/SeccompRule.h"
#include "seccomp/action/ActionAllow.h"
#include "seccomp/action/ActionTrace.h"
#include "seccomp/filter/LibSeccompFilter.h"

#include <sys/resource.h>
#include <sys/time.h>

#include <csignal>
#include <cstdint>
#include <fstream>
#include <iostream>

namespace s2j {
namespace limits {

const uint64_t MemoryLimitListener::MEMORY_LIMIT_MARGIN = 8 * 1024 * 1024;

MemoryLimitListener::MemoryLimitListener(uint64_t memoryLimitKb)
        : memoryPeakKb_(0)
        , memoryLimitKb_(memoryLimitKb)
        , vmPeakValid_(false)
        , childPid_(-1) {
    TRACE(memoryLimitKb);

    // Possible memory problem here, we will return this references to *this.
    // User is responsible for ensuring that MemoryLimitListener last at least
    // as long as any reference to it's rules.
    using Arg = seccomp::filter::SyscallArg;
    for (const auto& syscall: {"mmap2", "mmap"}) {
        syscallRules_.emplace_back(seccomp::SeccompRule(
                syscall,
                seccomp::action::ActionTrace([this](tracer::Tracee& tracee) {
                    TRACE();
                    if (!vmPeakValid_) {
                        return tracer::TraceAction::CONTINUE;
                    }
                    return handleMemoryAllocation(tracee.getSyscallArgument(1) / 1024);
                }),
                Arg(0) == 0 && Arg(1) > MEMORY_LIMIT_MARGIN / 2));
    }
    syscallRules_.emplace_back(seccomp::SeccompRule(
            "mremap",
            seccomp::action::ActionTrace([this](tracer::Tracee& tracee){
                TRACE();
                if (!vmPeakValid_) {
                    return tracer::TraceAction::CONTINUE;
                }
                bool doesUnmap = (tracee.getSyscallArgument(3)&MREMAP_DONTUNMAP)!=0;
                // Allow user to shrink its memory
                if(!doesUnmap&&tracee.getSyscallArgument(1)>=tracee.getSyscallArgument(2)){
                    return tracer::TraceAction::CONTINUE;
                }
                uint64_t newMemoryAllocated=tracee.getSyscallArgument(2);
                // Do not count already allocated memory
                if(!doesUnmap) newMemoryAllocated-=tracee.getSyscallArgument(1);
                newMemoryAllocated/=1024;
                return handleMemoryAllocation(newMemoryAllocated);
            }),
            Arg(2)>MEMORY_LIMIT_MARGIN/2));
}
tracer::TraceAction MemoryLimitListener::handleMemoryAllocation(uint64_t allocatedMemoryKb){
    uint64_t memoryUsage = getMemoryUsageKb() + allocatedMemoryKb;
    memoryPeakKb_ = std::max(memoryPeakKb_, memoryUsage);
    outputBuilder_->setMemoryPeak(memoryPeakKb_);
    logger::debug(
            "Memory usage after allocation ",
            VAR(memoryUsage),
            ", ",
            VAR(memoryPeakKb_));

    if (memoryUsage > memoryLimitKb_) {
        outputBuilder_->setKillReason(
                printer::OutputBuilder::KillReason::MLE,
                "memory limit exceeded");
        logger::debug(
                "Limit ",
                VAR(memoryLimitKb_),
                " exceeded, killing tracee");
        return tracer::TraceAction::KILL;
    }
    return tracer::TraceAction::CONTINUE;
}
void MemoryLimitListener::onPostForkChild() {
    TRACE();

    // If there is any memory limit, set it.
    if (memoryLimitKb_ > 0) {
        struct rlimit memoryLimit {};

        memoryLimit.rlim_cur = memoryLimit.rlim_max =
                memoryLimitKb_ * 1024 + MEMORY_LIMIT_MARGIN;
        logger::debug("Seting address space limit ", VAR(memoryLimit.rlim_max));
        withErrnoCheck(
                "setrlimit address space", setrlimit, RLIMIT_AS, &memoryLimit);

        memoryLimit.rlim_cur = memoryLimit.rlim_max = RLIM_INFINITY;
        logger::debug("Seting stack limit to infinity");
        withErrnoCheck(
                "setrlimit stack", setrlimit, RLIMIT_STACK, &memoryLimit);
    }
}

void MemoryLimitListener::onPostForkParent(pid_t childPid) {
    TRACE(childPid);

    childPid_ = childPid;
}

tracer::TraceAction MemoryLimitListener::onPostExec(
        const tracer::TraceEvent& /* traceEvent */,
        tracer::Tracee& /* tracee */) {
    vmPeakValid_ = true;
    return tracer::TraceAction::CONTINUE;
}

executor::ExecuteAction MemoryLimitListener::onExecuteEvent(
        const executor::ExecuteEvent& /*executeEvent*/) {
    TRACE();

    if (!vmPeakValid_) {
        return executor::ExecuteAction::CONTINUE;
    }

    memoryPeakKb_ = std::max(memoryPeakKb_, getMemoryPeakKb());
    logger::debug("Read new memory peak ", VAR(memoryPeakKb_));

    outputBuilder_->setMemoryPeak(memoryPeakKb_);
    if (memoryLimitKb_ > 0 && memoryPeakKb_ > memoryLimitKb_) {
        outputBuilder_->setKillReason(
                printer::OutputBuilder::KillReason::MLE,
                "memory limit exceeded");
        logger::debug(
                "Limit ", VAR(memoryLimitKb_), " exceeded, killing tracee");
        return executor::ExecuteAction::KILL;
    }

    return executor::ExecuteAction::CONTINUE;
}

uint64_t MemoryLimitListener::getMemoryPeakKb() {
    return procfs::readProcFS(childPid_, procfs::Field::VM_PEAK);
}

uint64_t MemoryLimitListener::getMemoryUsageKb() {
    return procfs::readProcFS(childPid_, procfs::Field::VM_SIZE);
}

const std::vector<seccomp::SeccompRule>& MemoryLimitListener::getRules() const {
    return syscallRules_;
}

} // namespace limits
} // namespace s2j

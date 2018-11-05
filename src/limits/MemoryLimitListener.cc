#include "MemoryLimitListener.h"

#include "common/WithErrnoCheck.h"
#include "common/ProcFS.h"
#include "logger/Logger.h"
#include "seccomp/SeccompRule.h"
#include "seccomp/action/ActionTrace.h"
#include "seccomp/action/ActionAllow.h"
#include "seccomp/filter/LibSeccompFilter.h"

#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

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

    // Possible memory problem here, we will return this references to *this. User is
    // responsible for ensuring that MemoryLimitListener last at least as long as any
    // reference to it's rules.
    using Arg = seccomp::filter::SyscallArg;
    for (const auto& syscall: {"mmap2", "mmap"}) {
        syscallRules_.emplace_back(seccomp::SeccompRule(
                    syscall,
                    seccomp::action::ActionTrace(
                        [this](tracer::Tracee& tracee) {
                            TRACE();
                            if (!vmPeakValid_)
                                return tracer::TraceAction::CONTINUE;

                            uint64_t memoryUsage = getMemoryUsageKb() + tracee.getSyscallArgument(1) / 1024;
                            memoryPeakKb_ = std::max(memoryPeakKb_, memoryUsage);
                            outputBuilder_->setMemoryPeak(memoryPeakKb_);
                            logger::debug("Memory usage after mmap ", VAR(memoryUsage), ", ", VAR(memoryPeakKb_));

                            if (memoryUsage > memoryLimitKb_) {
                                outputBuilder_->setKillReason(printer::OutputBuilder::KillReason::MLE, "memory limit exceeded");
                                logger::debug("Limit ", VAR(memoryLimitKb_), " exceeded, killing tracee");
                                return tracer::TraceAction::KILL;
                            }
                            return tracer::TraceAction::CONTINUE;
                        }),
                    Arg(0) == 0 && Arg(1) > MEMORY_LIMIT_MARGIN / 2));
    }
}

void MemoryLimitListener::onPostForkChild() {
    TRACE();

    // If there is any memory limit, set it.
    if (memoryLimitKb_ > 0) {
        struct rlimit memoryLimit{memoryLimitKb_ * 1024 + MEMORY_LIMIT_MARGIN,
                                  memoryLimitKb_ * 1024 + MEMORY_LIMIT_MARGIN};

        logger::debug("Seting limit ", VAR(memoryLimit.rlim_max));
        withErrnoCheck("setrlimit memory", setrlimit, RLIMIT_AS, &memoryLimit);
        withErrnoCheck("setrlimit stack", setrlimit, RLIMIT_STACK, &memoryLimit);
    }
}

void MemoryLimitListener::onPostForkParent(pid_t childPid) {
    TRACE(childPid);

    childPid_ = childPid;
}

void MemoryLimitListener::onPostExec(const tracer::TraceEvent& /* traceEvent */) {
    vmPeakValid_ = true;
}

executor::ExecuteAction MemoryLimitListener::onExecuteEvent(const executor::ExecuteEvent& executeEvent) {
    TRACE();

    if (!vmPeakValid_) {
        // This case happens when process has bigger .bss section than ulimit virtual memory limit.
        // It gets killed by SIGSEGV before PTRACE_EVENT_EXEC, and values in procfs aren't bigger than limit.
        if ((executeEvent.killed || executeEvent.trapped) && executeEvent.signal == SIGSEGV) {
            logger::debug("Process has been killed by SIGSEGV before exec event, assuming memory limit exceeded");
            outputBuilder_->setKillReason(printer::OutputBuilder::KillReason::MLE,
                    "process exited due to signal 11 before exec event, assuming memory limit exceeded");
            return executor::ExecuteAction::KILL;
        }
        return executor::ExecuteAction::CONTINUE;
    }

    memoryPeakKb_ = std::max(memoryPeakKb_, getMemoryPeakKb());
    logger::debug("Read new memory peak ", VAR(memoryPeakKb_));

    outputBuilder_->setMemoryPeak(memoryPeakKb_);
    if (memoryLimitKb_ > 0 && memoryPeakKb_ > memoryLimitKb_) {
        outputBuilder_->setKillReason(printer::OutputBuilder::KillReason::MLE, "memory limit exceeded");
        logger::debug("Limit ", VAR(memoryLimitKb_), " exceeded, killing tracee");
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

}
}

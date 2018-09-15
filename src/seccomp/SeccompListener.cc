#include "SeccompListener.h"
#include "action/ActionAllow.h"
#include "action/ActionTrace.h"

#include "common/Exception.h"
#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <seccomp.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ptrace.h>

#include <algorithm>

namespace s2j {
namespace seccomp {

const Feature SeccompListener::feature = Feature::SECCOMP;
const uint32_t SeccompListener::TRACE_EVENT_ID_BASE = 0;

SeccompListener::SeccompListener()
    : basePolicy_(std::make_unique<policy::DefaultPolicy>())
    , lastSyscallArch_(tracer::Arch::X86) {}

void SeccompListener::onPreFork() {
    TRACE();

    // Now current syscalls are frozen, and we can add default ones from syscall policy.
    addPolicy(*basePolicy_);

    // Id, that will be used to map any trace event from libseccomp to rules set from SeccompListener.
    uint32_t ruleId = TRACE_EVENT_ID_BASE;

    // Create context builder
    SeccompContext::Builder contextBuilder;

    // Add rules in order
    for (auto ruleIter = rules_.begin(); ruleIter != rules_.end(); ++ruleIter) {
        /**
         * Sort rules by their action precedence: kill > errno > trace > allow. Libseccomp
         * includes rules in bpf program in opposid order than they where added to context. So
         * if here we will stable_sort them in reverse order, and then add them from end to
         * begin adding order will be preserved.
         */
        std::stable_sort(ruleIter->second.begin(), ruleIter->second.end(),
                         [](const auto& a, const auto& b) {
                            return static_cast<int>(a.action->getType()) > static_cast<int>(b.action->getType());
                         });

        rulesById_[++ruleId] = ruleIter;
        for (auto rule = ruleIter->second.rbegin(); rule != ruleIter->second.rend(); ++rule) {
            contextBuilder.addRule(*rule, ruleId);
        }
    }

    // And finally create a context
    context_ = std::make_unique<SeccompContext>(std::move(contextBuilder));

    try {
        logger::debug("Libseccomp filter:\n", context_->exportFilter());
    }
    catch(const s2j::SystemException &ex) {
        if (ex.getErrno() != ENOSYS)
            throw ex;
        logger::debug("Libseccomp filter: ", ex.what());
    }
}

void SeccompListener::onPostForkChild() {
    TRACE();

    assert(context_ != nullptr, "seccomp filter is configured");
    context_->loadFilter();
}

tracer::TraceAction SeccompListener::onTraceEvent(const tracer::TraceEvent& traceEvent, tracer::Tracee& tracee) {
    TRACE();

    if (!tracee.isAlive()) {
        logger::debug("Tracee already exited, do nothing");
        return tracer::TraceAction::CONTINUE;
    }

    if (!traceEvent.executeEvent.trapped ||
            traceEvent.executeEvent.signal != (SIGTRAP | PTRACE_EVENT_SECCOMP << 8)) {
        logger::debug("Not a seccomp event, do nothing");
        return tracer::TraceAction::CONTINUE;
    }

    uint32_t traceEventMsg = tracee.getEventMsg();

    /* Sadly we can't distinguish architecture on default action,
     * because libseccomp doesn't allow to merge multiple contexts
     * with different default actions. Later, when we create
     * custom seccomp bpf compiler we will change this.
     */
    if (traceEventMsg != 0) {
        lastSyscallArch_ = static_cast<tracer::Arch>(traceEventMsg & ((1 << SeccompContext::SECCOMP_TRACE_MSG_NUM_SHIFT) - 1));
    }
    tracee.setSyscallArch(lastSyscallArch_);

    /* Pretty-print syscall name. */
    std::string syscallName =
        resolveSyscallNumber(tracee.getSyscallNumber(), tracee.getSyscallArch()) +
        "(" + std::to_string(tracee.getSyscallNumber()) + ") (";
    for (size_t i = 0; i < 6; ++i) {
        if (i > 0)
            syscallName += ", ";
        syscallName += std::to_string(tracee.getSyscallArgument(i));
    }
    syscallName += ")";

    logger::debug("Detected syscall architecture ", to_string(lastSyscallArch_), " from ", VAR(traceEventMsg), " for ", syscallName);

    if (traceEventMsg == 0) {
        logger::debug("Default syscall filter action after syscall ", syscallName);
        outputBuilder_->setKillReason(printer::OutputBuilder::KillReason::RV, "intercepted forbidden syscall " + syscallName);
        return tracer::TraceAction::KILL;
    }

    std::shared_ptr<action::SeccompAction> seccompAction = nullptr;
    auto ruleSetIter = rulesById_.find(traceEventMsg >> SeccompContext::SECCOMP_TRACE_MSG_NUM_SHIFT);
    if (ruleSetIter != rulesById_.end()) {
        for (auto& rule: ruleSetIter->second->second) {
            if (rule.filter->match(traceEvent, tracee)) {
                if (seccompAction == nullptr || rule.action->getType() > seccompAction->getType())
                    seccompAction = rule.action;
            }
        }
    }

    assert(seccompAction != nullptr, "trace triggered rule is handled by some rule");

    logger::debug("Returning with action " + to_string(seccompAction->getType()));
    return seccompAction->execute(tracee);
}

void SeccompListener::addPolicy(const policy::SyscallPolicy& policy) {
    TRACE();

    for (auto rule: policy.getRules())
        addRule(rule);
}

std::string SeccompListener::resolveSyscallNumber(uint32_t syscallNumber, const tracer::Arch& arch) {
    if (arch == tracer::Arch::UNKNOWN)
        return std::string();

    char* name = seccomp_syscall_resolve_num_arch(SeccompContext::SECCOMP_FILTER_ARCHITECTURES.at(arch), syscallNumber);
    std::string syscallName(name);
    free(name);
    return syscallName;
}

}
}

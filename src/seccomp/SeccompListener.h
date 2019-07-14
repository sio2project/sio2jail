#pragma once

#include "SeccompContext.h"
#include "SeccompException.h"
#include "SeccompRule.h"
#include "action/SeccompAction.h"
#include "filter/SyscallFilter.h"
#include "policy/DefaultPolicy.h"

#include "common/Assert.h"
#include "common/Feature.h"
#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"
#include "tracer/TraceEventListener.h"
#include "tracer/Tracee.h"

#include <map>
#include <memory>
#include <vector>

namespace s2j {
namespace seccomp {

class SeccompListener
        : public virtual executor::ExecuteEventListener
        , public virtual tracer::TraceEventListener
        , public printer::OutputSource {
public:
    using syscall_t = int;

    SeccompListener();
    SeccompListener(std::shared_ptr<policy::BaseSyscallPolicy> basePolicy);

    /* Create seccomp context and build syscall filter. */
    void onPreFork() override;

    /* Load syscall filter into kernel. */
    void onPostForkChild() override;

    /* Inform any rules about event that have occured. */
    tracer::TraceAction onTraceEvent(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee) override;

    /* Adds new rule to seccomp syscall filter. */
    template<typename SeccompRule>
    void addRule(SeccompRule&& rule) {
        if (context_ != nullptr)
            throw SeccompFilterAlreadyCompiledException();
        rules_[rule.syscall].emplace_back(std::forward<SeccompRule>(rule));
    }

    /* Emebds new policy */
    void addPolicy(const policy::SyscallPolicy& policy);

    const static Feature feature;

private:
    static std::string resolveSyscallNumber(
            uint32_t syscallNumber,
            const tracer::Arch& arch);

    const static uint32_t TRACE_EVENT_ID_BASE;

    std::unique_ptr<SeccompContext> context_;

    std::shared_ptr<policy::BaseSyscallPolicy> basePolicy_;

    std::map<syscall_t, std::vector<SeccompRule>> rules_;
    std::map<uint16_t, decltype(rules_)::iterator> rulesById_;

    tracer::Arch lastSyscallArch_;
};

} // namespace seccomp
} // namespace s2j

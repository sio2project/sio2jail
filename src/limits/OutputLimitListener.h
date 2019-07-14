#pragma once

#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"
#include "seccomp/policy/SyscallPolicy.h"

namespace s2j {
namespace limits {

class OutputLimitListener
        : public executor::ExecuteEventListener
        , public printer::OutputSource {
public:
    OutputLimitListener(uint64_t outputLimitB);

    void onPostForkChild() override;
    void onPostForkParent(pid_t childPid) override;
    executor::ExecuteAction onExecuteEvent(
            const executor::ExecuteEvent& executeEvent) override;

private:
    uint64_t outputLimitB_;
    pid_t childPid_;
};

} // namespace limits
} // namespace s2j

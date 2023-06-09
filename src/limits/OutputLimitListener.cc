#include "OutputLimitListener.h"

#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"
#include "seccomp/SeccompRule.h"
#include "seccomp/action/ActionAllow.h"
#include "seccomp/action/ActionTrace.h"
#include "seccomp/filter/LibSeccompFilter.h"

#include <csignal>
#include <cstdint>
#include <sys/resource.h>
#include <sys/time.h>

#include <fstream>
#include <iostream>

namespace s2j {
namespace limits {

OutputLimitListener::OutputLimitListener(uint64_t outputLimitB)
        : outputLimitB_(outputLimitB), childPid_(-1) {
    TRACE(outputLimitB);
}

void OutputLimitListener::onPostForkChild() {
    TRACE();

    // If there is any output limit, set it.
    if (outputLimitB_ > 0) {
        struct rlimit outputLimit {
            outputLimitB_, outputLimitB_
        };

        logger::debug("Seting limit ", VAR(outputLimit.rlim_max));
        withErrnoCheck(
                "setrlimit file size", setrlimit, RLIMIT_FSIZE, &outputLimit);
    }
}

void OutputLimitListener::onPostForkParent(pid_t childPid) {
    TRACE(childPid);

    childPid_ = childPid;
}

executor::ExecuteAction OutputLimitListener::onExecuteEvent(
        const executor::ExecuteEvent& executeEvent) {
    TRACE();

    if (outputLimitB_ > 0 && executeEvent.signal == SIGXFSZ) {
        logger::info("Tracee got SIGXFSZ, assuming output limit exceeded");
        outputBuilder_->setKillReason(
                printer::OutputBuilder::KillReason::OLE,
                "output limit exceeded");
        return executor::ExecuteAction::KILL;
    }

    return executor::ExecuteAction::CONTINUE;
}

} // namespace limits
} // namespace s2j

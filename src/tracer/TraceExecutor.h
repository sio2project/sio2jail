#pragma once

#include "ProcessInfo.h"
#include "TraceAction.h"
#include "TraceEventListener.h"

#include "common/EventProvider.h"
#include "common/Feature.h"
#include "executor/ExecuteEvent.h"
#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"

#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <unistd.h>

namespace s2j {
namespace tracer {

class TraceExecutor
        : public virtual executor::ExecuteEventListener
        , public printer::OutputSource
        , public EventProvider<TraceEventListener> {
public:
    void onPostForkChild() override;
    void onPostForkParent(pid_t childPid) override;
    executor::ExecuteAction onExecuteEvent(
            const executor::ExecuteEvent& executeEvent) override;

    const static Feature feature;

private:
    TraceAction onEventExec(const TraceEvent& executeEvent, Tracee& tracee);

    TraceAction onEventClone(const TraceEvent& executeEvent, Tracee& tracee);

    executor::ExecuteAction continueChildAfterClone(
            pid_t childPid,
            TraceAction childAction);

    /* Returns action and injectedSignal */
    std::tuple<TraceAction, int> handleTraceeSignal(
            const TraceEvent& event,
            Tracee& tracee);

    void continueTracee(
            TraceAction action,
            int injectedSignal,
            const TraceEvent& event,
            Tracee& tracee);

    static const uint64_t PTRACE_OPTIONS;

    std::shared_ptr<ProcessInfo> rootTraceeInfo_;
    std::unordered_set<pid_t> stoppedPostCloneChildren_;
    std::unordered_map<pid_t, TraceAction> delayedPostCloneChildrenAction_;
    bool hasExecved_{false};
};

} // namespace tracer
} // namespace s2j

#pragma once

#include "TraceAction.h"
#include "TraceEventListener.h"

#include "common/EventProvider.h"
#include "common/Feature.h"
#include "executor/ExecuteEvent.h"
#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"

#include <memory>
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
    bool hasExecved_ = false;
    pid_t traceePid_;
};

} // namespace tracer
} // namespace s2j

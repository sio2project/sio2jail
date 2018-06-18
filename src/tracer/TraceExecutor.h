#pragma once

#include "TraceEventListener.h"
#include "TraceAction.h"

#include "common/Feature.h"
#include "common/EventProvider.h"
#include "printer/OutputSource.h"
#include "executor/ExecuteEventListener.h"
#include "executor/ExecuteEvent.h"

#include <vector>
#include <memory>

#include <unistd.h>

namespace s2j {
namespace tracer {

class TraceExecutor : public virtual executor::ExecuteEventListener
                    , public printer::OutputSource
                    , public EventProvider<TraceEventListener> {
public:
    void onPostForkChild() override;
    void onPostForkParent(pid_t childPid) override;
    executor::ExecuteAction onExecuteEvent(const executor::ExecuteEvent& executeEvent) override;

    const static Feature feature;

private:
    bool hasExecved_ = false;
    pid_t traceePid_;
};

}
}

#pragma once

#include "executor/ExecuteEventListener.h"
#include "tracer/TraceEventListener.h"

#include <ostream>

namespace s2j {
namespace logger {

class LoggerListener
        : public executor::ExecuteEventListener
        , public tracer::TraceEventListener {
public:
    void onPreFork() override;
    void onPostForkChild() override;
    void onPostForkParent(pid_t childPid) override;
    void onPostExec(const tracer::TraceEvent& traceEvent) override;
    executor::ExecuteAction onExecuteEvent(
            const executor::ExecuteEvent& executeEvent) override;
    tracer::TraceAction onTraceEvent(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee) override;
    void onPostExecute() override;
};

} // namespace logger
} // namespace s2j

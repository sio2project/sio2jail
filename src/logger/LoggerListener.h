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
    executor::ExecuteAction onExecuteEvent(
            const executor::ExecuteEvent& executeEvent) override;
    void onPostExecute() override;

    tracer::TraceAction onPostExec(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee) override;
    std::tuple<tracer::TraceAction, tracer::TraceAction> onPostClone(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee,
            pid_t traceeChildPid) override;
    tracer::TraceAction onTraceEvent(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee) override;
};

} // namespace logger
} // namespace s2j

#pragma once

#include "TraceAction.h"
#include "TraceEvent.h"

#include "executor/ExecuteEventListener.h"

#include <tuple>

namespace s2j {
namespace tracer {

class TraceEventListener {
public:
    virtual ~TraceEventListener() = default;

    /**
     * Triggers in parent after child's initial execve.
     */
    virtual TraceAction onPostExec(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee) {
        return TraceAction::CONTINUE;
    }

    /**
     * Triggers in parent after each clone. Returns pair tracee action,
     * tracee child action.
     */
    virtual std::tuple<TraceAction, TraceAction> onPostClone(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee,
            tracer::Tracee& traceeChild) {
        return {TraceAction::CONTINUE, TraceAction::CONTINUE};
    }

    /**
     * Triggers in parent after each event
     */
    virtual TraceAction onTraceEvent(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee) {
        return TraceAction::CONTINUE;
    }
};

} // namespace tracer
} // namespace s2j

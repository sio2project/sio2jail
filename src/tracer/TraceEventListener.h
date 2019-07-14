#pragma once

#include "TraceAction.h"
#include "TraceEvent.h"

#include "executor/ExecuteEventListener.h"

namespace s2j {
namespace tracer {

class TraceEventListener {
public:
    virtual ~TraceEventListener() = default;

    /**
     * Triggers in parent after child's initial execve.
     */
    virtual void onPostExec(const tracer::TraceEvent& traceEvent) {}

    virtual TraceAction onTraceEvent(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee) {
        return TraceAction::CONTINUE;
    }
};

} // namespace tracer
} // namespace s2j

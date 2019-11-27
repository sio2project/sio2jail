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
     * Triggers in parent after each clone before paren and new child are
     * resumed.
     */
    virtual TraceAction onPostClone(pid_t traceePid, pid_t traceeChildPid) {
        return TraceAction::CONTINUE;
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

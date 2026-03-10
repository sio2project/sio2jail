#pragma once

#include "common/Feature.h"
#include "executor/ExecuteEventListener.h"
#include "tracer/TraceEventListener.h"

namespace s2j {
namespace seccomp {

class TimeRandomizerListener : public executor::ExecuteEventListener,
                                public tracer::TraceEventListener {
public:
    explicit TimeRandomizerListener(bool returnZero = false);

    void onPostForkChild() override;

    tracer::TraceAction onTraceEvent(
            const tracer::TraceEvent& traceEvent,
            tracer::Tracee& tracee) override;

    const static Feature feature;

private:
    bool returnZero_;
};

} // namespace seccomp
} // namespace s2j

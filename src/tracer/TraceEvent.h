#pragma once

#include "Tracee.h"

#include "executor/ExecuteEvent.h"

namespace s2j {
namespace tracer {

struct TraceEvent {
    // TODO: use reference here to avoid copying
    executor::ExecuteEvent executeEvent;
};

} // namespace tracer
} // namespace s2j

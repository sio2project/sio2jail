#pragma once

#include "common/Exception.h"
#include "executor/ExecuteAction.h"

namespace s2j {
namespace tracer {

enum class TraceAction { CONTINUE = 0, KILL = 1 };

} // namespace tracer

constexpr executor::ExecuteAction asExecuteAction(
        const tracer::TraceAction traceAction) {
    switch (traceAction) {
    case tracer::TraceAction::CONTINUE:
        return executor::ExecuteAction::CONTINUE;
    case tracer::TraceAction::KILL:
        return executor::ExecuteAction::KILL;
    }

    throw Exception("Unreachable code");
}

} // namespace s2j

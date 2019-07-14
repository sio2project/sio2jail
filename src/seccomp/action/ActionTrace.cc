#include "ActionTrace.h"

#include <iostream>

namespace s2j {
namespace seccomp {
namespace action {

SeccompAction::Type ActionTrace::getType() const {
    return SeccompAction::Type::TRACE;
}

tracer::TraceAction ActionTrace::execute(tracer::Tracee& tracee) {
    return handler_(tracee);
}

uint32_t ActionTrace::createLibSeccompAction() const {
    return SCMP_ACT_TRACE(getRuleId());
}

} // namespace action
} // namespace seccomp
} // namespace s2j

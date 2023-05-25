#include "ActionKill.h"

#include <cstdint>

namespace s2j {
namespace seccomp {
namespace action {

SeccompAction::Type ActionKill::getType() const {
    return SeccompAction::Type::KILL;
}

tracer::TraceAction ActionKill::execute(tracer::Tracee& /*tracee*/) {
    /**
     * Do not kill explicitly, just return TraceAction::KILL. Child will be
     * sacrificed by Executor.
     */
    return tracer::TraceAction::KILL;
}

uint32_t ActionKill::createLibSeccompAction() const {
    // Trace here so that executor will display nice output. Perfomance doesn't
    // matter here since we are gpoing to finish anyway.
    return SCMP_ACT_TRACE(getRuleId());
}

} // namespace action
} // namespace seccomp
} // namespace s2j

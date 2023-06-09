#include "ActionAllow.h"

#include <cstdint>

namespace s2j {
namespace seccomp {
namespace action {

SeccompAction::Type ActionAllow::getType() const {
    return SeccompAction::Type::ALLOW;
}

tracer::TraceAction ActionAllow::execute(tracer::Tracee& /*tracee*/) {
    return tracer::TraceAction::CONTINUE;
}

uint32_t ActionAllow::createLibSeccompAction() const {
    return SCMP_ACT_ALLOW;
}

} // namespace action
} // namespace seccomp
} // namespace s2j

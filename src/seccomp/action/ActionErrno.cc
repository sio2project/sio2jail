#include "ActionErrno.h"

#include "common/Exception.h"

namespace s2j {
namespace seccomp {
namespace action {

ActionErrno::ActionErrno(uint64_t errnoNumber) : errnoNumber_(errnoNumber) {}

SeccompAction::Type ActionErrno::getType() const {
    return SeccompAction::Type::ERRNO;
}

tracer::TraceAction ActionErrno::execute(tracer::Tracee& tracee) {
    tracee.cancelSyscall(-errnoNumber_);
    return tracer::TraceAction::CONTINUE;
}

uint32_t ActionErrno::createLibSeccompAction() const {
    return SCMP_ACT_ERRNO(errnoNumber_);
}

} // namespace action
} // namespace seccomp
} // namespace s2j

#pragma once

#include "SeccompAction.h"

#include <cstdint>

namespace s2j {
namespace seccomp {
namespace action {

class ActionErrno : public SeccompAction {
public:
    ActionErrno(uint64_t errnoNumber);

    Type getType() const override;

    tracer::TraceAction execute(tracer::Tracee& tracee) override;

protected:
    virtual uint32_t createLibSeccompAction() const override;

    uint64_t errnoNumber_;
};

} // namespace action
} // namespace seccomp
} // namespace s2j

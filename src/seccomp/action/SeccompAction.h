#pragma once

#include "tracer/TraceAction.h"
#include "tracer/Tracee.h"

#include <seccomp.h>

namespace s2j {
namespace seccomp {

class SeccompContext;

namespace action {

class SeccompAction {
public:
    enum class Type {
        ALLOW   = 0,
        TRACE   = 1,
        ERRNO   = 2,
        KILL    = 3
    };

    SeccompAction();
    virtual ~SeccompAction() = default;

    virtual Type getType() const = 0;

    virtual void setRuleId(uint32_t groupId);
    virtual uint32_t getRuleId() const;

    virtual tracer::TraceAction execute(tracer::Tracee& tracee) = 0;

protected:
    friend class s2j::seccomp::SeccompContext;

    virtual uint32_t createLibSeccompAction() const = 0;

    uint32_t ruleId_;
};

}
}

std::string to_string(const seccomp::action::SeccompAction::Type type);

}

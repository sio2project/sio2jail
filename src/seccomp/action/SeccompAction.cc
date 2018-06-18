#include "SeccompAction.h"

namespace s2j {
namespace seccomp {
namespace action {

SeccompAction::SeccompAction()
    : ruleId_(0) {}

void SeccompAction::setRuleId(uint32_t groupId) {
    ruleId_ = groupId;
}

uint32_t SeccompAction::getRuleId() const {
    return ruleId_;
}

}
}

std::string to_string(const seccomp::action::SeccompAction::Type type) {
    switch (type) {
        case seccomp::action::SeccompAction::Type::ALLOW:
            return "Allow";
        case seccomp::action::SeccompAction::Type::TRACE:
            return "Trace";
        case seccomp::action::SeccompAction::Type::ERRNO:
            return "Errno";
        case seccomp::action::SeccompAction::Type::KILL:
            return "Kill";
        default:
            return "Unknown";
    }
}

}

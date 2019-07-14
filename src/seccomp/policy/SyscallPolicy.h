#pragma once

#include "seccomp/SeccompRule.h"
#include "seccomp/action/SeccompAction.h"

namespace s2j {
namespace seccomp {
namespace policy {

class SyscallPolicy {
public:
    virtual const std::vector<SeccompRule>& getRules() const = 0;
};

class BaseSyscallPolicy : public SyscallPolicy {
public:
    BaseSyscallPolicy(std::shared_ptr<action::SeccompAction> defaultAction)
            : defaultAction_(defaultAction) {}

    std::shared_ptr<action::SeccompAction> getDefaultAction() const {
        return defaultAction_;
    }

private:
    std::shared_ptr<action::SeccompAction> defaultAction_;
};

} // namespace policy
} // namespace seccomp
} // namespace s2j

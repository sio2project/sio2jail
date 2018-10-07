#pragma once

#include "seccomp/action/ActionAllow.h"

namespace s2j {
namespace seccomp {
namespace policy {

class PermissivePolicy final : public BaseSyscallPolicy {
public:
    PermissivePolicy()
        : BaseSyscallPolicy(std::make_shared<action::ActionAllow>()) {}

    const std::vector<SeccompRule>& getRules() const override {
        static const std::vector<SeccompRule> emptyRules_;
        return emptyRules_;
    }
};

}
}
}

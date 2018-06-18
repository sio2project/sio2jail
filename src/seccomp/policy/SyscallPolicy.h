#pragma once

#include "seccomp/SeccompRule.h"

namespace s2j {
namespace seccomp {
namespace policy {

class SyscallPolicy {
public:
    virtual const std::vector<SeccompRule>& getRules() const = 0;
};

}
}
}

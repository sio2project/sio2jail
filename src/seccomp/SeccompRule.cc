#include "SeccompRule.h"
#include "SeccompException.h"

namespace s2j {
namespace seccomp {

SeccompRule::SeccompRule(uint32_t syscallNumber,
                         std::shared_ptr<action::SeccompAction> action,
                         std::shared_ptr<filter::SyscallFilter> filter)
    : syscall(syscallNumber), action(action), filter(filter) {}

SeccompRule::SeccompRule(const std::string syscallName,
                         std::shared_ptr<action::SeccompAction> action,
                         std::shared_ptr<filter::SyscallFilter> filter)
    : SeccompRule(resolveSyscallName(syscallName), action, filter) {}

SeccompRule::SeccompRule(uint32_t syscallNumber,
                         std::shared_ptr<action::SeccompAction> action)
    : SeccompRule(syscallNumber, action, std::make_shared<filter::LibSeccompFilter>()) {}

SeccompRule::SeccompRule(const std::string syscallName,
                         std::shared_ptr<action::SeccompAction> action)
    : SeccompRule(resolveSyscallName(syscallName), action) {}

uint32_t SeccompRule::resolveSyscallName(const std::string& name) {
    auto syscall = seccomp_syscall_resolve_name(name.c_str());
    if (syscall == __NR_SCMP_ERROR)
        throw UnknownSyscallNameException(name);
    return syscall;
}

}
}

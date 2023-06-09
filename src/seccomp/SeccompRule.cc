#include "SeccompRule.h"
#include "SeccompException.h"

#include <cstdint>

namespace s2j {
namespace seccomp {

SeccompRule::SeccompRule(
        uint32_t syscallNumber,
        std::shared_ptr<action::SeccompAction> action,
        std::shared_ptr<filter::SyscallFilter> filter)
        : syscall(syscallNumber)
        , action(std::move(action))
        , filter(std::move(filter)) {}

SeccompRule::SeccompRule(
        const std::string& syscallName,
        std::shared_ptr<action::SeccompAction> action,
        std::shared_ptr<filter::SyscallFilter> filter)
        : SeccompRule(
                  resolveSyscallName(syscallName),
                  std::move(action),
                  std::move(filter)) {}

SeccompRule::SeccompRule(
        uint32_t syscallNumber,
        std::shared_ptr<action::SeccompAction> action)
        : SeccompRule(
                  syscallNumber,
                  std::move(action),
                  std::make_shared<filter::LibSeccompFilter>()) {}

SeccompRule::SeccompRule(
        const std::string& syscallName,
        std::shared_ptr<action::SeccompAction> action)
        : SeccompRule(resolveSyscallName(syscallName), std::move(action)) {}

uint32_t SeccompRule::resolveSyscallName(const std::string& name) {
    auto syscall = seccomp_syscall_resolve_name(name.c_str());
    if (syscall == __NR_SCMP_ERROR) {
        throw UnknownSyscallNameException(name);
    }
    return syscall;
}

} // namespace seccomp
} // namespace s2j

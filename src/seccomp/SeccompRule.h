#pragma once

#include "action/SeccompAction.h"
#include "filter/LibSeccompFilter.h"
#include "filter/SyscallFilter.h"

#include <memory>
#include <type_traits>

namespace s2j {
namespace seccomp {

class SeccompRule {
public:
    SeccompRule(const SeccompRule&) = default;
    SeccompRule(SeccompRule&&) = default;

    SeccompRule(
            uint32_t syscallNumber,
            std::shared_ptr<action::SeccompAction> action,
            std::shared_ptr<filter::SyscallFilter> filter);
    SeccompRule(
            const std::string& syscallName,
            std::shared_ptr<action::SeccompAction> action,
            std::shared_ptr<filter::SyscallFilter> filter);
    SeccompRule(
            uint32_t syscallNumber,
            std::shared_ptr<action::SeccompAction> action);
    SeccompRule(
            const std::string& syscallName,
            std::shared_ptr<action::SeccompAction> action);

    template<typename SeccompAction, typename SyscallFilter>
    SeccompRule(
            const std::string syscallName,
            SeccompAction&& action,
            SyscallFilter&& filter,
            typename std::enable_if<std::is_base_of<
                    action::SeccompAction,
                    SeccompAction>::value>::type* = nullptr,
            typename std::enable_if<std::is_base_of<
                    filter::SyscallFilter,
                    SyscallFilter>::value>::type* = nullptr)
            : syscall(resolveSyscallName(syscallName))
            , action(std::make_shared<SeccompAction>(
                      std::forward<SeccompAction>(action)))
            , filter(std::make_shared<SyscallFilter>(
                      std::forward<SyscallFilter>(filter))) {}

    template<typename SeccompAction>
    SeccompRule(
            const std::string syscallName,
            SeccompAction&& action,
            typename std::enable_if<std::is_base_of<
                    action::SeccompAction,
                    SeccompAction>::value>::type* = nullptr)
            : syscall(resolveSyscallName(syscallName))
            , action(std::make_shared<SeccompAction>(
                      std::forward<SeccompAction>(action)))
            , filter(std::make_shared<filter::LibSeccompFilter>()) {}

    SeccompRule& operator=(const SeccompRule&) = default;
    SeccompRule& operator=(SeccompRule&&) = default;

    uint32_t syscall;
    std::shared_ptr<action::SeccompAction> action;
    std::shared_ptr<filter::SyscallFilter> filter;

private:
    static uint32_t resolveSyscallName(const std::string& name);
};

} // namespace seccomp
} // namespace s2j

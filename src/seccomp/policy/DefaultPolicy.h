#pragma once

#include "SyscallPolicy.h"
#include "seccomp/SeccompRule.h"

#include <signal.h>

namespace s2j {
namespace seccomp {
namespace policy {

/**
 * Class that represents generic, tweakable (TODO) syscall
 * policy used in sio2jail
 */
class DefaultPolicy : public BaseSyscallPolicy {
public:
    DefaultPolicy();

    const std::vector<SeccompRule>& getRules() const override;

private:
    /**
     * Process should be capable of controlling it's exection.
     */
    void addExecutionControlRules(bool allowFork = false);

    /**
     * Adds rules for read-only access to filesystem.
     */
    void addFileSystemAccessRules(bool readOnly = true);

    /**
     * Adds rules for input and output
     */
    void addInputOutputRules();

    /**
     * Adds rules for memory management
     */
    void addMemoryManagementRules();

    /**
     * Adds rules that control gaining informations about system.
     */
    void addSystemInformationRules();

    /**
     * Allow many syscalls.
     */
    void allowSyscalls(std::initializer_list<std::string> syscalls);

    /**
     * If signal is valid for tracee to send.
     */
    static inline bool isSignalValid(int signal) {
        return signal != SIGSTOP && signal != SIGKILL && signal != SIGVTALRM &&
               signal != SIGKILL;
    }

    std::vector<SeccompRule> rules_;
};

} // namespace policy
} // namespace seccomp
} // namespace s2j

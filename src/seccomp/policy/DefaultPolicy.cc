#include "DefaultPolicy.h"
#include "seccomp/action/ActionAllow.h"
#include "seccomp/action/ActionErrno.h"
#include "seccomp/action/ActionKill.h"
#include "seccomp/action/ActionTrace.h"
#include "seccomp/filter/LibSeccompFilter.h"

#include <fcntl.h>

namespace s2j {
namespace seccomp {
namespace policy {

DefaultPolicy::DefaultPolicy()
        : BaseSyscallPolicy(std::make_shared<action::ActionKill>()) {
    addExecutionControlRules();
    addMemoryManagementRules();
    addSystemInformationRules();
    addFileSystemAccessRules();
    addInputOutputRules();
}

const std::vector<SeccompRule>& DefaultPolicy::getRules() const {
    return rules_;
}

void DefaultPolicy::addExecutionControlRules(bool allowFork) {
    // Some syscalls must be enabled
    allowSyscalls(
            {"restart_syscall",
             "getpriority",
             "setpriority",
             "sigaction",
             "sigaltstack",
             "rt_sigaction",
             "rt_sigprocmask",
             "futex",
             "set_tid_address",
             "set_robust_list",
             "getpid",
             "getrandom",
             "sigaltstack",
             "sigsuspend",
             "clock_nanosleep",
             "open",
             "epoll_create1",
             "openat"});

    rules_.emplace_back(SeccompRule(
            "set_thread_area", action::ActionTrace([](auto& /* tracee */) {
                // Allow syscall, let sio2jail detect syscall architecture
                return tracer::TraceAction::CONTINUE;
            })));

    rules_.emplace_back(SeccompRule(
            "execve",
            action::ActionTrace([executed = false](auto& /* tracee */) mutable {
                if (executed) {
                    return tracer::TraceAction::KILL;
                }
                executed = true;
                return tracer::TraceAction::CONTINUE;
            })));

    for (const auto& syscall: {"kill", "tkill"}) {
        rules_.emplace_back(SeccompRule(
                syscall, action::ActionTrace([](auto& tracee) {
                    if (isSignalValid(tracee.getSyscallArgument(1))) {
                        return tracer::TraceAction::CONTINUE;
                    }
                    return tracer::TraceAction::KILL;
                })));
    }

    rules_.emplace_back(
            SeccompRule("tgkill", action::ActionTrace([](auto& tracee) {
                            if (isSignalValid(tracee.getSyscallArgument(2))) {
                                return tracer::TraceAction::CONTINUE;
                            }
                            return tracer::TraceAction::KILL;
                        })));
    for (const auto& syscall: {
                 "exit",
                 "exit_group",
         }) {
        rules_.emplace_back(SeccompRule(syscall, action::ActionTrace()));
    }

    for (const auto& syscall: {"rseq"}) {
        rules_.emplace_back(SeccompRule(syscall, action::ActionErrno(ENOSYS)));
    }

    if (allowFork) {
        allowSyscalls({"fork"});
    }

    // Others may be always unaccessible
    for (const auto& syscall: {"prlimit64"}) {
        rules_.emplace_back(SeccompRule(syscall, action::ActionErrno(EPERM)));
    }
}

void DefaultPolicy::addMemoryManagementRules() {
    allowSyscalls(
            {"brk",
             "munmap",
             "mremap",
             "mprotect",
             "arch_prctl"});

    // Allow mmap and mmap2 only on fd >= 3
    for (const auto& syscall: {"mmap", "mmap2"}) {
        rules_.emplace_back(SeccompRule(
                syscall,
                action::ActionAllow(),
                filter::SyscallArg(4) >= 3));
    }

    rules_.emplace_back(SeccompRule{"madvise", action::ActionErrno{EINVAL}});
}

void DefaultPolicy::addSystemInformationRules() {
    allowSyscalls({"getuid",    "getuid32",      "getgid",       "getgid32",
                   "geteuid",   "geteuid32",     "getegid",      "getegid32",
                   "getrlimit", "ugetrlimit",    "getcpu",       "gettid",
                   "getcwd",    "uname",         "olduname",     "oldolduname",
                   "sysinfo",   "clock_gettime", "gettimeofday", "time"});
}

void DefaultPolicy::addInputOutputRules() {
    // Allow writing to stdout and stderr
    for (const auto& syscall: {"write", "writev"}) {
        rules_.emplace_back(SeccompRule(
                syscall, action::ActionAllow(), filter::SyscallArg(0) > 0));
    }

    // Allow dup only on fd >= 3
    rules_.emplace_back(SeccompRule(
            "dup", action::ActionAllow(), filter::SyscallArg(0) >= 3));
    for (const auto& syscall: {"dup2", "dup3"}) {
        rules_.emplace_back(SeccompRule(
                syscall,
                action::ActionAllow(),
                filter::SyscallArg(0) >= 3 &&
                filter::SyscallArg(1) >= 3));
    }

    // Allow reading from any file descriptor
    allowSyscalls({"read", "readv", "fcntl", "fcntl64", "pread64"});

    rules_.emplace_back(SeccompRule("ioctl", action::ActionErrno(ENOTTY)));

    // Allow seeking any file other than stdin/stdou/stderr
    for (const auto& syscall: {"lseek", "_llseek"}) {
        rules_.emplace_back(SeccompRule(
                syscall,
                action::ActionErrno(ESPIPE),
                filter::SyscallArg(0) <= 2));
        rules_.emplace_back(SeccompRule(
                syscall, action::ActionAllow(), filter::SyscallArg(0) >= 3));
    }
}

void DefaultPolicy::addFileSystemAccessRules(bool readOnly) {
    // Allow any informations about file system
    allowSyscalls({
            "stat",
            "stat64",
            "fstat",
            "fstat64",
            "newfstatat",
            "lstat",
            "lstat64",
            "listxattr",
            "llistxattr",
            "flistxattr",
            "readlink",
            "readlinkat",
            "access",
            "faccessat",
            "getdents",
            "getdents64",
    });

    rules_.emplace_back(SeccompRule(
            "close", action::ActionAllow(), filter::SyscallArg(0) >= 3));

    for (const auto& syscall: {
                 "statfs",
                 "statfs64",
                 "fstatfs",
                 "fstatfs64",
         }) {
        rules_.emplace_back(SeccompRule(syscall, action::ActionErrno(ENOSYS)));
    }

    if (readOnly) {
        rules_.emplace_back(SeccompRule(
                "open",
                action::ActionAllow(),
                (filter::SyscallArg(1) & (O_RDWR | O_WRONLY)) == 0));

        rules_.emplace_back(SeccompRule(
                "openat",
                action::ActionAllow(),
                (filter::SyscallArg(2) & (O_RDWR | O_WRONLY)) == 0));

        for (const auto& syscall: {
                     "unlink",
                     "unlinkat",
                     "symlink",
                     "symlinkat",
                     "mkdir",
                     "mkdirat",
                     "setxattr",
                     "lsetxattr",
                     "fsetxattr",
             }) {
            rules_.emplace_back(
                    SeccompRule(syscall, action::ActionErrno(EPERM)));
        }
    }
    else {
        allowSyscalls({
                "open",
                "openat",
                "unlink",
                "unlinkat",
                "symlink",
                "symlinkat",
                "mkdir",
                "mkdirat",
                "setxattr",
                "lsetxattr",
                "fsetxattr",
        });
    }
}

void DefaultPolicy::allowSyscalls(std::initializer_list<std::string> syscalls) {
    for (const auto& syscall: syscalls) {
        rules_.emplace_back(SeccompRule(syscall, action::ActionAllow()));
    }
}

} // namespace policy
} // namespace seccomp
} // namespace s2j

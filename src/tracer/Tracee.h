#pragma once

#include "ProcessInfo.h"

#include <string>

#include <sys/user.h>
#include <unistd.h>

#if defined(__x86_64__)
using reg_t = uint64_t;
#elif defined(__i386__)
using reg_t = uint32_t;
#else
#error "arch not supported"
#endif

namespace s2j {
namespace tracer {

enum Arch : uint8_t { UNKNOWN = 0, X86 = 1, X86_64 = 2 };

class Tracee {
public:
    Tracee(std::shared_ptr<ProcessInfo> traceeInfo);

    pid_t getPid() const {
        return traceeInfo_->getPid();
    }

    std::shared_ptr<ProcessInfo> getInfo() {
        return traceeInfo_;
    }

    /**
     * Checks wheather underlying process is still alive.
     */
    bool isAlive();

    /**
     * Returns trace event message.
     */
    int64_t getEventMsg();

    /**
     * Syscall related functions, will work only with seccomp listener.
     */
    void setSyscallArch(Arch arch);
    Arch getSyscallArch() const;
    reg_t getSyscallNumber();
    reg_t getSyscallArgument(uint8_t argumentNumber);

    void cancelSyscall(reg_t returnValue);

    std::string getMemoryString(uint64_t address, size_t sizeLimit = 512);

    // TODO: simple wrapper around ptrace syscall that simulates RW access to
    // traced process. getRegister getMemory getMemoryString
    // ...

private:
    std::shared_ptr<ProcessInfo> traceeInfo_;
    user_regs_struct regs_{};

    Arch syscallArch_;
};

} // namespace tracer

std::string to_string(const tracer::Arch);

} // namespace s2j

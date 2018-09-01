#pragma once

#include <string>

#include <unistd.h>
#include <sys/user.h>

namespace s2j {
namespace tracer {

enum Arch : uint8_t {
    UNKNOWN     = 0,
    X86         = 1,
    X86_64      = 2
};

class Tracee {
public:
    Tracee(pid_t traceePid);

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
    uint64_t getSyscallNumber();
    uint64_t getSyscallArgument(uint8_t argumentNumber);

    void cancelSyscall(uint64_t returnValue);

    std::string getMemoryString(uint64_t address, size_t sizeLimit = 512);

    // TODO: simple wrapper around ptrace syscall that simulates RW access to traced process.
    // getRegister
    // getMemory
    // getMemoryString
    // ...

private:
    pid_t traceePid_;
    user_regs_struct regs_;

    Arch syscallArch_;
};

}

std::string to_string(const tracer::Arch);

}
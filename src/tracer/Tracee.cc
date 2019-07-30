#include "Tracee.h"

#include "common/Assert.h"
#include "common/Exception.h"
#include "common/WithErrnoCheck.h"

#include <sys/ptrace.h>

#include <csignal>

namespace s2j {
namespace tracer {

Tracee::Tracee(std::shared_ptr<ProcessInfo> traceeInfo)
        : traceeInfo_{std::move(traceeInfo)}, syscallArch_{Arch::UNKNOWN} {
    assert(traceeInfo_ != nullptr);

    if (isAlive()) {
        try {
            withErrnoCheck(
                    "ptrace getregs",
                    ptrace,
                    PTRACE_GETREGS,
                    getPid(),
                    nullptr,
                    &regs_);
        }
        catch (const SystemException& e) {
            if (e.getErrno() != ESRCH) {
                throw;
            }
        }
    }
}

bool Tracee::isAlive() {
    return kill(getPid(), 0) == 0;
}

int64_t Tracee::getEventMsg() {
    int64_t code;
    withErrnoCheck(
            "ptrace geteventmsg",
            ptrace,
            PTRACE_GETEVENTMSG,
            getPid(),
            nullptr,
            &code);
    return code;
}

void Tracee::setSyscallArch(Arch arch) {
    syscallArch_ = arch;
}

Arch Tracee::getSyscallArch() const {
    return syscallArch_;
}

reg_t Tracee::getSyscallNumber() {
    if (syscallArch_ == Arch::UNKNOWN) {
        throw Exception("Can't get syscall number, unknown syscall arch");
    }
#if defined(__x86_64__)
    return regs_.orig_rax;
#elif defined(__i386__)
    return regs_.orig_eax;
#else
#error "arch not supported"
#endif
}

reg_t Tracee::getSyscallArgument(uint8_t argumentNumber) {
#if defined(__x86_64__)
    if (syscallArch_ == Arch::X86) {
        switch (argumentNumber) {
        case 0:
            return static_cast<uint32_t>(regs_.rbx);

        case 1:
            return static_cast<uint32_t>(regs_.rcx);

        case 2:
            return static_cast<uint32_t>(regs_.rdx);

        case 3:
            return static_cast<uint32_t>(regs_.rsi);

        case 4:
            return static_cast<uint32_t>(regs_.rdi);

        case 5:
            return static_cast<uint32_t>(regs_.rbp);
        }
    }
    else if (syscallArch_ == Arch::X86_64) {
        switch (argumentNumber) {
        case 0:
            return regs_.rdi;

        case 1:
            return regs_.rsi;

        case 2:
            return regs_.rdx;

        case 3:
            return regs_.r10;

        case 4:
            return regs_.r8;

        case 5:
            return regs_.r9;
        }
    }
#elif defined(__i386__)
    if (syscallArch_ == Arch::X86) {
        switch (argumentNumber) {
        case 0:
            return static_cast<uint32_t>(regs_.ebx);

        case 1:
            return static_cast<uint32_t>(regs_.ecx);

        case 2:
            return static_cast<uint32_t>(regs_.edx);

        case 3:
            return static_cast<uint32_t>(regs_.esi);

        case 4:
            return static_cast<uint32_t>(regs_.edi);

        case 5:
            return static_cast<uint32_t>(regs_.ebp);
        }
    }
    else if (syscallArch_ == Arch::X86_64) {
        throw s2j::AssertionException(
                "Tracing 64bit program from 32bit not implemented");
    }
#else
#error "arch not supported"
#endif
    else {
        throw Exception("Can't get syscall argument, unknown syscall arch");
    }
    throw Exception(
            "No such syscall argument number " +
            std::to_string(argumentNumber));
}

std::string Tracee::getMemoryString(
        uint64_t /*address*/,
        size_t /*sizeLimit*/) {
    NOT_IMPLEMENTED();
    // This code is broken in may ways. Implement using process_vm_readv
    // syscall.
    /*
    for (size_t ptr = address; str.size() < sizeLimit; ptr += sizeof(int)) {
        int word = withErrnoCheck(
                "ptrace peektext",
                ptrace,
                PTRACE_PEEKTEXT,
                traceePid_,
                ptr,
                nullptr);
        for (size_t i = 0; i < sizeof(word); ++i) {
            char byte = (word >> (8 * i));
            if (byte == '\0')
                return str;
            str += byte;
        }
    }
    */
}

void Tracee::cancelSyscall(reg_t returnValue) {
#if defined(__x86_64__)
    regs_.orig_rax = -1;
    regs_.rax = returnValue;
#elif defined(__i386__)
    regs_.orig_eax = -1;
    regs_.eax = returnValue;
#else
#error "arch not supported"
#endif
    withErrnoCheck(
            "ptrace setregs",
            ptrace,
            PTRACE_SETREGS,
            getPid(),
            nullptr,
            &regs_);
}

} // namespace tracer

std::string to_string(const tracer::Arch arch) {
    switch (arch) {
    case tracer::Arch::X86:
        return "x86";
    case tracer::Arch::X86_64:
        return "x86_64";
    default:
        return "UNKNOWN";
    }
}

} // namespace s2j

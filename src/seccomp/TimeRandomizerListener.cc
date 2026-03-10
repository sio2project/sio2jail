#include "TimeRandomizerListener.h"

#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <sys/prctl.h>
#include <sys/ptrace.h>

namespace s2j {
namespace seccomp {

const Feature TimeRandomizerListener::feature = Feature::FAKE_TIME;

TimeRandomizerListener::TimeRandomizerListener(bool returnZero)
        : returnZero_(returnZero) {}

void TimeRandomizerListener::onPostForkChild() {
    TRACE();

    // PR_SET_TSC with PR_TSC_SIGSEGV causes RDTSC/RDTSCP instructions
    // to deliver SIGSEGV instead of executing. The tracer intercepts
    // these SIGSEGVs and emulates the instructions with random values.
    // This setting is inherited across execve.
    withErrnoCheck(
            "prctl set tsc sigsegv",
            prctl,
            PR_SET_TSC,
            PR_TSC_SIGSEGV,
            0,
            0,
            0);

    logger::debug("Disabled RDTSC/RDTSCP via PR_SET_TSC");
}

tracer::TraceAction TimeRandomizerListener::onTraceEvent(
        const tracer::TraceEvent& traceEvent,
        tracer::Tracee& tracee) {
    int signal = traceEvent.executeEvent.signal & 0xff;
    if (signal != SIGSEGV) {
        return tracer::TraceAction::CONTINUE;
    }

    reg_t rip = tracee.getInstructionPointer();

    // Read instruction bytes at RIP using PTRACE_PEEKTEXT.
    // PEEKTEXT returns -1 on error but -1 can also be valid data,
    // so we must check errno explicitly.
    errno = 0;
    long word = ptrace(PTRACE_PEEKTEXT, tracee.getPid(), rip, nullptr);
    if (errno != 0) {
        // Cannot read memory at RIP, let normal signal handling proceed
        return tracer::TraceAction::CONTINUE;
    }

    auto byte0 = static_cast<uint8_t>(word & 0xff);
    auto byte1 = static_cast<uint8_t>((word >> 8) & 0xff);
    auto byte2 = static_cast<uint8_t>((word >> 16) & 0xff);

    size_t instrSize = 0;
    if (byte0 == 0x0F && byte1 == 0x31) {
        // RDTSC: 0F 31
        instrSize = 2;
    }
    else if (byte0 == 0x0F && byte1 == 0x01 && byte2 == 0xF9) {
        // RDTSCP: 0F 01 F9
        instrSize = 3;
    }
    else {
        // Not RDTSC/RDTSCP, let normal SIGSEGV handling proceed
        return tracer::TraceAction::CONTINUE;
    }

    // Emulate RDTSC with fake values
    reg_t fakeRax = returnZero_ ? 0 : static_cast<reg_t>(rand());
    reg_t fakeRdx = returnZero_ ? 0 : static_cast<reg_t>(rand());

    tracee.setRegisters(rip + instrSize, fakeRax, fakeRdx);
    tracee.suppressSignal();

    logger::debug(
            "Emulated RDTSC at RIP=",
            rip,
            " with fake values RAX=",
            fakeRax,
            " RDX=",
            fakeRdx);

    return tracer::TraceAction::CONTINUE;
}

} // namespace seccomp
} // namespace s2j

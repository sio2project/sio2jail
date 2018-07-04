#include "PerfListener.h"

#include "common/Exception.h"
#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>

#include <cstdlib>
#include <cstring>

namespace {

long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

}

namespace s2j {
namespace perf {

const Feature PerfListener::feature = Feature::PERF;

PerfListener::PerfListener(uint64_t instructionCountLimit)
    : perfFd_(-1), instructionCountLimit_(instructionCountLimit) {}

PerfListener::~PerfListener() {
    // TODO: handle closing perfFd in move assignement / constructor as well
    if (perfFd_ >= 0) {
        close(perfFd_);
        perfFd_ = -1;
    }
}

void PerfListener::onPreFork() {
    TRACE();

    barrier_ =
            withErrnoCheck("mmap shared memory", mmap,
                nullptr, sizeof(pthread_barrier_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0).as<pthread_barrier_t*>();

    pthread_barrierattr_t attr;
    pthread_barrierattr_init(&attr);
    pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    pthread_barrier_init(barrier_, &attr, 2);

    pthread_barrierattr_destroy(&attr);
}

void PerfListener::onPostForkParent(pid_t childPid) {
    TRACE();

    childPid_ = childPid;
    // TODO: fix this // What fix?
    struct perf_event_attr attrs;
    memset(&attrs, 0, sizeof(attrs));
    attrs.type = PERF_TYPE_HARDWARE;
    attrs.size = sizeof(attrs);
    attrs.config = PERF_COUNT_HW_INSTRUCTIONS;
    attrs.exclude_user = 0;
    attrs.exclude_kernel = 1;
    attrs.exclude_hv = 1;
    attrs.disabled = 1;
    attrs.enable_on_exec = 1;
    if (instructionCountLimit_ != 0) {
        attrs.sample_period = instructionCountLimit_;
        attrs.wakeup_events = 1;
    }
    // Apparently older (3.13) kernel versions doesn't support PERF_FLAG_FD_CLOEXEC. This
    // fd will be closed anyway (by FilesListener) so it isn't very bad to not use it on
    // newer kernels (and add it with fcnlt) until we implement some linux version discovery.
    perfFd_ = withErrnoCheck("perf event open", perf_event_open, &attrs, childPid, -1, -1,
            PERF_FLAG_FD_NO_GROUP /* | PERF_FLAG_FD_CLOEXEC */);
    withErrnoCheck("set cloexec flag on perfFd", fcntl, F_SETFD, FD_CLOEXEC);
    if (instructionCountLimit_ != 0) {
        int myPid = getpid();
        withErrnoCheck("fcntl", fcntl, perfFd_, F_SETOWN, myPid);
        int oldFlags = withErrnoCheck("fcntl", fcntl, perfFd_, F_GETFL, 0);;
        withErrnoCheck("fcntl", fcntl, perfFd_, F_SETFL, oldFlags | O_ASYNC);
    }

    pthread_barrier_wait(barrier_);
    pthread_barrier_destroy(barrier_);
    withErrnoCheck("munmap shared memory", munmap,
            barrier_, sizeof(pthread_barrier_t));
}

void PerfListener::onPostForkChild() {
    TRACE();

    pthread_barrier_wait(barrier_);
    withErrnoCheck("munmap shared memory", munmap,
            barrier_, sizeof(pthread_barrier_t));
}

uint64_t PerfListener::getInstructionsUsed() {
    TRACE();

    long long int instructionsUsed;
    int size = withErrnoCheck("read perf value", read, perfFd_, &instructionsUsed, sizeof(long long));
    if (size != sizeof(instructionsUsed))
        throw Exception("read failed");
    if (instructionsUsed < 0)
        throw Exception("read negative instructions count");
    return (uint64_t)instructionsUsed;
}

void PerfListener::onPostExecute() {
    TRACE();

    outputBuilder_->setCyclesUsed(getInstructionsUsed());
}

executor::ExecuteAction PerfListener::onSigioSignal() {
    TRACE();

    uint64_t instructionsUsed = getInstructionsUsed();
    if (instructionCountLimit_ != 0 && instructionsUsed >= instructionCountLimit_) {
        logger::debug("Killing tracee after instructions count ", instructionsUsed, " exceeded limit");
        outputBuilder_->setKillReason(printer::OutputBuilder::KillReason::TLE, "time limit exceeded");
        return executor::ExecuteAction::KILL;
    }
    return executor::ExecuteAction::CONTINUE;
}

}
}

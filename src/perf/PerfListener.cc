#include "PerfListener.h"

#include "common/Assert.h"
#include "common/Exception.h"
#include "common/Utils.h"
#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <asm/unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <sys/mman.h>

#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>

namespace {

long perf_event_open(
        struct perf_event_attr* hw_event,
        pid_t pid,
        int cpu,
        int group_fd,
        unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

} // namespace

namespace s2j {
namespace perf {

const Feature PerfListener::feature = Feature::PERF;

PerfListener::PerfListener(
        uint64_t instructionCountLimit,
        uint64_t samplingFactor)
        : instructionCountLimit_(instructionCountLimit)
        , samplingFactor_{std::max<uint64_t>(1ULL, samplingFactor)} {}

PerfListener::~PerfListener() {
    // TODO: handle closing perfFds in move assignement / constructor as well
    for (int& fd: perfFds_)
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
}

void PerfListener::onPreFork() {
    TRACE();

    barrier_ =
            withGuardedErrnoCheck(
                    "mmap shared memory",
                    [](auto&& result) -> bool { return result != MAP_FAILED; },
                    mmap,
                    nullptr,
                    sizeof(pthread_barrier_t),
                    PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_SHARED,
                    0,
                    0)
                    .as<pthread_barrier_t*>();

    pthread_barrierattr_t attr;
    pthread_barrierattr_init(&attr);
    pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    pthread_barrier_init(barrier_, &attr, 2);

    pthread_barrierattr_destroy(&attr);
}

// A "simple" file is oneline and has no whitespace.
std::string readSimpleFile(const std::string& path) {
    std::string result;
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    file >> result;
    return result;
}

// Parse "config", "config1" and "config2", for everything else return -1.
int getConfigFieldNumber(const std::string& name) {
    if (name.size() < 6 || name.size() > 7 || name.substr(0, 6) != "config") {
        return -1;
    }
    int index = name.size() == 7 ? int(name[6] - '0') : 0;
    return index < 0 || index > 3 ? -1 : index;
}

struct perfEventConfig {
    uint32_t type = 0;
    uint64_t config[3] = {0, 0, 0};
    void insertIntoConfig(const std::string& format, uint64_t value) {
        // `format` specifies the bits which need to be set to `value`.
        // It can look like config:0-7, config1:8 or even config:0-7,32-35.
        // According to https://lwn.net/Articles/611945/, the attributes
        // can overlap.
        auto spl = split(format, ":");
        assert_1(spl.size() == 2);
        int field = getConfigFieldNumber(spl[0]);
        assert_1(field >= 0);
        // Iterate over bit ranges.
        for (const std::string& s: split(spl[1], ",")) {
            auto rangespl = split(s, "-");
            assert_1(rangespl.size() && rangespl.size() < 3);
            unsigned long start = std::stoul(rangespl[0]), end = start;
            if (rangespl.size() == 2) {
                end = std::stoul(rangespl[1]);
            }
            assert_1(start <= end);
            unsigned long width = end - start + 1;
            config[field] |= (value & ((1ull << width) - 1)) << start;
            value >>= width;
        }
        assert_1(!value);
    }
};

void PerfListener::onPostForkParent(pid_t childPid) {
    TRACE();

    childPid_ = childPid;

    std::vector<perfEventConfig> eventConfigs;
    const std::string sysfsPath = "/sys/devices";
    std::unique_ptr<DIR, int (*)(DIR*)> sysfsDir(
            withErrnoCheck(
                    "open /sys/devices directory", opendir, sysfsPath.c_str()),
            closedir);
    for (struct dirent* entry = readdir(sysfsDir.get()); entry != nullptr;
         entry = readdir(sysfsDir.get())) {
        // According to linux's perf tool at tools/perf/util/pmus.c, we need
        // to consider folders with the "cpu" name or with a "cpus" file inside.
        std::string dir = sysfsPath + "/" + entry->d_name + "/",
                    cpus = readSimpleFile(dir + "cpus"),
                    type = readSimpleFile(dir + "type");
        if ((strcmp(entry->d_name, "cpu") && !cpus.size()) || !type.size()) {
            continue;
        }

        logger::debug("Generating a raw perf event from ", dir);
        perfEventConfig config;
        config.type = std::stoul(type);
        std::string configStr = readSimpleFile(dir + "events/instructions");
        assert_1(configStr.size());

        // This parses for example "event=0x2e,umask=0x4f"
        for (const std::string& s: split(configStr, ",")) {
            auto spl = split(s, "=0x");
            assert_1(spl.size() == 2);
            std::string& name = spl[0];
            uint64_t value = std::stoull(spl[1], nullptr, 16);
            logger::debug("Setting '", name, "' in the config to ", value);
            int field = getConfigFieldNumber(name);
            if (field >= 0) { // set an entire config field
                config.config[field] |= value;
            }
            else { // set only certain bits
                std::string format = readSimpleFile(dir + "format/" + name);
                assert_1(format.size());
                config.insertIntoConfig(format, value);
            }
        }
        eventConfigs.emplace_back(config);
    }
    // Inform the user rather than silently provide a possibly faulty fallback.
    if (eventConfigs.empty()) {
        throw Exception("failed to generate at least one perf event config");
    }

    struct perf_event_attr attrs {};
    memset(&attrs, 0, sizeof(attrs));
    attrs.size = sizeof(attrs);
    attrs.exclude_user = 0;
    attrs.exclude_kernel = 1;
    attrs.exclude_hv = 1;
    attrs.disabled = 1;
    attrs.enable_on_exec = 1;
    attrs.inherit = 1;
    if (instructionCountLimit_ != 0) {
        attrs.sample_period = instructionCountLimit_ / samplingFactor_;
        attrs.wakeup_events = 1;
    }

    for (auto& config: eventConfigs) {
        logger::debug("Opening perf event for pmu with id ", config.type);
        attrs.type = config.type;
        attrs.config = config.config[0];
        attrs.config1 = config.config[1];
        attrs.config2 = config.config[2];
        // Apparently older (3.13) kernel versions doesn't support
        // PERF_FLAG_FD_CLOEXEC. This fd will be closed anyway (by
        // FilesListener) so it isn't very bad to not use it on newer kernels
        // (and add it with fcnlt) until we implement some linux version
        // discovery.
        int perfFd = withErrnoCheck(
                "perf event open",
                perf_event_open,
                &attrs,
                childPid,
                -1,
                -1,
                PERF_FLAG_FD_NO_GROUP /* | PERF_FLAG_FD_CLOEXEC */);
        withErrnoCheck(
                "set cloexec flag on perfFd", fcntl, F_SETFD, FD_CLOEXEC);
        if (instructionCountLimit_ != 0) {
            int myPid = getpid();
            withErrnoCheck("fcntl", fcntl, perfFd, F_SETOWN, myPid);
            int oldFlags = withErrnoCheck("fcntl", fcntl, perfFd, F_GETFL, 0);
            withErrnoCheck("fcntl", fcntl, perfFd, F_SETFL, oldFlags | O_ASYNC);
        }
        perfFds_.emplace_back(perfFd);
    }

    pthread_barrier_wait(barrier_);
    pthread_barrier_destroy(barrier_);
    withErrnoCheck(
            "munmap shared memory",
            munmap,
            barrier_,
            sizeof(pthread_barrier_t));
}

void PerfListener::onPostForkChild() {
    TRACE();

    pthread_barrier_wait(barrier_);
    withErrnoCheck(
            "munmap shared memory",
            munmap,
            barrier_,
            sizeof(pthread_barrier_t));
}

uint64_t PerfListener::getInstructionsUsed() {
    TRACE();

    uint64_t instructionsUsedSum = 0;
    for (int fd: perfFds_) {
        long long int instructionsUsed;
        int size = withErrnoCheck(
                "read perf value",
                read,
                fd,
                &instructionsUsed,
                sizeof(long long));
        if (size != sizeof(instructionsUsed)) {
            throw Exception("read failed");
        }
        if (instructionsUsed < 0) {
            throw Exception("read negative instructions count");
        }
        instructionsUsedSum += static_cast<uint64_t>(instructionsUsed);
    }
    return instructionsUsedSum;
}

void PerfListener::onPostExecute() {
    TRACE();

    outputBuilder_->setCyclesUsed(getInstructionsUsed());
}

executor::ExecuteAction PerfListener::onSigioSignal() {
    TRACE();

    uint64_t instructionsUsed = getInstructionsUsed();
    if (instructionCountLimit_ != 0 &&
        instructionsUsed >= instructionCountLimit_) {
        logger::debug(
                "Killing tracee after instructions count ",
                instructionsUsed,
                " exceeded limit");
        outputBuilder_->setKillReason(
                printer::OutputBuilder::KillReason::TLE, "time limit exceeded");
        return executor::ExecuteAction::KILL;
    }
    return executor::ExecuteAction::CONTINUE;
}

} // namespace perf
} // namespace s2j

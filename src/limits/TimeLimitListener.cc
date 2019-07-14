#include "TimeLimitListener.h"

#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <limits>
#include <signal.h>
#include <time.h>
#include <unistd.h>

namespace s2j {
namespace limits {

const uint64_t TimeLimitListener::TIMER_TICKING_INTERVAL_US =
        200 * 1000; // 200ms
const long TimeLimitListener::CLOCK_TICKS_PER_SECOND =
        withErrnoCheck("sysconf", sysconf, _SC_CLK_TCK);

TimeLimitListener::TimeLimitListener(
        uint64_t rTimelimitUs,
        uint64_t uTimelimitUs,
        uint64_t sTimelimitUs,
        uint64_t usTimelimitUs)
        : rTimelimitUs_(rTimelimitUs)
        , uTimelimitUs_(uTimelimitUs)
        , sTimelimitUs_(sTimelimitUs)
        , usTimelimitUs_(usTimelimitUs)
        , isTimerCreated_(false) {
    TRACE(rTimelimitUs, uTimelimitUs, sTimelimitUs, usTimelimitUs);
}

TimeLimitListener::~TimeLimitListener() {
    if (isTimerCreated_) {
        timer_delete(timerId_);
        isTimerCreated_ = false;
    }
}

void TimeLimitListener::onPostForkParent(pid_t childPid) {
    childPid_ = childPid;
    // TODO: run this just before execve
    startRealTime_ = std::chrono::steady_clock::now();

    uint64_t firstTimerTick = std::numeric_limits<uint64_t>::max();
    if (rTimelimitUs_ != 0 && rTimelimitUs_ < firstTimerTick) {
        firstTimerTick = rTimelimitUs_;
    }
    if (uTimelimitUs_ != 0 && uTimelimitUs_ < firstTimerTick) {
        firstTimerTick = uTimelimitUs_;
    }
    if (sTimelimitUs_ != 0 && sTimelimitUs_ < firstTimerTick) {
        firstTimerTick = sTimelimitUs_;
    }
    if (usTimelimitUs_ != 0 && usTimelimitUs_ < firstTimerTick) {
        firstTimerTick = usTimelimitUs_;
    }

    if (firstTimerTick != std::numeric_limits<uint64_t>::max()) {
        withErrnoCheck(
                "timer_create",
                timer_create,
                CLOCK_MONOTONIC,
                nullptr,
                &timerId_);
        isTimerCreated_ = true;

        struct itimerspec timerSpec;
        timerSpec.it_value.tv_sec = firstTimerTick / 1000000;
        timerSpec.it_value.tv_nsec = firstTimerTick % 1000000 * 1000;
        timerSpec.it_interval.tv_sec = TIMER_TICKING_INTERVAL_US / 1000000;
        timerSpec.it_interval.tv_nsec =
                TIMER_TICKING_INTERVAL_US % 1000000 * 1000;
        withErrnoCheck(
                "timer_settime",
                timer_settime,
                timerId_,
                0,
                &timerSpec,
                nullptr);
    }
}

executor::ExecuteAction TimeLimitListener::onSigalrmSignal() {
    if (!isTimerCreated_) {
        return executor::ExecuteAction::CONTINUE;
    }
    return verifyTimeUsage();
}

void TimeLimitListener::onPostExecute() {
    // TODO: run this just after child exit
    verifyTimeUsage();
    // TODO Save time usage to OutputBuilder.
}

executor::ExecuteAction TimeLimitListener::verifyTimeUsage() {
    if (rTimelimitUs_ != 0 && getRealTimeUsage() > rTimelimitUs_) {
        outputBuilder_->setKillReason(
                printer::OutputBuilder::KillReason::TLE,
                "real time limit exceeded");
        return executor::ExecuteAction::KILL;
    }
    if (uTimelimitUs_ != 0 || sTimelimitUs_ != 0 || usTimelimitUs_ != 0) {
        ProcessTimeUsage ptu = getProcessTimeUsage();
        if (uTimelimitUs_ != 0 && ptu.uTimeUs > uTimelimitUs_) {
            outputBuilder_->setKillReason(
                    printer::OutputBuilder::KillReason::TLE,
                    "user time limit exceeded");
            return executor::ExecuteAction::KILL;
        }
        if (sTimelimitUs_ != 0 && ptu.sTimeUs > sTimelimitUs_) {
            outputBuilder_->setKillReason(
                    printer::OutputBuilder::KillReason::TLE,
                    "system time limit exceeded");
            return executor::ExecuteAction::KILL;
        }
        if (usTimelimitUs_ != 0 && ptu.uTimeUs + ptu.sTimeUs > usTimelimitUs_) {
            outputBuilder_->setKillReason(
                    printer::OutputBuilder::KillReason::TLE,
                    "user+system time limit exceeded");
            return executor::ExecuteAction::KILL;
        }
    }
    return executor::ExecuteAction::CONTINUE;
}

uint64_t TimeLimitListener::getRealTimeUsage() {
    std::chrono::steady_clock::time_point now =
            std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration realTimeUsage = now - startRealTime_;
    uint64_t realTimeUsageUs =
            std::chrono::duration_cast<std::chrono::microseconds>(realTimeUsage)
                    .count();
    return realTimeUsageUs;
}

TimeLimitListener::ProcessTimeUsage TimeLimitListener::getProcessTimeUsage() {
    std::ifstream stat("/proc/" + std::to_string(childPid_) + "/stat");
    if (!stat.good()) {
        throw SystemException("Error reading /proc/childPid_/stat");
    }

    for (int i = 1; i <= 13; i++) {
        stat.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
        if (!stat.good()) {
            throw SystemException("Error reading /proc/childPid_/stat");
        }
    }

    uint64_t uTimeTicks;
    uint64_t sTimeTicks;
    stat >> uTimeTicks >> sTimeTicks;
    if (!stat.good()) {
        throw SystemException("Error reading /proc/childPid_/stat");
    }

    ProcessTimeUsage result;
    result.uTimeUs = uTimeTicks * 1000000 / CLOCK_TICKS_PER_SECOND;
    result.sTimeUs = sTimeTicks * 1000000 / CLOCK_TICKS_PER_SECOND;
    return result;
}


} // namespace limits
} // namespace s2j

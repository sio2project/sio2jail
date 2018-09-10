#include "OITimeToolOutputBuilder.h"

#include <sstream>

namespace s2j {
namespace printer {

const std::string OITimeToolOutputBuilder::FORMAT_NAME = "oitt";

OITimeToolOutputBuilder::OITimeToolOutputBuilder()
    : milliSecondsElapsed_(0), memoryPeakKb_(0), syscallsCounter_(0), exitStatus_(0), killSignal_(0) {}

OutputBuilder& OITimeToolOutputBuilder::setCyclesUsed(uint64_t cyclesUsed) {
    milliSecondsElapsed_ = cyclesUsed * 1'000 / CYCLES_PER_SECOND;
    return *this;
}

OutputBuilder& OITimeToolOutputBuilder::setMemoryPeak(uint64_t memoryPeakKb) {
    memoryPeakKb_ = memoryPeakKb;
    return *this;
}

OutputBuilder& OITimeToolOutputBuilder::setExitStatus(uint32_t exitStatus) {
    if (exitStatus_ == 0) {
        exitStatus_ = exitStatus;
    }
    return *this;
}

OutputBuilder& OITimeToolOutputBuilder::setKillSignal(uint32_t killSignal) {
    if (killSignal_ == 0) {
        killSignal_ = killSignal;
        setExitStatus(128 + killSignal_);
    }
    return *this;
}

OutputBuilder& OITimeToolOutputBuilder::setKillReason(KillReason reason, const std::string& comment) {
    // Remember only first kill reason
    if (killReason_ == KillReason::NONE) {
        killReason_ = reason;
        killReasonComment_ = comment;
    }

    return *this;
}

std::string OITimeToolOutputBuilder::dump() const {
    // mimic orginal oititmetools' output
    std::stringstream ss;
    ss << "__RESULT__ " << encodeStatusCode()
        << " " << milliSecondsElapsed_
        << " " << 0ULL
        << " " << memoryPeakKb_
        << " " << syscallsCounter_
        << std::endl;
    dumpStatus(ss);
    ss << std::endl;
    return ss.str();
}

void OITimeToolOutputBuilder::dumpStatus(std::ostream& ss) const {
    if (killReason_ != KillReason::NONE) {
            ss << killReasonComment_;
    }
    else if (killSignal_ > 0) {
        ss << "process exited due to signal " << killSignal_;
    }
    else if (exitStatus_ > 0) {
        ss << "runtime error " << exitStatus_;
    }
    else {
        ss << "ok";
    }
}

int OITimeToolOutputBuilder::encodeStatusCode() const {
    static const int CODES[] = {
        [KillReason::NONE] = 0,
        [KillReason::RE] = 100,
        [KillReason::RV] = 121,
        [KillReason::TLE] = 125,
        [KillReason::MLE] = 124,
        [KillReason::OLE] = 120,
    };
    static const int CODE_SIG_BASE = 0;
    static const int CODE_RE_BASE = 200;

    if (killReason_ == KillReason::NONE) {
        // NOTE: order of ifs is important, as nonzero killSignal also sets exitStatus_
        if (killSignal_ > 0) {
            return CODE_SIG_BASE + killSignal_;
        } else if (exitStatus_ > 0) {
            return CODE_RE_BASE + exitStatus_;
        }
    }
    return CODES[killReason_];
}

}
}

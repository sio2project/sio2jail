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
        if (exitStatus > 128)
            setKillSignal(exitStatus - 128);
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

OutputBuilder& OITimeToolOutputBuilder::setKillReason(const std::string& reason) {
    // Remember only first kill reason
    if (killReason_.empty())
        killReason_ = reason;
    return *this;
}

std::string OITimeToolOutputBuilder::dump() const {
    // mimic orginal oititmetools' output
    std::stringstream ss;
    ss << "__RESULT__ " << exitStatus_
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
    if (!killReason_.empty()) {
        ss << killReason_;
    }
    else if (killSignal_ > 0) {
        ss << "process exited due to signal " << killSignal_;
    }
    else {
        ss << "ok";
    }
}

}
}

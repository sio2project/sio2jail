#include "RealTimeOIOutputBuilder.h"

#include <sstream>

namespace s2j {
namespace printer {

const std::string RealTimeOIOutputBuilder::FORMAT_NAME = "oireal";

std::string RealTimeOIOutputBuilder::dump() const {
    KillReason reason = killReason_;
    if (reason == KillReason::NONE) {
        if (killSignal_ > 0 || exitStatus_ > 0) {
            reason = KillReason::RE;
        }
    }

    std::stringstream ss;
    ss << killReasonName(reason) << " " << exitStatus_ << " "
       << milliSecondsElapsed_ << " " << realMilliSecondsElapsed_ << " " 
       << 0ULL << " " << memoryPeakKb_ << " "
       << syscallsCounter_ << std::endl;
    dumpStatus(ss);
    ss << std::endl;
    return ss.str();
}

void RealTimeOIOutputBuilder::dumpStatus(std::ostream& ss) const {
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

} // namespace printer
} // namespace s2j

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
       << realMilliSecondsElapsed_ << " " << 0ULL << " " << memoryPeakKb_ << " "
       << syscallsCounter_ << std::endl;
    dumpStatus(ss);
    ss << std::endl;
    return ss.str();
}

} // namespace printer
} // namespace s2j

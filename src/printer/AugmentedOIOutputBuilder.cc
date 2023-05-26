#include "AugmentedOIOutputBuilder.h"

#include <sstream>

namespace s2j {
namespace printer {

const std::string AugmentedOIOutputBuilder::FORMAT_NAME = "oiaug";

std::string AugmentedOIOutputBuilder::dump() const {
    KillReason reason = killReason_;
    if (reason == KillReason::NONE) {
        if (killSignal_ > 0 || exitStatus_ > 0) {
            reason = KillReason::RE;
        }
    }

    std::stringstream ss;
    ss << killReasonName(reason) << " " << exitStatus_ << " "
       << milliSecondsElapsed_ << " " << 0ULL << " " << memoryPeakKb_ << " "
       << syscallsCounter_ << std::endl;
    dumpStatus(ss);
    ss << std::endl;
    return ss.str();
}

} // namespace printer
} // namespace s2j

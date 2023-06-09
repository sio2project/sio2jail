#include "UserTimeOIOutputBuilder.h"
#include "common/Exception.h"

#include <sstream>

namespace s2j {
namespace printer {

const std::string UserTimeOIOutputBuilder::FORMAT_NAME = "oiuser";

std::string UserTimeOIOutputBuilder::dump() const {
    KillReason reason = killReason_;
    if (reason == KillReason::NONE) {
        if (killSignal_ > 0 || exitStatus_ > 0) {
            reason = KillReason::RE;
        }
    }

    std::stringstream ss;
    ss << killReasonName(reason) << " " << exitStatus_ << " "
       << userMilliSecondsElapsed_ << " " << 0ULL << " " << memoryPeakKb_ << " "
       << syscallsCounter_ << std::endl;
    dumpStatus(ss);
    ss << std::endl;
    return ss.str();
}

} // namespace printer
} // namespace s2j

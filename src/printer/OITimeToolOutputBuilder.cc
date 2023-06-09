#include "OITimeToolOutputBuilder.h"
#include "common/Exception.h"

#include <sstream>

namespace s2j {
namespace printer {

const std::string OITimeToolOutputBuilder::FORMAT_NAME = "oitt";

std::string OITimeToolOutputBuilder::dump() const {
    // mimic orginal oititmetools' output
    std::stringstream ss;
    ss << "__RESULT__ " << encodeStatusCode() << " " << milliSecondsElapsed_
       << " " << 0ULL << " " << memoryPeakKb_ << " " << syscallsCounter_
       << std::endl;
    dumpStatus(ss);
    ss << std::endl;
    return ss.str();
}

int OITimeToolOutputBuilder::encodeStatusCode() const {
    static const int CODE_SIG_BASE = 0;
    static const int CODE_RE_BASE = 200;

    if (killReason_ == KillReason::NONE) {
        // NOTE: order of ifs is important, as nonzero killSignal also sets
        // exitStatus_
        if (killSignal_ > 0) {
            return CODE_SIG_BASE + killSignal_;
        }
        if (exitStatus_ > 0) {
            return CODE_RE_BASE + exitStatus_;
        }
    }

    switch (killReason_) {
    case KillReason::NONE:
        return 0;
    case KillReason::RE:
        return 100;
    case KillReason::RV:
        return 121;
    case KillReason::TLE:
        return 125;
    case KillReason::MLE:
        return 124;
    case KillReason::OLE:
        return 120;
    };
    __builtin_unreachable();
}

} // namespace printer
} // namespace s2j

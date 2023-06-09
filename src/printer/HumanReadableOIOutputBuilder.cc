#include "HumanReadableOIOutputBuilder.h"

#include <sstream>

namespace s2j {
namespace printer {

const std::string HumanReadableOIOutputBuilder::FORMAT_NAME = "human";

std::string HumanReadableOIOutputBuilder::dump() const {
    // This is inspired by the oiejq script
    std::stringstream ss;
    ss << std::endl << "-------------------------" << std::endl << "Result: ";
    dumpStatus(ss);
    ss << std::endl
       << "Time used: " << static_cast<float>(milliSecondsElapsed_) / 1000
       << "s" << std::endl
       << "Memory used: " << memoryPeakKb_ / 1024 << "MiB" << std::endl;
    return ss.str();
}

} // namespace printer
} // namespace s2j

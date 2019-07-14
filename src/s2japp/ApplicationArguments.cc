#include "ApplicationArguments.h"
#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <tclap/CmdLine.h>


/**
 * This is tiny bit repetitive. Possibly could be replaced by one "fun" template
 */

namespace s2j {
namespace app {
namespace args {

const std::array<std::string, 4> MemoryArgument::sizes = {"b", "k", "m", "g"};

MemoryArgument& MemoryArgument::operator=(const std::string& str) {
    std::string istr = str;
    std::transform(str.begin(), str.end(), istr.begin(), ::tolower);
    std::istringstream iss(istr);
    std::string unit;

    if (!(iss >> value_)) {
        throw TCLAP::ArgParseException(str + " is not a valid memory limit");
    }
    if (!(iss >> unit)) {
        unit = "k";
    }

    // std::array::size is constexpr
    for (size_t i = 0; i < sizes.size(); ++i) {
        if (sizes[i] == unit) {
            return *this;
        }
        value_ *= 1024;
    }
    throw TCLAP::ArgParseException(unit + " is not a valid memory unit");
}

const std::array<std::string, 6> TimeArgument::sizes =
        {"u", "ms", "s", "m", "h", "d"};
const std::array<uint64_t, 6> TimeArgument::multipliers =
        {1000, 1000, 60, 60, 60, 24};

TimeArgument& TimeArgument::operator=(const std::string& str) {
    std::string istr = str;
    std::transform(str.begin(), str.end(), istr.begin(), ::tolower);
    std::istringstream iss(istr);
    std::string unit;

    if (!(iss >> value_)) {
        throw TCLAP::ArgParseException(str + " is not a valid time limit");
    }
    if (!(iss >> unit)) {
        unit = "u";
    }

    // std::array::size is constexpr
    for (size_t i = 0; i < sizes.size(); ++i) {
        if (sizes[i] == unit) {
            return *this;
        }
        value_ *= multipliers[i];
    }
    throw TCLAP::ArgParseException(unit + " is not a valid time unit");
}

const std::array<std::string, 4> AmountArgument::sizes = {"", "k", "m", "g"};

AmountArgument& AmountArgument::operator=(const std::string& str) {
    std::string istr = str;
    std::transform(str.begin(), str.end(), istr.begin(), ::tolower);
    std::istringstream iss(istr);
    std::string unit;

    if (!(iss >> value_)) {
        throw TCLAP::ArgParseException(
                str + " is not a valid amount specifier");
    }
    if (!(iss >> unit)) {
        unit = "";
    }

    // std::array::size is constexpr
    for (size_t i = 0; i < sizes.size(); ++i) {
        if (sizes[i] == unit) {
            return *this;
        }
        value_ *= 1000;
    }
    throw TCLAP::ArgParseException(unit + " is not a valid amount unit");
}

} // namespace args
} // namespace app
} // namespace s2j

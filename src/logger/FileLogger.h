#pragma once

#include "FDLogger.h"

#include <fstream>

namespace s2j {
namespace logger {

class FileLogger : public FDLogger {
public:
    FileLogger(const std::string& fileName = "/dev/null");
};

} // namespace logger
} // namespace s2j

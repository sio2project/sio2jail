#include "FileLogger.h"

#include "common/Exception.h"

#include <fcntl.h>

namespace s2j {
namespace logger {

FileLogger::FileLogger(const std::string& fileName)
        : FDLogger(
                  open(fileName.c_str(),
                       O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC,
                       S_IRUSR | S_IWUSR | S_IRGRP),
                  true) {}

} // namespace logger
} // namespace s2j

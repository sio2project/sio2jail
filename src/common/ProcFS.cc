#include "ProcFS.h"

#include <map>
#include <string>
#include <fstream>
#include <functional>

namespace {

struct FieldReader {
    std::string label;
    std::function<uint64_t(const std::string&)> read;
};

auto stoull = static_cast<unsigned long long(*)(const std::string&, std::size_t*, int)>(std::stoull);
const std::map<s2j::procfs::Field, FieldReader> FIELD_READERS = {
    {s2j::procfs::Field::VM_PEAK, FieldReader{"VmPeak", std::bind(stoull, std::placeholders::_1, nullptr, 10)}},
    {s2j::procfs::Field::VM_SIZE, FieldReader{"VmSize", std::bind(stoull, std::placeholders::_1, nullptr, 10)}},
    {s2j::procfs::Field::SIG_CGT, FieldReader{"SigCgt", std::bind(stoull, std::placeholders::_1, nullptr, 16)}}
};

}

namespace s2j {
namespace procfs {

uint64_t readProcFS(pid_t pid, Field field, uint64_t defaultValue) {
    // Read tracee vmpeak from /proc/pid/status
    std::ifstream status("/proc/" + std::to_string(pid) + "/status");
    if (!status.good()) {
        return defaultValue;
    }

    const auto& fieldReader = FIELD_READERS.at(field);

    std::string statusLine;
    while (std::getline(status, statusLine)) {
        if (statusLine.compare(0, fieldReader.label.size(), fieldReader.label) == 0) {
            auto valuePos = statusLine.find_first_not_of(" \t", fieldReader.label.size() + 1);
            return fieldReader.read(statusLine.substr(valuePos));
        }
    }

    return defaultValue;
}

}
}

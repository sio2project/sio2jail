#include "Utils.h"
#include "Exception.h"
#include "WithErrnoCheck.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>

#include <sys/stat.h>
#include <sys/types.h>

namespace s2j {

char* stringToCStr(const std::string& str) {
    char* cStr = new char[str.size() + 1];
    std::copy(str.begin(), str.end(), cStr);
    cStr[str.size()] = '\0';
    return cStr;
}

std::vector<std::string> split(
        const std::string& str,
        const std::string& delimeter) {
    std::vector<std::string> tokens;
    std::string::size_type pos = 0, end;
    while ((end = str.find(delimeter, pos)) != std::string::npos) {
        tokens.emplace_back(str.substr(pos, end - pos));
        pos = end + delimeter.size();
    }
    if (pos < str.size() && pos != std::string::npos) {
        tokens.emplace_back(str.substr(pos));
    }
    return tokens;
}

std::string createTemporaryDirectory(const std::string& directoryTemplate) {
    char directory[directoryTemplate.size() + 1];
    strncpy(directory, directoryTemplate.c_str(), sizeof(directory));
    char* res = mkdtemp(directory);
    if (res == nullptr)
        throw SystemException("mkdtemp failed");
    return directory;
}

bool checkKernelVersion(int major, int minor) {
    std::ifstream verfile("/proc/sys/kernel/osrelease");
    if (!verfile.good()) {
        return false;
    }

    std::string verstr;
    if (!std::getline(verfile, verstr, '-')) {
        return false;
    }

    auto versplit = split(verstr, ".");
    std::vector<int> verints;
    for (auto s: versplit) {
        verints.push_back(std::stoi(s));
    }

    if (verints[0] < major) {
        return false;
    }
    else if (verints[0] > major) {
        return true;
    }

    return verints[1] >= minor;
}

} // namespace s2j

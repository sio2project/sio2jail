#pragma once

#include <string>

namespace s2j {
namespace ns {

class MountEventListener {
public:
    virtual void onProgramNameChange(const std::string& newProgramName){};
};

} // namespace ns
} // namespace s2j

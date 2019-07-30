#pragma once

#include "ExecuteEventListener.h"

#include "common/EventProvider.h"
#include "ns/MountEventListener.h"
#include "printer/OutputSource.h"

#include <memory>
#include <vector>

namespace s2j {
namespace executor {

class Executor
        : public s2j::printer::OutputSource
        , public s2j::ns::MountEventListener
        , public EventProvider<ExecuteEventListener> {
public:
    Executor(
            std::string childProgramName,
            std::vector<std::string> childProgramArgv,
            bool supportThreads = false);

    template<typename ProgramNameType>
    void setChildProgramName(ProgramNameType&& programName) {
        childProgramName_ = std::forward<ProgramNameType>(programName);
    }

    template<typename ProgramArgvType>
    void setChildProgramArgv(ProgramArgvType&& programArgv) {
        childProgramArgv_ = std::forward<ProgramArgvType>(programArgv);
    }

    void onProgramNameChange(const std::string& newProgramName);

    void execute();

private:
    void executeChild();
    void executeParent();
    void setupSignalHandling();
    void killChild();
    ExecuteAction checkSignals();

    std::string childProgramName_;
    std::vector<std::string> childProgramArgv_;

    pid_t childPid_;
    const bool supportThreads_;
};

} // namespace executor
} // namespace s2j

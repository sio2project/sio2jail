#pragma once

#include "common/Feature.h"
#include "ns/MountNamespaceListener.h"
#include "printer/OutputBuilder.h"
#include "seccomp/policy/SyscallPolicy.h"

#include "common/Utils.h"

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace s2j {
namespace app {

struct ApplicationSettings : public ns::MountNamespaceListener::Settings {
    enum class Action { PRINT_HELP, PRINT_VERSION, RUN };

    ApplicationSettings();
    ApplicationSettings(int argc, const char* argv[]);

    static const std::string VERSION;
    static const std::string DESCRIPTION;
    static const FactoryMap<s2j::printer::OutputBuilder> OUTPUT_FORMATS;
    static const std::string DEFAULT_OUTPUT_FORMAT;
    static const FactoryMap<s2j::seccomp::policy::BaseSyscallPolicy>
            SYSCALL_POLICIES;
    static const std::string DEFAULT_SYSCALL_POLICY;
    static const std::map<std::string, std::pair<Feature, bool>>
            FEATURE_BY_NAME;

    Action action;
    std::string loggerPath;

    uint64_t memoryLimitKb{};
    uint64_t outputLimitB{};
    uint64_t instructionCountLimit{};
    // [us] - microseconds, 10^(-6) s
    uint64_t rTimelimitUs{};
    uint64_t uTimelimitUs{};
    uint64_t sTimelimitUs{};
    uint64_t usTimelimitUs{};

    int resultsFD{};
    int threadsLimit{};
    uint32_t perfOversamplingFactor{};

    std::string parsingError;
    std::string helpMessage;
    std::string versionMessage;

    std::string programName;
    std::vector<std::string> programArgv;
    std::string programWorkingDir;

    Factory<s2j::printer::OutputBuilder> outputBuilderFactory;
    Factory<s2j::seccomp::policy::BaseSyscallPolicy> syscallPolicyFactory;
    std::set<Feature> features;

    bool suppressStderr{};

private:
    static const std::vector<std::string> FLAGS_ON, FLAGS_OFF;

    void addBindMount(const std::string& bindMountLine);
};

} // namespace app
} // namespace s2j

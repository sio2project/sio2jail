#pragma once

#include "common/Feature.h"
#include "printer/OutputBuilder.h"
#include "ns/MountNamespaceListener.h"

#include <string>
#include <vector>
#include <memory>
#include <set>
#include <map>

namespace s2j {
namespace app {

struct ApplicationSettings : public ns::MountNamespaceListener::Settings {
    enum class Action {
        PRINT_HELP,
        PRINT_VERSION,
        RUN
    };

    ApplicationSettings();
    ApplicationSettings(int argc, const char* argv[]);

    std::shared_ptr<s2j::printer::OutputBuilder> createOutputBuilder() const;

    static const std::string VERSION;
    static const std::string DESCRIPTION;
    static const std::vector<std::string> OUTPUT_FORMATS_NAMES;
    static const std::map<std::string, std::pair<Feature, bool>> FEATURE_BY_NAME;

    Action action;
    std::string loggerPath;

    uint64_t memoryLimitKb;
    uint64_t outputLimitB;
    uint64_t instructionCountLimit;
    // [us] - microseconds, 10^(-6) s
    uint64_t rTimelimitUs;
    uint64_t uTimelimitUs;
    uint64_t sTimelimitUs;
    uint64_t usTimelimitUs;

    int resultsFD;

    std::string parsingError;
    std::string helpMessage;
    std::string versionMessage;

    std::string programName;
    std::vector<std::string> programArgv;

    std::string outputFormatName;
    std::set<Feature> features;

    bool suppressStderr;

private:
    static const std::vector<std::string> FLAGS_ON, FLAGS_OFF;

    void addBindMount(const std::string& bindMountLine);
};

}
}

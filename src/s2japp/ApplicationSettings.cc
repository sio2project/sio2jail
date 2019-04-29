#include "ApplicationSettings.h"
#include "ApplicationException.h"
#include "ApplicationArguments.h"

#include "printer/OITimeToolOutputBuilder.h"
#include "printer/AugmentedOIOutputBuilder.h"
#include "seccomp/policy/DefaultPolicy.h"
#include "seccomp/policy/PermissivePolicy.h"
#include "common/Utils.h"

#include <seccomp.h>
#include <sstream>

#include <tclap/CmdLine.h>

#include <linux/version.h>

namespace {

/* Simple wrapper around TCLAP::StdOutput thats saves messages in an
 * ApplicationSettings object instead of printing them to stdout. It
 * allows more flexible error handling.
 */
class StringOutputGenerator : public TCLAP::StdOutput {
public:
    StringOutputGenerator(s2j::app::ApplicationSettings& settings)
        : settings_(settings), hasFailure_(false) {}

    void usage(TCLAP::CmdLineInterface& cmd) override {
        std::stringstream ss;

        ss << "Usage: " << std::endl;
        _shortUsage(cmd, ss);
        ss << std::endl << "Where: " << std::endl;
        _longUsage(cmd, ss);

        settings_.helpMessage = ss.str();
    }

    void failure(const TCLAP::ArgException& ex) {
        hasFailure_ = true;
        settings_.parsingError = ex.error();
    }

    void failure(TCLAP::CmdLineInterface& cmd, TCLAP::ArgException& ex) override {
        failure(ex);
    }

    void version(TCLAP::CmdLineInterface& cmd) override {
        std::stringstream ss;

        ss << "SIO2jail v" << cmd.getVersion()
            << " compiled on " << __DATE__ << " " << __TIME__;
#ifdef BUILD_KERNEL_RELEASE
        ss << " Linux " << BUILD_KERNEL_RELEASE;
#endif
        ss << "with gcc "
            << __VERSION__ << std::endl;

        const struct scmp_version* seccompVersion = seccomp_version();
        ss << "libseccomp "
            << seccompVersion->major << "."
            << seccompVersion->minor << "."
            << seccompVersion->micro << std::endl;

        settings_.versionMessage = ss.str();
    }

    bool hasFailure() const {
        return hasFailure_;
    }

private:
    s2j::app::ApplicationSettings& settings_;

    bool hasFailure_;
};

}

namespace s2j {
namespace app {

const std::string ApplicationSettings::VERSION = "1.2.0";

const std::string ApplicationSettings::DESCRIPTION = "SIO2jail, a sandbox for programming contests.";

const FactoryMap<s2j::printer::OutputBuilder> ApplicationSettings::OUTPUT_FORMATS({
        {"oitt", std::make_shared<s2j::printer::OITimeToolOutputBuilder>},
        {"oiaug", std::make_shared<s2j::printer::AugmentedOIOutputBuilder>}
        });
const std::string ApplicationSettings::DEFAULT_OUTPUT_FORMAT = "oitt";

const FactoryMap<s2j::seccomp::policy::BaseSyscallPolicy> ApplicationSettings::SYSCALL_POLICIES({
        {"default", std::make_shared<s2j::seccomp::policy::DefaultPolicy>},
        {"permissive", std::make_shared<s2j::seccomp::policy::PermissivePolicy>}
        });
const std::string ApplicationSettings::DEFAULT_SYSCALL_POLICY = "default";

const std::map<std::string, std::pair<Feature, bool>> ApplicationSettings::FEATURE_BY_NAME({
        {"ptrace",          {Feature::PTRACE, true}},
        {"perf",            {Feature::PERF, true}},
        {"seccomp",         {Feature::SECCOMP, true}},
        {"pid-namespace",   {Feature::PID_NAMESPACE, true}},
        {"net-namespace",   {Feature::NET_NAMESPACE, true}},
        {"ipc-namespace",   {Feature::IPC_NAMESPACE, true}},
        {"uts-namespace",   {Feature::UTS_NAMESPACE, true}},
        {"user-namespace",  {Feature::USER_NAMESPACE, true}},
        {"mount-namespace", {Feature::MOUNT_NAMESPACE, true}},
        {"procfs",          {Feature::MOUNT_PROCFS, false}},
        {"capability-drop", {Feature::CAPABILITY_DROP, true}}});

const std::vector<std::string> ApplicationSettings::FLAGS_ON({"on", "yes", "1"});
const std::vector<std::string> ApplicationSettings::FLAGS_OFF({"off", "no", "0"});

ApplicationSettings::ApplicationSettings()
    : action(Action::PRINT_HELP)
    , outputBuilderFactory([](){ return nullptr; })
    , syscallPolicyFactory([](){ return nullptr; }) {}

ApplicationSettings::ApplicationSettings(int argc, const char* argv[])
    : ApplicationSettings() { StringOutputGenerator outputGenerator(*this);
    try {
        TCLAP::CmdLine cmd(DESCRIPTION, ' ', VERSION, false);
        cmd.setExceptionHandling(true);
        cmd.setOutput(&outputGenerator);

        TCLAP::SwitchArg argPrintHelp("h", "help", "Print this help message", cmd, false);
        TCLAP::SwitchArg argPrintVersion("v", "version", "Print version", cmd, false);

        std::list<std::pair<Feature, std::unique_ptr<TCLAP::ValueArg<std::string>>>> argsFeatures;
        for (const auto featureIter: FEATURE_BY_NAME) {
            argsFeatures.emplace_back(featureIter.second.first, std::make_unique<TCLAP::ValueArg<std::string>>(
                        "", featureIter.first, "Control feature " + featureIter.first, false, (featureIter.second.second ? "on" : "off"), "on|off", cmd));
        }

        args::ImplementationNameArgument<s2j::printer::OutputBuilder> outputFormat("output format", DEFAULT_OUTPUT_FORMAT, OUTPUT_FORMATS);
        TCLAP::ValueArg<decltype(outputFormat)> argOutputFormat("o", "output", "Output format", false, outputFormat, &outputFormat, cmd);

        args::ImplementationNameArgument<s2j::seccomp::policy::BaseSyscallPolicy> syscallPolicy("syscall policy", DEFAULT_SYSCALL_POLICY, SYSCALL_POLICIES);
        TCLAP::ValueArg<decltype(syscallPolicy)> argSyscallPolicy("p", "policy", "Syscall policy", false, syscallPolicy, &syscallPolicy, cmd);

        TCLAP::ValueArg<args::MemoryArgument> argMemoryLimit("m", "memory-limit",
                "Memory limit. Use with K,M,G sufixes (case-insensitive) for 1024**{1,2,3} bytes respectively. "
                "Default is kilobytes. Use 0 for no limit.",
                false, args::MemoryArgument(), "string", cmd);

        TCLAP::ValueArg<args::MemoryArgument> argOutputLimit("", "output-limit",
                "Output file size limit. Use with K,M,G sufixes (case-insensitive) for 1024**{1,2,3} bytes respectively. "
                "Default is kilobytes. Use 0 for no limit.",
                false, args::MemoryArgument(), "string", cmd);

        TCLAP::SwitchArg argShowStderr("s", "stderr", "Pass stderr to console", cmd, false);
        TCLAP::ValueArg<int> argResultsFD("f", "resultsfd", "File descriptor to write results to", false, 2 /* stderr */, "fd", cmd);

        TCLAP::ValueArg<args::AmountArgument> argInstructionCountLimit("", "instruction-count-limit",
                "Instruction count limit. Use with k,m,g sufixes for 10**{3,6,9} respectively. Use 0 for no limit",
                false, args::AmountArgument(), "amount specifier", cmd);

        TCLAP::ValueArg<args::TimeArgument> argRtimelimit("", "rtimelimit",
                "Real time limit. Use with u,ms,s,m,h,d sufixes (case-insensitive) for microseconds, miliseconds, seconds, minutes, "
                "hours and days respectively. Defaults to microseconds. Use 0 for no limit",
                false, args::TimeArgument(), "time limit", cmd);

        TCLAP::ValueArg<args::TimeArgument> argUtimelimit("", "utimelimit",
                "User time limit. Use with u,ms,s,m,h,d sufixes (case-insensitive) for microseconds, miliseconds, seconds, minutes, "
                "hours and days respectively. Defaults to microseconds. Use 0 for no limit",
                false, args::TimeArgument(), "time limit", cmd);

        TCLAP::ValueArg<args::TimeArgument> argStimelimit("", "stimelimit",
                "System time limit. Use with u,ms,s,m,h,d sufixes (case-insensitive) for microseconds, miliseconds, seconds, minutes, "
                "hours and days respectively. Defaults to microseconds. Use 0 for no limit",
                false, args::TimeArgument(), "time limit", cmd);

        TCLAP::ValueArg<args::TimeArgument> argUStimelimit("", "ustimelimit",
                "User+System time limit. Use with u,ms,s,m,h,d sufixes (case-insensitive) for microseconds, miliseconds, seconds, "
                "minutes, hours and days respectively. Defaults to microseconds. Use 0 for no limit",
                false, args::TimeArgument(), "time limit", cmd);

        TCLAP::MultiArg<std::string> argBindMounts("b", "bind", "Bind mount path:path_inside_jail[:(rw|ro)]", false, "string", cmd);

        TCLAP::SwitchArg argNoDefaultBinds("B", "no-default-binds", "Don't add default bind-mounts (i.e. binding the executable at /exe)", cmd, false);

        TCLAP::ValueArg<std::string> argLoggerPath("l", "log", "Logging file, use - for stderr", false, "", "path", cmd);

        TCLAP::UnlabeledValueArg<std::string> argProgramName("path", "Name of program to run", true, "", "path", cmd);
        TCLAP::UnlabeledMultiArg<std::string> argProgramArgv("argv", "Arguments of supervised program", false, "argv", cmd);


        cmd.parse(argc, argv);

        // Force generation of usage and version messages
        outputGenerator.usage(cmd);
        outputGenerator.version(cmd);

        // If there is -v or -h flag ignore any other arguments
        if (argPrintHelp.isSet() || argPrintVersion.isSet())
            parsingError = "";

        if (argPrintHelp.isSet()) {
            action = Action::PRINT_HELP;
        }
        else if (argPrintVersion.isSet()) {
            action = Action::PRINT_VERSION;
        }
        else if (!outputGenerator.hasFailure()) {
            action = Action::RUN;
        }
        

        // This is in bytes underneath, so we divide
        memoryLimitKb = argMemoryLimit.getValue() / 1024;
        outputLimitB = argOutputLimit.getValue();

        programName = argProgramName.getValue();
        programArgv = argProgramArgv.getValue();

        outputBuilderFactory = argOutputFormat.getValue().getFactory();
        syscallPolicyFactory = argSyscallPolicy.getValue().getFactory();

        loggerPath = argLoggerPath.getValue();

        for (auto& argFeature: argsFeatures) {
            if (std::find(FLAGS_ON.begin(), FLAGS_ON.end(), argFeature.second->getValue()) != FLAGS_ON.end()) {
                features.insert(argFeature.first);
            }
            else if (std::find(FLAGS_OFF.begin(), FLAGS_OFF.end(), argFeature.second->getValue()) != FLAGS_OFF.end()) {
                features.erase(argFeature.first);
            }
            else {
                throw TCLAP::CmdLineParseException(
                        "Bad value '" + argFeature.second->getValue() + "' for argument " + argFeature.second->longID(),
                        argFeature.second->longID());
            }
        }

        for (auto& bindMount: argBindMounts) {
            addBindMount(bindMount);
        }

        bindExecutable = !argNoDefaultBinds.getValue();

        if (argInstructionCountLimit.isSet() && !features.count(Feature::PERF)) {
            throw InvalidConfigurationException("Instruction count limit can only be used if PERF is enabled");
        }

        instructionCountLimit = argInstructionCountLimit.getValue();
        rTimelimitUs = argRtimelimit.getValue();
        uTimelimitUs = argUtimelimit.getValue();
        sTimelimitUs = argStimelimit.getValue();
        usTimelimitUs = argUStimelimit.getValue();

        suppressStderr = !argShowStderr.getValue();
        resultsFD = argResultsFD.getValue();
    }
    catch (const TCLAP::ArgException& ex) {
        outputGenerator.failure(ex);
        action = Action::PRINT_HELP;
    }
    catch (const TCLAP::ExitException& ex) {
        action = Action::PRINT_HELP;
    }
}

void ApplicationSettings::addBindMount(const std::string& bindMountLine) {
    auto tokens = split(bindMountLine, ":");
    if (tokens.size() < 2 || tokens.size() > 3) {
        throw InvalidConfigurationException("Invalid bind mount specification: " + bindMountLine);
    }

    ns::MountNamespaceListener::BindMount bindMount{tokens[0], tokens[1], ns::MountNamespaceListener::BindMount::Mode::RO};
    if (tokens.size() == 3) {
        auto flags = split(tokens[2], ",");
        if (flags.size() < 1) {
            throw InvalidConfigurationException("Empty bind mount mode: " + tokens[2]);
        }
        if (flags[0] == "ro") {
            bindMount.mode = ns::MountNamespaceListener::BindMount::Mode::RO;
        }
        else if (flags[0] == "rw") {
            bindMount.mode = ns::MountNamespaceListener::BindMount::Mode::RW;
        }
        else {
            throw InvalidConfigurationException("No such bind mount mode: " + flags[0]);
        }
        if (flags.size() >= 2) {
            if (flags[1] == "dev") {
                bindMount.dev = true;
            } else {
                throw InvalidConfigurationException("No such bind mount flag: " + flags[1]);
            }
        }
    }

    bindMounts.emplace_back(std::move(bindMount));
}

}
}

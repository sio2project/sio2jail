#pragma once

#include "MountEventListener.h"

#include "common/Feature.h"
#include "common/EventProvider.h"
#include "printer/OutputSource.h"
#include "executor/ExecuteEventListener.h"

#include <tclap/CmdLine.h>

#include <string>

namespace s2j {
namespace ns {

/**
 * Makes the child run in a separate PID namespace.
 * Requires CAP_SYS_ADMIN.
 *
 * Note: it'd be way more useful if the child
 * didn't have access to parent's /proc
 */
class MountNamespaceListener : public executor::ExecuteEventListener,
                               public EventProvider<MountEventListener> {
public:
    struct BindMount {
        enum class Mode {
            RO, RW
        };

        BindMount() {}

        BindMount(const std::string& sourcePath, const std::string& targetPath, Mode mode)
            : sourcePath(sourcePath), targetPath(targetPath), mode(mode) {}

        uint32_t flags() const;
        void mount(const std::string& root);
        void umount(const std::string& root);

        std::string sourcePath, targetPath;
        Mode mode;
        bool dev = false;
    };

    struct Settings {
        std::vector<BindMount> bindMounts;
        bool bindExecutable;
    };

    MountNamespaceListener(const Settings& settings, const std::string& executablePath, bool mountProc);

    void onPostForkChild() override;
    void onPostExecute() override;

    static const std::string newExecutablePath;
    static const Feature feature;

private:
    BindMount newRoot_;
    std::string executablePath_;
    std::vector<BindMount> bindMounts_;
    bool mountProc_;
    bool bindExecutable_;
};

}
}

#include "MountNamespaceListener.h"

#include "common/Exception.h"
#include "common/WithErrnoCheck.h"
#include "common/FD.h"
#include "common/Utils.h"
#include "logger/Logger.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <linux/sched.h>

namespace {

void pivot_root(const char *new_root, const char *put_old) {
    s2j::withErrnoCheck("pivot_root", syscall, SYS_pivot_root, new_root, put_old);
}

}

namespace s2j {
namespace ns {

const std::string MountNamespaceListener::newExecutablePath = "/exe";

const Feature MountNamespaceListener::feature = Feature::MOUNT_NAMESPACE;

MountNamespaceListener::MountNamespaceListener(const Settings& settings, const std::string& executablePath, const bool mountProc)
    : executablePath_(executablePath), bindMounts_(settings.bindMounts), mountProc_(mountProc), bindExecutable_(settings.bindExecutable) {
    if (bindExecutable_) {
        bindMounts_.emplace_back(BindMount(executablePath_, newExecutablePath, BindMount::Mode::RO));
    }

    if (bindMounts_.empty() || bindMounts_.front().targetPath != "/") {
        throw Exception("Invalid configration, first bind mount must be root bind mount if namespace listener is used");
    }
    newRoot_ = std::move(bindMounts_.front());
    bindMounts_.erase(bindMounts_.begin());
}

void MountNamespaceListener::onPostForkChild() {
    TRACE();

    withErrnoCheck("unshare mount namespace", unshare, CLONE_NEWNS);

    // make-private on everything
    withErrnoCheck("mount make-private", mount, "none", "/", nullptr, MS_PRIVATE|MS_REC, nullptr);

    const std::string& newRootPath = newRoot_.sourcePath;
    // bind-mount newRoot
    withErrnoCheck("mount bind " + newRootPath, mount, newRootPath.c_str(), newRootPath.c_str(), "", MS_BIND, nullptr);
    for (auto& bindMount: bindMounts_) {
        bindMount.mount(newRootPath);
    }

    // cd to newRoot
    withErrnoCheck("chdir " + newRootPath, chdir, newRootPath.c_str());

    // move to new root, and put old root on top
    pivot_root(".", ".");

    // chroot because pivot_root manpage says so
    withErrnoCheck("chroot", chroot, ".");

    if (bindExecutable_) {
        for (auto listener: EventProvider<MountEventListener>::eventListeners_)
            listener->onProgramNameChange(newExecutablePath);
    }

    if (mountProc_) {
        // mount new /proc
        //TODO: add a flag to disable this
        if (mkdir("proc", 0755) < 0) {
            if (errno != EEXIST) {
                throw SystemException("mkdir proc");
            }
        }
        withErrnoCheck("mount proc", mount, "proc", "proc", "proc", MS_NOSUID|MS_NODEV|MS_NOEXEC, nullptr);
    }

    // now detach the old root
    // NOTE: this needs to be done AFTER new proc is already mounted, unless we don't want proc at all
    withErrnoCheck("umount detach old root", umount2, "/", MNT_DETACH);
    withErrnoCheck("remount root rdolny", mount, "none", "/", nullptr, MS_REMOUNT|MS_BIND| newRoot_.flags(), nullptr);
}

void MountNamespaceListener::onPostExecute() {
    TRACE();
}

uint32_t MountNamespaceListener::BindMount::flags() const {
    uint32_t flags = MS_NOSUID;
    if (mode == Mode::RO) {
        flags |= MS_RDONLY;
    }
    if (!dev) {
        flags |= MS_NODEV;
    }
    return flags;
}

void MountNamespaceListener::BindMount::mount(const std::string& root) {
    TRACE(root);

    // NOTE: On bind mount, any flags other than MS_BIND and MS_REC are ignored by the kernel
    withErrnoCheck("bind mount " + sourcePath + " -> " + targetPath,
                   ::mount, sourcePath.c_str(), (root + "/" + targetPath).c_str(), "", MS_BIND, nullptr);
    // remount to apply flags
    withErrnoCheck("bind remount " + targetPath,
                   ::mount, "none", (root + "/" + targetPath).c_str(), nullptr, MS_REMOUNT | MS_BIND | flags(), nullptr);
}

void MountNamespaceListener::BindMount::umount(const std::string& root) {
    TRACE(root);

    withErrnoCheck("ummount bind mount" + sourcePath, umount2, (root + "/" + targetPath).c_str(), MNT_DETACH);
}

}
}

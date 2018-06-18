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
    : newRoot_(createTemporaryDirectory()), executablePath_(executablePath), bindMounts_(settings.bindMounts), mountProc_(mountProc) {
    bindMounts_.emplace_back(BindMount(executablePath_, newExecutablePath, BindMount::Mode::RO));

    if (bindMounts_.front().targetPath != "/") {
        throw Exception("Invalid configration, first bind mount must be root bind mount if namespace listener is used");
    }
}

void MountNamespaceListener::onPostForkChild() {
    TRACE();

    withErrnoCheck("unshare mount namespace", unshare, CLONE_NEWNS);

    // make-private on everything
    withErrnoCheck("mount make-private", mount, "none", "/", nullptr, MS_PRIVATE|MS_REC, nullptr);

    // bind-mount newRoot
    withErrnoCheck("mount bind " + newRoot_, mount, newRoot_.c_str(), newRoot_.c_str(), "", MS_BIND, nullptr);
    for (auto& bindMount: bindMounts_) {
        bindMount.mount(newRoot_);
    }

    // cd to newRoot
    withErrnoCheck("chdir " + newRoot_, chdir, newRoot_.c_str());

    // move to new root, and put old root on top
    pivot_root(".", ".");

    // chroot because pivot_root manpage says so
    withErrnoCheck("chroot", chroot, ".");

    for (auto listener: EventProvider<MountEventListener>::eventListeners_)
        listener->onProgramNameChange(newExecutablePath);

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
    withErrnoCheck("remount root rdolny", mount, "none", "/", nullptr, MS_REMOUNT|MS_BIND|MS_RDONLY|MS_NODEV|MS_NOSUID, nullptr);
}

void MountNamespaceListener::onPostExecute() {
    TRACE();

    withErrnoCheck("rmdir " + newRoot_, rmdir, newRoot_.c_str());
}

void MountNamespaceListener::BindMount::mount(const std::string& root) {
    TRACE(root);

    uint32_t flags = 0;
    if (mode == Mode::RO) {
        flags |= MS_RDONLY;
    }
    withErrnoCheck("bind mount " + sourcePath + " -> " + targetPath,
                   ::mount, sourcePath.c_str(), (root + "/" + targetPath).c_str(), "", MS_BIND | MS_NODEV | MS_NOSUID | flags, nullptr);
}

void MountNamespaceListener::BindMount::umount(const std::string& root) {
    TRACE(root);

    withErrnoCheck("ummount bind mount" + sourcePath, umount2, (root + "/" + targetPath).c_str(), MNT_DETACH);
}

}
}

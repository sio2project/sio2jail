#include "UserNamespaceListener.h"

#include "common/Exception.h"
#include "common/FD.h"
#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

namespace s2j {
namespace ns {

const Feature UserNamespaceListener::feature = Feature::USER_NAMESPACE;

UserNamespaceListener::UserNamespaceListener()
        : UserNamespaceListener(-1, -1) {}

UserNamespaceListener::UserNamespaceListener(
        uid_t rootOutsideUid,
        gid_t rootOutsideGid)
        : UserNamespaceListener(rootOutsideUid, rootOutsideGid, -1, -1) {}

UserNamespaceListener::UserNamespaceListener(
        uid_t rootOutsideUid,
        gid_t rootOutsideGid,
        uid_t childOutsideUid,
        gid_t childOutsideGid)
        : rootOutsideUid_(rootOutsideUid)
        , rootOutsideGid_(rootOutsideGid)
        , childOutsideUid_(childOutsideUid)
        , childOutsideGid_(childOutsideGid) {}

void UserNamespaceListener::onPreFork() {
    TRACE();

    uid_t uid = rootOutsideUid_;
    gid_t gid = rootOutsideGid_;

    // watch out, they're unsigned ;)
    if (uid == (uid_t) -1) {
        uid = getuid();
    }
    if (gid == (gid_t) -1) {
        gid = getgid();
    }

    withErrnoCheck("unshare newuser", unshare, CLONE_NEWUSER);

    writeUidGidMap("uid_map", uid, childOutsideUid_);
    writeSetGroups();
    writeUidGidMap("gid_map", gid, childOutsideGid_);
}

void UserNamespaceListener::writeSetGroups() {
    TRACE();

    FD::open("/proc/self/setgroups", O_WRONLY | O_CLOEXEC) << "deny";
}

void UserNamespaceListener::writeUidGidMap(
        std::string file,
        uid_t rootUid,
        uid_t childUid) {
    TRACE(file, rootUid, childUid);

    std::stringstream map;
    map << "0 " << rootUid << " 1\n";
    // watch out, this is an unsigned -1
    if (childUid != (uid_t) -1) {
        map << "1 " << childUid << " 1\n";
    }
    FD::open("/proc/self/" + file, O_WRONLY | O_CLOEXEC) << map.str();
}

} // namespace ns
} // namespace s2j

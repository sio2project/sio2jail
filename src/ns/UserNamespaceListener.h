#pragma once

#include "common/Feature.h"
#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"

#include <unistd.h>

namespace s2j {
namespace ns {

class UserNamespaceListener : public executor::ExecuteEventListener {
public:
    UserNamespaceListener();
    UserNamespaceListener(uid_t rootOutsideUid, gid_t rootOutsideGid);
    UserNamespaceListener(
            uid_t rootOutsideUid,
            gid_t rootOutsideGid,
            uid_t childOutsideUid,
            gid_t childOutsideGid);

    void onPreFork() override;

    const static Feature feature;

private:
    void writeSetGroups();
    void writeUidGidMap(std::string file, uid_t rootUid, uid_t childUid);

    const uid_t rootOutsideUid_;
    const gid_t rootOutsideGid_;
    const uid_t childOutsideUid_;
    const gid_t childOutsideGid_;
};

} // namespace ns
} // namespace s2j

#include "IPCNamespaceListener.h"

#include "common/Exception.h"
#include "common/WithErrnoCheck.h"

#include <linux/sched.h>

namespace s2j {
namespace ns {

const Feature IPCNamespaceListener::feature = Feature::IPC_NAMESPACE;

void IPCNamespaceListener::onPostForkChild() {
    TRACE();

    withErrnoCheck("unshare newipc", unshare, CLONE_NEWIPC);
}

}
}

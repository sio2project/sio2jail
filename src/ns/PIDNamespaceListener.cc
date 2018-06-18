#include "PIDNamespaceListener.h"

#include "common/Exception.h"
#include "common/WithErrnoCheck.h"

#include <linux/sched.h>

namespace s2j {
namespace ns {

const Feature PIDNamespaceListener::feature = Feature::PID_NAMESPACE;

void PIDNamespaceListener::onPreFork() {
    TRACE();

    withErrnoCheck("unshare newpid", unshare, CLONE_NEWPID);
}

}
}

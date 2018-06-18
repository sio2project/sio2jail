#include "NetNamespaceListener.h"

#include "common/Exception.h"
#include "common/WithErrnoCheck.h"

#include <linux/sched.h>

namespace s2j {
namespace ns {

const Feature NetNamespaceListener::feature = Feature::NET_NAMESPACE;

void NetNamespaceListener::onPostForkChild() {
    TRACE();

    withErrnoCheck("unshare newnet", unshare, CLONE_NEWNET);
}

}
}

#include "UTSNamespaceListener.h"

#include "common/Exception.h"
#include "common/WithErrnoCheck.h"

#include <unistd.h>
#include <linux/sched.h>

namespace s2j {
namespace ns {

const Feature UTSNamespaceListener::feature = Feature::UTS_NAMESPACE;

void UTSNamespaceListener::onPostForkChild() {
    TRACE();

    withErrnoCheck("unshare newuts", unshare, CLONE_NEWUTS);

    std::string hostname = "sio2jail";
    withErrnoCheck("set hostname", sethostname, hostname.c_str(), hostname.length());
    withErrnoCheck("set domainname", setdomainname, hostname.c_str(), hostname.length());
}

}
}

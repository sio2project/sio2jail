#include "PrivListener.h"

#include "common/Exception.h"
#include "common/Utils.h"
#include "common/WithErrnoCheck.h"
#include "logger/Logger.h"

#include <linux/securebits.h>
#include <string>
#include <sys/capability.h>
#include <sys/prctl.h>

#include <iostream>

namespace s2j {
namespace priv {

const Feature PrivListener::feature = Feature::CAPABILITY_DROP;

PrivListener::PrivListener()
        : secureBits_(withErrnoCheck(
                  "prct get secure bits",
                  prctl,
                  PR_GET_SECUREBITS)) {}

void PrivListener::onPostForkParent(pid_t /*childPid*/) {
    TRACE();

    dropBSet();
    noNewPrivs();
    dropCaps();
}

void PrivListener::onPostForkChild() {
    TRACE();

    dropBSet();
    noNewPrivs();
    dropCaps();
}

void PrivListener::dropBSet() {
    TRACE();

    for (int i = 0; i <= CAP_LAST_CAP; ++i) {
        try {
            withErrnoCheck(
                    "prctl drop cap " + std::to_string(i),
                    prctl,
                    PR_CAPBSET_DROP,
                    i,
                    0,
                    0,
                    0);
        }
        catch (s2j::SystemException& ex) {
            if (ex.getErrno() != EINVAL) {
                throw ex;
            }
        }
    }
}

void PrivListener::dropCaps() {
    TRACE();

    /* If we're uid=0, we MUST set secbit_noroot or fail with an error.
     * IOW, the following must NEVER happen:
     * uid == 0 && SECBIT_NOROOT == 0 && we're doing execve()
     * otherwise, there's no point dropping capabilities.
     */
    addSecureBits(SECBIT_NOROOT | SECBIT_NOROOT_LOCKED);

    __user_cap_header_struct header = {
            _LINUX_CAPABILITY_VERSION_3, // version
            0 // pid
    };

#ifdef PR_CAP_AMBIENT_CLEAR_ALL
    if (checkKernelVersion(4, 3)) {
        // clear ambient set, just in case
        withErrnoCheck(
                "clear ambient caps",
                prctl,
                PR_CAP_AMBIENT,
                PR_CAP_AMBIENT_CLEAR_ALL,
                0,
                0,
                0);
    }
    else {
        logger::warn("kernel older than 4.3, not clearing ambinet caps");
    }
#endif

    // version 3 - 64-bit capabilities
    __user_cap_data_struct caps[2] = {
            {
                    0, // effective
                    0, // permitted
                    0 // inheritable
            },
            {
                    0, // effective
                    0, // permitted
                    0 // inheritable
            },
    };
    withErrnoCheck("capset", capset, &header, caps);
}

void PrivListener::noNewPrivs() {
    TRACE();

    addSecureBits(SECBIT_NOROOT | SECBIT_NOROOT_LOCKED);
#ifdef SECBIT_NO_CAP_AMBIENT_RAISE // Present since linux 4.3
    // Apparently, some distros have CAP_AMBINET_RAISE in headers even on
    // pre-4.3 kernels. But only in headers. So we need a runtime check.
    if (checkKernelVersion(4, 3)) {
        addSecureBits(
                SECBIT_NO_CAP_AMBIENT_RAISE |
                SECBIT_NO_CAP_AMBIENT_RAISE_LOCKED);
    }
    else {
        logger::warn(
                "kernel is older than 4.3, not setting NO_CAP_AMBIENT_RAISE");
    }
#endif // ifdef SECBIT_NO_CAP_AMBIENT_RAISE
    withErrnoCheck(
            "prctl no new privs", prctl, PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
}

// This function must NEVER succeed if the bits in question are not set
void PrivListener::addSecureBits(unsigned long bits) {
    TRACE(bits);

    try {
        if ((secureBits_ & bits) != bits) {
            secureBits_ |= bits;
            withErrnoCheck(
                    "prctl set securebits",
                    prctl,
                    PR_SET_SECUREBITS,
                    secureBits_,
                    0,
                    0,
                    0);
        }
    }
    catch (const SystemException& ex) {
        if (ex.getErrno() != EPERM) {
            throw;
        }
        logger::debug("Set securebits failed with EPERM");
    }
}

} // namespace priv
} // namespace s2j

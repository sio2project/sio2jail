#pragma once

namespace s2j {

enum class Feature {
    PTRACE,
    PERF,
    SECCOMP,
    PID_NAMESPACE,
    NET_NAMESPACE,
    IPC_NAMESPACE,
    UTS_NAMESPACE,
    USER_NAMESPACE,
    MOUNT_NAMESPACE,
    MOUNT_PROCFS,
    CAPABILITY_DROP
};

}

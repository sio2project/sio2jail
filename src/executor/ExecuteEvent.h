#pragma once

#include <sys/types.h>

namespace s2j {
namespace executor {

struct ExecuteEvent {
    pid_t pid{-1};
    int exitStatus{0};
    int signal{0};
    bool exited{false};
    bool killed{false};
    bool stopped{false};
    bool trapped{false};
};

} // namespace executor
} // namespace s2j

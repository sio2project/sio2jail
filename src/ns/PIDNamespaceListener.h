#pragma once

#include "common/Feature.h"
#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"

namespace s2j {
namespace ns {

/**
 * Makes the child run in a separate PID namespace.
 * Requires CAP_SYS_ADMIN.
 */
class PIDNamespaceListener : public executor::ExecuteEventListener {
public:
    void onPreFork() override;

    const static Feature feature;
};

} // namespace ns
} // namespace s2j

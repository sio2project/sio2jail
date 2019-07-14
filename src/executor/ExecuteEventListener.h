#pragma once

#include "ExecuteAction.h"
#include "ExecuteEvent.h"

#include <unistd.h>

namespace s2j {
namespace executor {

class ExecuteEventListener {
public:
    virtual ~ExecuteEventListener() = default;

    virtual void onPreFork() {}
    virtual void onPostForkChild() {}
    virtual void onPostForkParent(pid_t childPid) {}
    virtual ExecuteAction onExecuteEvent(const ExecuteEvent& executeEvent) {
        return ExecuteAction::CONTINUE;
    }
    virtual ExecuteAction onSigioSignal() {
        return ExecuteAction::CONTINUE;
    }
    virtual ExecuteAction onSigalrmSignal() {
        return ExecuteAction::CONTINUE;
    }
    virtual void onPostExecute() {}
};

} // namespace executor
} // namespace s2j

#pragma once

#include "common/Feature.h"
#include "printer/OutputSource.h"
#include "executor/ExecuteEventListener.h"

namespace s2j {
namespace perf {

class PerfListener : public executor::ExecuteEventListener
                   , public printer::OutputSource {
public:
    PerfListener(uint64_t instructionCountLimit);
    ~PerfListener();

    void onPreFork() override;
    void onPostForkParent(pid_t childPid) override;
    void onPostForkChild() override;
    void onPostExecute() override;
    executor::ExecuteAction onSigioSignal() override;

    const static Feature feature;

private:
    uint64_t getInstructionsUsed();

    int perfFd_;
    uint64_t instructionCountLimit_;
    pid_t childPid_;

    // Barrier used for synchronization
    pthread_barrier_t* barrier_;
};

}
}

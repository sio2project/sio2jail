#pragma once

#include "common/Feature.h"
#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"

#include <cstdint>
#include <vector>

namespace s2j {
namespace perf {

class PerfListener
        : public executor::ExecuteEventListener
        , public printer::OutputSource {
public:
    PerfListener(uint64_t instructionCountLimit, uint64_t samplingFactor);
    ~PerfListener();

    void onPreFork() override;
    void onPostForkParent(pid_t childPid) override;
    void onPostForkChild() override;
    void onPostExecute() override;
    executor::ExecuteAction onSigioSignal() override;

    const static Feature feature;

private:
    uint64_t getInstructionsUsed();

    const uint64_t instructionCountLimit_;
    const uint64_t samplingFactor_;
    std::vector<int> perfFds_;
    pid_t childPid_{};

    // Barrier used for synchronization
    pthread_barrier_t* barrier_{};
};

} // namespace perf
} // namespace s2j

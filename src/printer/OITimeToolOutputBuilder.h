#pragma once

#include "OutputBuilder.h"

namespace s2j {
namespace printer {

class OITimeToolOutputBuilder : public OutputBuilder {
public:
    OITimeToolOutputBuilder();

    OutputBuilder& setCyclesUsed(uint64_t cyclesUsed) override;
    OutputBuilder& setMemoryPeak(uint64_t memoryPeakKb) override;
    OutputBuilder& setExitStatus(uint32_t exitStatus) override;
    OutputBuilder& setKillSignal(uint32_t killSignal) override;
    OutputBuilder& setKillReason(KillReason reason, const std::string& comment) override;

    std::string dump() const override;

    const static std::string FORMAT_NAME;

private:
    void dumpStatus(std::ostream& ss) const;
    int encodeStatusCode() const;

    static const uint64_t CYCLES_PER_SECOND = 2'000'000'000;

    uint64_t milliSecondsElapsed_;
    uint64_t memoryPeakKb_;
    uint64_t syscallsCounter_;
    uint32_t exitStatus_;
    uint32_t killSignal_;

    // TODO
    KillReason killReason_ = KillReason::NONE;
    std::string killReasonComment_;
};

}
}

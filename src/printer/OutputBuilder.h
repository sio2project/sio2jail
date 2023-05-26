#pragma once

#include <cstdint>
#include <string>

namespace s2j {
namespace printer {

class OutputBuilder {
public:
    enum class KillReason { NONE, RE, RV, TLE, MLE, OLE };

    static std::string killReasonName(KillReason reason) {
        switch (reason) {
        case KillReason::NONE:
            return "OK";
        case KillReason::RE:
            return "RE";
        case KillReason::RV:
            return "RV";
        case KillReason::TLE:
            return "TLE";
        case KillReason::MLE:
            return "MLE";
        case KillReason::OLE:
            return "OLE";
        }
        __builtin_unreachable();
    }

    virtual ~OutputBuilder() = default;

    virtual OutputBuilder& setCyclesUsed(uint64_t cyclesUsed) {
        return *this;
    }
    virtual OutputBuilder& setRealTimeMicroseconds(uint64_t time) {
        return *this;
    }
    virtual OutputBuilder& setUserTimeMicroseconds(uint64_t time) {
        return *this;
    }
    virtual OutputBuilder& setSysTimeMicroseconds(uint64_t time) {
        return *this;
    }
    virtual OutputBuilder& setMemoryPeak(uint64_t memoryPeakKb) {
        return *this;
    }
    virtual OutputBuilder& setExitStatus(uint32_t exitStatus) {
        return *this;
    }
    virtual OutputBuilder& setKillSignal(uint32_t killSignal) {
        return *this;
    }
    virtual OutputBuilder& setKillReason(
            KillReason reason,
            const std::string& comment) {
        return *this;
    }

    virtual std::string dump() const {
        return "";
    }
};

} // namespace printer
} // namespace s2j

#pragma once

#include <string>

namespace s2j {
namespace printer {

class OutputBuilder {
public:
    enum KillReason {
        OK, RE, RV, TLE, MLE, OLE
    };

    static std::string killReasonName(KillReason reason) {
        switch (reason) {
            case OK: return "OK";
            case RE: return "RE";
            case RV: return "RV";
            case TLE: return "TLE";
            case MLE: return "MLE";
            case OLE: return "OLE";
        }
        return "??";
    }

    virtual ~OutputBuilder() = default;

    virtual OutputBuilder& setCyclesUsed(uint64_t cyclesUsed) { return *this; }
    virtual OutputBuilder& setMemoryPeak(uint64_t memoryPeakKb) { return *this; }
    virtual OutputBuilder& setExitStatus(uint32_t exitStatus) { return *this; }
    virtual OutputBuilder& setKillSignal(uint32_t killSignal) { return *this; }
    virtual OutputBuilder& setKillReason(KillReason reason, const std::string& comment) { return *this; }

    virtual std::string dump() const { return ""; }
};

}
}

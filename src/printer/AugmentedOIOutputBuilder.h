#pragma once

#include "OIModelOutputBuilder.h"

namespace s2j {
namespace printer {

class AugmentedOIOutputBuilder : public OIModelOutputBuilder {
public:
    std::string dump() const override;

    const static std::string FORMAT_NAME;

private:
    void dumpStatus(std::ostream& ss) const;
    int encodeStatusCode() const;
};

}
}

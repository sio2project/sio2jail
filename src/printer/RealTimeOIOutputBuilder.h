#pragma once

#include "OIModelOutputBuilder.h"

namespace s2j {
namespace printer {

class RealTimeOIOutputBuilder : public OIModelOutputBuilder {
public:
    std::string dump() const override;

    const static std::string FORMAT_NAME;
};

} // namespace printer
} // namespace s2j

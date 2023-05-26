#pragma once

#include "OIModelOutputBuilder.h"

namespace s2j {
namespace printer {

class OITimeToolOutputBuilder : public OIModelOutputBuilder {
public:
    std::string dump() const override;

    const static std::string FORMAT_NAME;

private:
    int encodeStatusCode() const;
};

} // namespace printer
} // namespace s2j

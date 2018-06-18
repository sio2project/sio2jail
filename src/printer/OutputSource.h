#pragma once

#include "OutputBuilder.h"

#include <memory>

namespace s2j {
namespace printer {

class OutputSource {
public:
    void setOutputBuilder(std::shared_ptr<OutputBuilder> outputBuilder);

protected:
    std::shared_ptr<OutputBuilder> outputBuilder_;
};

}
}

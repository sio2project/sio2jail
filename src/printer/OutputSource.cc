#include "OutputSource.h"

#include <utility>

namespace s2j {
namespace printer {

void OutputSource::setOutputBuilder(
        std::shared_ptr<OutputBuilder> outputBuilder) {
    outputBuilder_ = std::move(outputBuilder);
}

} // namespace printer
} // namespace s2j

#include "OutputSource.h"

namespace s2j {
namespace printer {

void OutputSource::setOutputBuilder(
        std::shared_ptr<OutputBuilder> outputBuilder) {
    outputBuilder_ = outputBuilder;
}

} // namespace printer
} // namespace s2j

#include "LibSeccompFilter.h"

#include <algorithm>
#include <iostream>

namespace s2j {
namespace seccomp {
namespace filter {

bool LibSeccompFilter::isPureLibSeccompFilter() const {
    return true;
}

bool LibSeccompFilter::match(
        const tracer::TraceEvent& event,
        tracer::Tracee& tracee) const {
    return std::all_of(
            filterConditions_.begin(),
            filterConditions_.end(),
            [&](auto& filter) { return filter(event, tracee); });
}

const std::vector<struct scmp_arg_cmp>&
LibSeccompFilter::createLibSeccompFilter() const {
    return libSeccompFilterConditions_;
}

LibSeccompFilter LibSeccompFilter::operator&&(
        const LibSeccompFilter& filter) const {
    LibSeccompFilter newFilter;

    newFilter.filterConditions_ = filterConditions_;
    std::copy(
            filter.filterConditions_.begin(),
            filter.filterConditions_.end(),
            std::back_inserter(newFilter.filterConditions_));

    newFilter.libSeccompFilterConditions_ = libSeccompFilterConditions_;
    std::copy(
            filter.libSeccompFilterConditions_.begin(),
            filter.libSeccompFilterConditions_.end(),
            std::back_inserter(newFilter.libSeccompFilterConditions_));

    return newFilter;
}

SyscallArg::SyscallArg(uint8_t argumentIndex) : argumentIndex_(argumentIndex) {}

LibSeccompFilter SyscallArg::operator==(const uint64_t data) const {
    return LibSeccompFilter(
            [index = this->argumentIndex_, data](
                    const tracer::TraceEvent& /* event */,
                    tracer::Tracee& tracee) -> bool {
                return tracee.getSyscallArgument(index) == data;
            },
            SCMP_CMP(argumentIndex_, SCMP_CMP_EQ, data));
}

LibSeccompFilter SyscallArg::operator!=(const uint64_t data) const {
    return LibSeccompFilter(
            [index = this->argumentIndex_, data](
                    const tracer::TraceEvent& /* event */,
                    tracer::Tracee& tracee) -> bool {
                return tracee.getSyscallArgument(index) != data;
            },
            SCMP_CMP(argumentIndex_, SCMP_CMP_NE, data));
}

LibSeccompFilter SyscallArg::operator<=(const uint64_t data) const {
    return LibSeccompFilter(
            [index = this->argumentIndex_, data](
                    const tracer::TraceEvent& /* event */,
                    tracer::Tracee& tracee) -> bool {
                return tracee.getSyscallArgument(index) <= data;
            },
            SCMP_CMP(argumentIndex_, SCMP_CMP_LE, data));
}

LibSeccompFilter SyscallArg::operator>=(const uint64_t data) const {
    return LibSeccompFilter(
            [index = this->argumentIndex_, data](
                    const tracer::TraceEvent& /* event */,
                    tracer::Tracee& tracee) -> bool {
                return tracee.getSyscallArgument(index) >= data;
            },
            SCMP_CMP(argumentIndex_, SCMP_CMP_GE, data));
}

LibSeccompFilter SyscallArg::operator<(const uint64_t data) const {
    return LibSeccompFilter(
            [index = this->argumentIndex_, data](
                    const tracer::TraceEvent& /* event */, tracer::Tracee& tracee)
                    -> bool { return tracee.getSyscallArgument(index) < data; },
            SCMP_CMP(argumentIndex_, SCMP_CMP_LT, data));
}

LibSeccompFilter SyscallArg::operator>(const uint64_t data) const {
    return LibSeccompFilter(
            [index = this->argumentIndex_, data](
                    const tracer::TraceEvent& /* event */, tracer::Tracee& tracee)
                    -> bool { return tracee.getSyscallArgument(index) > data; },
            SCMP_CMP(argumentIndex_, SCMP_CMP_GT, data));
}

MaskedSyscallArg::MaskedSyscallArg(uint8_t argumentIndex, uint64_t mask)
        : argumentIndex_(argumentIndex), mask_(mask) {}

MaskedSyscallArg SyscallArg::operator&(const uint64_t mask) const {
    return MaskedSyscallArg{argumentIndex_, mask};
}

LibSeccompFilter MaskedSyscallArg::operator==(const uint64_t data) const {
    return LibSeccompFilter(
            [index = this->argumentIndex_, mask = this->mask_, data](
                    const tracer::TraceEvent& /* event */,
                    tracer::Tracee& tracee) -> bool {
                return (tracee.getSyscallArgument(index) & mask) == data;
            },
            SCMP_CMP(
                    argumentIndex_,
                    SCMP_CMP_MASKED_EQ,
                    static_cast<uint64_t>(mask_),
                    static_cast<uint64_t>(data)));
}

} // namespace filter
} // namespace seccomp
} // namespace s2j

#pragma once

#include "SyscallFilter.h"

#include <seccomp.h>

#include <functional>

namespace s2j {
namespace seccomp {
namespace filter {

class SyscallArg;
class MaskedSyscallArg;

/**
 * Class that represents libseccomp syscall filter.
 */
class LibSeccompFilter : public SyscallFilter {
public:
    LibSeccompFilter() = default;

    bool isPureLibSeccompFilter() const override;

    bool match(const tracer::TraceEvent& event, tracer::Tracee& tracee)
            const override;

    /* Filters can be joined tohegher. */
    LibSeccompFilter operator&&(const LibSeccompFilter& filter) const;

protected:
    const std::vector<struct scmp_arg_cmp>& createLibSeccompFilter()
            const override;

private:
    friend class SyscallArg;
    friend class MaskedSyscallArg;

    template<typename SeccompFilterFunction>
    LibSeccompFilter(
            SeccompFilterFunction filterCondition,
            struct scmp_arg_cmp libSeccompFilterCondition)
            : filterConditions_({std::function<
                      bool(const tracer::TraceEvent&, tracer::Tracee&)>(
                      filterCondition)})
            , libSeccompFilterConditions_({libSeccompFilterCondition}) {}

    std::vector<std::function<bool(const tracer::TraceEvent&, tracer::Tracee&)>>
            filterConditions_;
    std::vector<struct scmp_arg_cmp> libSeccompFilterConditions_;
};

/**
 * Helpers used for LibSeccmpFilter creation.
 */
class SyscallArg {
public:
    SyscallArg(uint8_t argumentIndex);

    /**
     * Syscall arguments can be compared to produce filter.
     */
    LibSeccompFilter operator==(const uint64_t data) const;
    LibSeccompFilter operator!=(const uint64_t data) const;
    LibSeccompFilter operator<=(const uint64_t data) const;
    LibSeccompFilter operator>=(const uint64_t data) const;
    LibSeccompFilter operator<(const uint64_t data) const;
    LibSeccompFilter operator>(const uint64_t data) const;

    MaskedSyscallArg operator&(const uint64_t mask) const;

private:
    uint8_t argumentIndex_;
};

class MaskedSyscallArg {
public:
    MaskedSyscallArg(uint8_t argumentIndex, uint64_t mask);

    /**
     * Masked syscall argument can be compared as well.
     */
    LibSeccompFilter operator==(const uint64_t data) const;

private:
    uint8_t argumentIndex_;
    uint64_t mask_;
};

} // namespace filter
} // namespace seccomp
} // namespace s2j

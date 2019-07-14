#pragma once

#include "tracer/TraceEvent.h"
#include "tracer/Tracee.h"

#include <seccomp.h>

#include <vector>

namespace s2j {
namespace seccomp {

class SeccompContext;

namespace filter {

class SyscallFilter {
public:
    virtual ~SyscallFilter() = default;

    /**
     * Wheather filter can be expressed with plain seccomp filters.
     * */
    virtual bool isPureLibSeccompFilter() const = 0;

    /**
     * Checks wheather conditions in this filter match given event.
     */
    virtual bool match(const tracer::TraceEvent& event, tracer::Tracee& tracee)
            const = 0;

protected:
    friend class s2j::seccomp::SeccompContext;

    virtual const std::vector<struct scmp_arg_cmp>& createLibSeccompFilter()
            const = 0;
};

} // namespace filter
} // namespace seccomp
} // namespace s2j

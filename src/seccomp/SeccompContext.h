#pragma once

#include "SeccompRule.h"

#include "tracer/Tracee.h"

#include <seccomp.h>

#include <cstdint>
#include <list>
#include <map>

namespace s2j {
namespace seccomp {

class SeccompContext {
public:
    class Builder {
    public:
        Builder();
        ~Builder();

        /**
         * Adds new rule to filter, it won't be active until @loadFilter is
         * called.
         */
        void addRule(const SeccompRule& rule, uint32_t actionGroupId);

        /**
         * Consumes builder and creates seccomp context.
         */
        SeccompContext build() &&;

    private:
        friend class SeccompContext;

        /**
         * Mantain two separate contexts, one for x86 and one for i386
         * architecture. In laodFilter merge them. This allows to distuinguish
         * syscalls architectures
         */
        std::map<tracer::Arch, scmp_filter_ctx> ctx_;
    };

    SeccompContext(Builder&& builder);
    ~SeccompContext();

    /**
     * Loads filter into kernel, from this point it will be active.
     */
    void loadFilter();

    /**
     * Exports filter in human-readable pseudocode.
     */
    std::string exportFilter() const;

    /**
     * List of all supported architectures
     */
    static const std::map<tracer::Arch, uint32_t> SECCOMP_FILTER_ARCHITECTURES;

    /**
     * Shift for trace values, smallest power of two not lesser than
     * size of SECCOMP_FILTER_ARCHITECTURES.
     */
    static const uint32_t SECCOMP_TRACE_MSG_NUM_SHIFT;


private:
    /**
     * Libseccomp's context of created filter.
     */
    scmp_filter_ctx ctx_;
};

} // namespace seccomp
} // namespace s2j

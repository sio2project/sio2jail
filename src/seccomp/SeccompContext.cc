#include "SeccompContext.h"
#include "SeccompException.h"

#include "common/FD.h"

#include <algorithm>

namespace s2j {
namespace seccomp {

const std::map<tracer::Arch, uint32_t>
        SeccompContext::SECCOMP_FILTER_ARCHITECTURES(
                {{tracer::Arch::X86, SCMP_ARCH_X86},
                 {tracer::Arch::X86_64, SCMP_ARCH_X86_64}});

const uint32_t SeccompContext::SECCOMP_TRACE_MSG_NUM_SHIFT = 3;

SeccompContext::Builder::Builder() {
    for (const auto arch: SECCOMP_FILTER_ARCHITECTURES) {
        scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_TRACE(0));
        if (ctx == nullptr) {
            throw Exception("Can't create a seccomp context");
        }
        if (seccomp_arch_remove(ctx, SCMP_ARCH_NATIVE) < 0) {
            throw Exception("Can't remove native architecture");
        }
        if (seccomp_arch_add(ctx, arch.second) < 0) {
            throw Exception("Can't add architecture to seccomp context");
        }
        ctx_.emplace(arch.first, ctx);
    }
}

SeccompContext::Builder::~Builder() {
    std::for_each(ctx_.begin(), ctx_.end(), [](auto ctx) {
        seccomp_release(ctx.second);
    });
    ctx_.clear();
}

SeccompContext SeccompContext::Builder::build() && {
    TRACE();

    return SeccompContext(std::move(*this));
}

void SeccompContext::Builder::addRule(
        const SeccompRule& rule,
        uint32_t actionGroupId) {
    TRACE(rule.syscall, actionGroupId);

    auto filter = rule.filter->createLibSeccompFilter();
    for (auto& arch: SECCOMP_FILTER_ARCHITECTURES) {
        int32_t ruleId = (actionGroupId << SECCOMP_TRACE_MSG_NUM_SHIFT) |
                         static_cast<uint8_t>(arch.first);

        rule.action->setRuleId(ruleId);
        auto action = rule.action->createLibSeccompAction();
        if (!rule.filter->isPureLibSeccompFilter()) {
            /**
             * When filter is too complex to be expressed by libseccomp filters,
             * always trace when rule is matched. Later filter will be checked
             * by tracer.
             */
            action = SCMP_ACT_TRACE(ruleId);
        }

        int res;

        /**
         * XXX
         * Libseccomp seems to optimize cases when there is an action without
         * any filter conditions. In these cases other rules are discarded. As
         * we rely on order and masking of rules always include at least one
         * filter condition.
         */
        if (!filter.empty()) {
            res = seccomp_rule_add_array(
                    ctx_[arch.first],
                    action,
                    rule.syscall,
                    filter.size(),
                    &filter[0]);
        }
        else {
            res = seccomp_rule_add(
                    ctx_[arch.first],
                    action,
                    rule.syscall,
                    1,
                    SCMP_CMP(0, SCMP_CMP_GE, 0));
            if (res > 0) {
                res = seccomp_rule_add(
                        ctx_[arch.first],
                        action,
                        rule.syscall,
                        1,
                        SCMP_CMP(0, SCMP_CMP_LT, 0));
            }
        }
        if (res < 0) {
            throw SystemException("Can't add rule to seccomp filter", -res);
        }
    }
}

SeccompContext::SeccompContext(Builder&& builder) {
    if (builder.ctx_.empty()) {
        ctx_ = seccomp_init(SCMP_ACT_KILL);
        if (ctx_ == nullptr) {
            throw Exception("Can't create a seccomp context");
        }
    }
    else {
        ctx_ = builder.ctx_.begin()->second;
        builder.ctx_.erase(builder.ctx_.begin());

        for (auto it = builder.ctx_.begin(); it != builder.ctx_.end();
             it = builder.ctx_.erase(it)) {
            if (seccomp_merge(ctx_, it->second) < 0) {
                throw Exception("Can't merge libseccomp filters");
            }
        }
    }
}

SeccompContext::~SeccompContext() {
    if (ctx_ != nullptr) {
        seccomp_release(ctx_);
        ctx_ = nullptr;
    }
}

void SeccompContext::loadFilter() {
    TRACE();

    if (seccomp_load(ctx_) < 0) {
        throw SeccompException("Filter load failed");
    }
}

std::string SeccompContext::exportFilter() const {
    FD fd(withErrnoCheck("memfd_create", syscall, __NR_memfd_create, "", 0));
    withErrnoCheck("truncate memfd_created file", ftruncate, fd, 1024 * 1024);

    // Export filter...
    if (seccomp_export_pfc(ctx_, fd) < 0) {
        throw Exception("Can't export libseccomp filter");
    }

    // ... and read it.
    withErrnoCheck("lseek on memfd file", lseek, fd, 0, SEEK_SET);
    std::string pseudoCode;
    while (true) {
        char buff[32 * 4096];
        ssize_t bytesRead =
                withErrnoCheck("read", read, fd, buff, sizeof(buff) - 1);
        if (bytesRead <= 0) {
            break;
        }

        // Filter out NULL bytes
        for (ssize_t index = 0; index < bytesRead; ++index) {
            if (buff[index] != '\0') {
                pseudoCode += buff[index];
            }
        }
    }

    return pseudoCode;
}

} // namespace seccomp
} // namespace s2j

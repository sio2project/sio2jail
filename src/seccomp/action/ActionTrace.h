#pragma once

#include "SeccompAction.h"

#include <functional>
#include <memory>

namespace s2j {
namespace seccomp {
namespace action {

class ActionTrace : public SeccompAction {
public:
    template<typename Handler>
    ActionTrace(Handler&& handler) : handler_(handler) {}

    ActionTrace()
            : ActionTrace([](auto& tracee) {
                return tracer::TraceAction::CONTINUE;
            }) {}

    Type getType() const override;

    virtual tracer::TraceAction execute(tracer::Tracee& tracee) override;

protected:
    virtual uint32_t createLibSeccompAction() const override;

private:
    std::function<tracer::TraceAction(tracer::Tracee&)> handler_;
};

} // namespace action
} // namespace seccomp
} // namespace s2j

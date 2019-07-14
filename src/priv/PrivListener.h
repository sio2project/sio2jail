#pragma once

#include "common/Feature.h"
#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"

namespace s2j {
namespace priv {

class PrivListener : public executor::ExecuteEventListener {
public:
    PrivListener();

    void onPostForkParent(pid_t childPid) override;
    void onPostForkChild() override;

    const static Feature feature;

private:
    void dropBSet();
    void dropCaps();
    void noNewPrivs();
    void addSecureBits(unsigned long bits);

    unsigned long secureBits_;
};

} // namespace priv
} // namespace s2j

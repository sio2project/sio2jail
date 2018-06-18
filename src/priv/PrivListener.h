#pragma once

#include "common/Feature.h"
#include "printer/OutputSource.h"
#include "executor/ExecuteEventListener.h"

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

}
}

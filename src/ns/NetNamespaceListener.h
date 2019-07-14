#pragma once

#include "common/Feature.h"
#include "executor/ExecuteEventListener.h"
#include "printer/OutputSource.h"

namespace s2j {
namespace ns {

class NetNamespaceListener : public executor::ExecuteEventListener {
public:
    void onPostForkChild() override;

    const static Feature feature;
};

} // namespace ns
} // namespace s2j

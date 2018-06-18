#pragma once

#include "common/Feature.h"
#include "printer/OutputSource.h"
#include "executor/ExecuteEventListener.h"

namespace s2j {
namespace ns {

class NetNamespaceListener : public executor::ExecuteEventListener {
public:
    void onPostForkChild() override;

    const static Feature feature;
};

}
}

#pragma once

namespace s2j {
namespace executor {

struct ExecuteEvent {
public:
    ExecuteEvent();

    int exitStatus;
    int signal;
    bool exited;
    bool killed;
    bool stopped;
    bool trapped;
};

}
}

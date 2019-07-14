#include "ExecuteEvent.h"

namespace s2j {
namespace executor {

ExecuteEvent::ExecuteEvent()
        : exitStatus(0)
        , signal(0)
        , exited(false)
        , killed(false)
        , stopped(false)
        , trapped(false) {}

} // namespace executor
} // namespace s2j

#include "LoggerListener.h"

#include "Logger.h"

namespace s2j {
namespace logger {

void LoggerListener::onPreFork() {
    logger::debug("Execution stage onPreFork");
}

void LoggerListener::onPostForkChild() {
    logger::debug("Execution stage onPostForkChild");
}

void LoggerListener::onPostForkParent(pid_t childPid) {
    logger::debug("Execution stage onPostForkParent, ", VAR(childPid));
}

executor::ExecuteAction LoggerListener::onExecuteEvent(
        const executor::ExecuteEvent& executeEvent) {
    logger::debug(
            "Execution stage onExecuteEvent, ",
            "exitStatus=",
            executeEvent.exitStatus,
            ", "
            "signal=",
            executeEvent.signal,
            ", "
            "exited=",
            executeEvent.exited,
            ", "
            "killed=",
            executeEvent.killed,
            ", "
            "stopped=",
            executeEvent.stopped,
            ", "
            "trapped=",
            executeEvent.trapped);
    return executor::ExecuteAction::CONTINUE;
}

void LoggerListener::onPostExec(const tracer::TraceEvent& /* traceEvent */) {
    logger::debug("Execution stage onPostExec");
}

void LoggerListener::onPostExecute() {
    logger::debug("Execution stage onPostExecute");
}

tracer::TraceAction LoggerListener::onTraceEvent(
        const tracer::TraceEvent& traceEvent,
        tracer::Tracee& tracee) {
    logger::debug("Execution stage onTraceEvent, isAlive=", tracee.isAlive());
    return tracer::TraceAction::CONTINUE;
}

} // namespace logger
} // namespace s2j

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
            "pid=",
            executeEvent.pid,
            ", "
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

void LoggerListener::onPostExecute() {
    logger::debug("Execution stage onPostExecute");
}

tracer::TraceAction LoggerListener::onPostExec(
        const tracer::TraceEvent& /* traceEvent */,
        tracer::Tracee& /* tracee */) {
    logger::debug("Execution stage onPostExec");
    return tracer::TraceAction::CONTINUE;
}

std::tuple<tracer::TraceAction, tracer::TraceAction>
LoggerListener::onPostClone(
        const tracer::TraceEvent& /* traceEvent */,
        tracer::Tracee& tracee,
        pid_t traceeChildPid) {
    logger::debug(
            "Execution stage onPostClone, traceePid=",
            tracee.getPid(),
            ", traceeChildPid=",
            traceeChildPid);
    return {tracer::TraceAction::CONTINUE, tracer::TraceAction::CONTINUE};
}

tracer::TraceAction LoggerListener::onTraceEvent(
        const tracer::TraceEvent& /*traceEvent*/,
        tracer::Tracee& tracee) {
    logger::debug("Execution stage onTraceEvent, isAlive=", tracee.isAlive());
    return tracer::TraceAction::CONTINUE;
}

} // namespace logger
} // namespace s2j

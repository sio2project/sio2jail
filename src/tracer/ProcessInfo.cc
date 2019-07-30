#include "ProcessInfo.h"

#include "common/Assert.h"

namespace s2j {
namespace tracer {

ProcessInfo::ProcessInfo(pid_t pid, std::shared_ptr<ProcessInfo> parent)
        : pid_{pid}, parent_{parent} {
    if (parent == nullptr)
        wholeTreeInfo_ =
                std::make_shared<std::map<pid_t, std::weak_ptr<ProcessInfo>>>();
    else
        wholeTreeInfo_ = parent->wholeTreeInfo_;
}

std::shared_ptr<ProcessInfo> ProcessInfo::makeProcessInfo(
        pid_t pid,
        std::shared_ptr<ProcessInfo> parent) {
    auto process = std::make_shared<ProcessInfo>(pid, parent);

    auto selfInsert = process->wholeTreeInfo_->emplace(pid, process);
    assert(selfInsert.second, "pid was already present in process tree info");

    return process;
}

ProcessInfo::~ProcessInfo() {
    wholeTreeInfo_->erase(pid_);
}

std::shared_ptr<ProcessInfo> ProcessInfo::addChild(pid_t pid) {
    auto childInsert = children_.emplace(
            pid, ProcessInfo::makeProcessInfo(pid, shared_from_this()));
    assert(childInsert.second, "pid was already present in children_");
    return childInsert.first->second;
}

void ProcessInfo::delChild(pid_t pid) {
    auto child = children_.find(pid);
    assert(child != children_.end());
    children_.erase(child);
}

} // namespace tracer
} // namespace s2j

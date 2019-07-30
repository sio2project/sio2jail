#pragma once

#include <sys/types.h>

#include <map>
#include <memory>

namespace s2j {
namespace tracer {

class ProcessInfo : public std::enable_shared_from_this<ProcessInfo> {
public:
    // Should not be called directly, use makeProcessInfo instead
    ProcessInfo(pid_t pid, std::shared_ptr<ProcessInfo> parent);
    static std::shared_ptr<ProcessInfo> makeProcessInfo(
            pid_t pid,
            std::shared_ptr<ProcessInfo> parent);

    ~ProcessInfo();

    std::shared_ptr<ProcessInfo> addChild(pid_t pid);
    void delChild(pid_t pid);

    pid_t getPid() const {
        return pid_;
    }

    std::shared_ptr<ProcessInfo> getParent() const {
        return parent_.lock();
    }

    std::shared_ptr<ProcessInfo> getChild(pid_t pid) const {
        auto child = children_.find(pid);
        if (child == children_.end()) {
            return nullptr;
        }
        return child->second;
    }

    std::shared_ptr<ProcessInfo> getProcess(pid_t pid) const {
        auto process = wholeTreeInfo_->find(pid);
        if (process == wholeTreeInfo_->end()) {
            return nullptr;
        }
        auto processInfo = process->second.lock();
        if (processInfo == nullptr) {
            wholeTreeInfo_->erase(process);
            return nullptr;
        }
        return processInfo;
    }

private:
    const pid_t pid_;
    const std::weak_ptr<ProcessInfo> parent_;
    std::shared_ptr<std::map<pid_t, std::weak_ptr<ProcessInfo>>> wholeTreeInfo_;
    std::map<pid_t, std::shared_ptr<ProcessInfo>> children_;
};

} // namespace tracer
} // namespace s2j

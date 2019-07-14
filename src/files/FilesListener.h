#pragma once

#include "common/WithErrnoCheck.h"
#include "executor/ExecuteEventListener.h"

#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <vector>

namespace s2j {
namespace files {

class FilesListener : public executor::ExecuteEventListener {
public:
    FilesListener(bool suppressStderr = true);

    void onPostForkChild() override;
    void onPreFork() override;

    const static std::string DEV_NULL;
    const static std::string FDS_PATH;

private:
    const bool suppressStderr_;
    std::vector<int> fds_;
    int devnull_;
};

} // namespace files
} // namespace s2j

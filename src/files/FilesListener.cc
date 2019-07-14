#include "FilesListener.h"

#include <dirent.h>
#include <sys/types.h>

namespace s2j {
namespace files {

const std::string FilesListener::DEV_NULL = "/dev/null";
const std::string FilesListener::FDS_PATH = "/proc/self/fd/";

FilesListener::FilesListener(bool suppressStderr)
        : suppressStderr_(suppressStderr) {}

void FilesListener::onPreFork() {
    TRACE();

    // Gather all open fds we will want to close
    std::unique_ptr<DIR, int (*)(DIR*)> fdsDirectory(
            withErrnoCheck("open fds directory", opendir, FDS_PATH.c_str()),
            closedir);

    if (fdsDirectory == nullptr)
        throw Exception("Can't open fds directory " + FDS_PATH);

    for (struct dirent* entry = readdir(fdsDirectory.get()); entry != nullptr;
         entry = readdir(fdsDirectory.get())) {
        int fd;
        try {
            fd = std::stoi(entry->d_name);
        }
        catch (const std::exception&) {
            continue;
        }

        // Don't close stdin, stdout and logs output fd
        if ((fd >= 0 && fd <= 2) || s2j::logger::isLoggerFD(fd))
            continue;
        fds_.push_back(fd);
    }

    // Open /dev/null for child
    devnull_ =
            withErrnoCheck("open /dev/null", open, DEV_NULL.c_str(), O_WRONLY);
}

void FilesListener::onPostForkChild() {
    TRACE();

    // Redirect stderr to /dev/null
    if (suppressStderr_)
        withErrnoCheck("redirect stderr to /dev/null", dup2, devnull_, 2);
    withErrnoCheck("close /dev/null", close, devnull_);

    for (size_t fdsIndex = 0; fdsIndex < fds_.size();) {
        try {
            withErrnoCheck("close fd", close, fds_[fdsIndex]);
        }
        catch (const SystemException& ex) {
            if (ex.getErrno() == EINTR)
                continue;
            else if (ex.getErrno() != EBADF)
                throw;
        }
        ++fdsIndex;
    }
}

} // namespace files
} // namespace s2j

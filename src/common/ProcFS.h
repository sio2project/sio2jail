#pragma once

#include <sys/types.h>

#include <cinttypes>


namespace s2j {
namespace procfs {

/**
 * Supported /proc/$PID/status fields
 */
enum class Field { VM_PEAK, VM_SIZE, SIG_CGT };

/**
 * Read a field from /proc/$PID/status file
 */
uint64_t readProcFS(pid_t pid, Field field, uint64_t defaultValue = 0);

} // namespace procfs
} // namespace s2j

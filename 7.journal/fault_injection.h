#ifndef FAULT_INJECTION_H_
#define FAULT_INJECTION_H_

#include <sys/types.h>
#include <unistd.h>

// Fault injection handlers - tests can set these to inject failures
// Return true to use the injected behavior, false to call real syscall

// read() handler
// Can modify: buf (data), return value, errno
extern bool (*fault_inject_read)(int fd, void* buf, size_t count,
                                  ssize_t* ret, int* err);

// write() handler
// Can modify: buf (data being written), return value, errno
extern bool (*fault_inject_write)(int fd, const void* buf, size_t count,
                                   ssize_t* ret, int* err);

// pread() handler
// Can modify: buf, offset, return value, errno
extern bool (*fault_inject_pread)(int fd, void* buf, size_t count,
                                   off_t* offset, ssize_t* ret, int* err);

// pwrite() handler
// Can modify: buf, offset, return value, errno
extern bool (*fault_inject_pwrite)(int fd, const void* buf, size_t count,
                                    off_t* offset, ssize_t* ret, int* err);

// fsync() handler
// Can modify: return value, errno
extern bool (*fault_inject_fsync)(int fd, int* ret, int* err);

// Helper to reset all handlers to nullptr
void ResetFaultInjection();

#endif  // FAULT_INJECTION_H_

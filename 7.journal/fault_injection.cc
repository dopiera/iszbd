#include "fault_injection.h"

#include <dlfcn.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

// Function pointers for fault injection handlers
bool (*fault_inject_read)(int fd, void* buf, size_t count, ssize_t* ret,
                          int* err) = nullptr;
bool (*fault_inject_write)(int fd, const void* buf, size_t count, ssize_t* ret,
                           int* err) = nullptr;
bool (*fault_inject_pread)(int fd, void* buf, size_t count, off_t* offset,
                           ssize_t* ret, int* err) = nullptr;
bool (*fault_inject_pwrite)(int fd, const void* buf, size_t count,
                            off_t* offset, ssize_t* ret, int* err) = nullptr;
bool (*fault_inject_fsync)(int fd, int* ret, int* err) = nullptr;

void ResetFaultInjection() {
  fault_inject_read = nullptr;
  fault_inject_write = nullptr;
  fault_inject_pread = nullptr;
  fault_inject_pwrite = nullptr;
  fault_inject_fsync = nullptr;
}

// Real syscall function pointers
using ReadFunc = ssize_t (*)(int, void*, size_t);
using WriteFunc = ssize_t (*)(int, const void*, size_t);
using PreadFunc = ssize_t (*)(int, void*, size_t, off_t);
using PwriteFunc = ssize_t (*)(int, const void*, size_t, off_t);
using FsyncFunc = int (*)(int);

static ReadFunc real_read = nullptr;
static WriteFunc real_write = nullptr;
static PreadFunc real_pread = nullptr;
static PwriteFunc real_pwrite = nullptr;
static FsyncFunc real_fsync = nullptr;

// Initialize real function pointers
static void InitRealFunctions() {
  if (!real_read) {
    real_read = reinterpret_cast<ReadFunc>(dlsym(RTLD_NEXT, "read"));
  }
  if (!real_write) {
    real_write = reinterpret_cast<WriteFunc>(dlsym(RTLD_NEXT, "write"));
  }
  if (!real_pread) {
    real_pread = reinterpret_cast<PreadFunc>(dlsym(RTLD_NEXT, "pread"));
  }
  if (!real_pwrite) {
    real_pwrite = reinterpret_cast<PwriteFunc>(dlsym(RTLD_NEXT, "pwrite"));
  }
  if (!real_fsync) {
    real_fsync = reinterpret_cast<FsyncFunc>(dlsym(RTLD_NEXT, "fsync"));
  }
}

// Intercepted syscalls
extern "C" {

ssize_t read(int fd, void* buf, size_t count) {
  InitRealFunctions();

  if (fault_inject_read) {
    ssize_t ret;
    int err;
    if (fault_inject_read(fd, buf, count, &ret, &err)) {
      errno = err;
      return ret;
    }
  }

  return real_read(fd, buf, count);
}

ssize_t write(int fd, const void* buf, size_t count) {
  InitRealFunctions();

  if (fault_inject_write) {
    ssize_t ret;
    int err;
    if (fault_inject_write(fd, buf, count, &ret, &err)) {
      errno = err;
      return ret;
    }
  }

  return real_write(fd, buf, count);
}

ssize_t pread(int fd, void* buf, size_t count, off_t offset) {
  InitRealFunctions();

  if (fault_inject_pread) {
    ssize_t ret;
    int err;
    off_t modified_offset = offset;
    if (fault_inject_pread(fd, buf, count, &modified_offset, &ret, &err)) {
      errno = err;
      return ret;
    }
    // If handler modified offset but didn't override, use modified offset
    if (modified_offset != offset) {
      offset = modified_offset;
    }
  }

  return real_pread(fd, buf, count, offset);
}

ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset) {
  InitRealFunctions();

  if (fault_inject_pwrite) {
    ssize_t ret;
    int err;
    off_t modified_offset = offset;
    if (fault_inject_pwrite(fd, buf, count, &modified_offset, &ret, &err)) {
      errno = err;
      return ret;
    }
    // If handler modified offset but didn't override, use modified offset
    if (modified_offset != offset) {
      offset = modified_offset;
    }
  }

  return real_pwrite(fd, buf, count, offset);
}

int fsync(int fd) {
  InitRealFunctions();

  if (fault_inject_fsync) {
    int ret;
    int err;
    if (fault_inject_fsync(fd, &ret, &err)) {
      errno = err;
      return ret;
    }
  }

  return real_fsync(fd);
}

}  // extern "C"

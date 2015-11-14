// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/gpu_memory_buffer.h"

namespace gfx {

base::trace_event::MemoryAllocatorDumpGuid GetGpuMemoryBufferGUIDForTracing(
    uint64 tracing_process_id,
    GpuMemoryBufferId buffer_id) {
  // TODO(ericrk): Currently this function just wraps
  // GetGenericSharedMemoryGUIDForTracing, we may want to special case this if
  // the GPU memory buffer is not backed by shared memory.
  return gfx::GetGenericSharedMemoryGUIDForTracing(tracing_process_id,
                                                   buffer_id);
}

GpuMemoryBufferHandle::GpuMemoryBufferHandle()
    : type(EMPTY_BUFFER), id(0), handle(base::SharedMemory::NULLHandle()) {
}

#ifdef DO_ZERO_COPY
GpuMemoryBufferHandle GpuMemoryBufferHandle::ShareToProcess() const {
  gfx::GpuMemoryBufferHandle handle;
  handle = *this;
  handle.fd1.fd = HANDLE_EINTR(dup(fd1.fd));
  handle.fd1.auto_close = true;
  handle.fd2.fd = HANDLE_EINTR(dup(fd2.fd));
  handle.fd2.auto_close = true;
  return handle;
}
#endif

}  // namespace gfx

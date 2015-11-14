// Copyright (c) 2015, The Linux Foundation. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/child/child_gpu_memory_buffer_manager.h"

#include "content/common/child_process_messages.h"
#include "content/common/generic_shared_memory_id_generator.h"
#include "content/common/gpu/client/gpu_memory_buffer_impl.h"
#include "content/common/gpu/gpu_memory_buffer_factory_texture_memory.h"

#include "ui/gfx/sweadreno_texture_memory.h"

namespace content {
namespace {

void DeletedGpuMemoryBuffer(ThreadSafeSender* sender,
                            gfx::GpuMemoryBufferId id,
                            uint32 sync_point) {
  TRACE_EVENT0("renderer",
               "ChildGpuMemoryBufferManager::DeletedGpuMemoryBuffer");
  sender->Send(new ChildProcessHostMsg_DeletedGpuMemoryBuffer(id, sync_point));
}

#ifdef DO_ZERO_COPY
void DeletedTextureMemoryGpuMemoryBuffer(ThreadSafeSender* sender,
                            gfx::GpuMemoryBufferId id,
                            uint32 sync_point) {
  TRACE_EVENT0("renderer",
               "ChildGpuMemoryBufferManager::DeletedTextureMemoryGpuMemoryBuffer");
  //do nothing as the buffer is not created by GPU process and there's not need to notify GPU process to delete it.
}

WebTech::PixelFormat FormatToPixelFormat(gfx::BufferFormat format) {
  switch (format) {
    case gfx::BufferFormat::RGBA_8888:
    case gfx::BufferFormat::BGRX_8888:
    case gfx::BufferFormat::BGRA_8888:
      return WebTech::FORMAT_8888;
    default:
      NOTREACHED();
      return WebTech::FORMAT_8888;
    }
}
#endif

}  // namespace

ChildGpuMemoryBufferManager::ChildGpuMemoryBufferManager(
    ThreadSafeSender* sender)
    : sender_(sender) {
}

ChildGpuMemoryBufferManager::~ChildGpuMemoryBufferManager() {
}

scoped_ptr<gfx::GpuMemoryBuffer>
ChildGpuMemoryBufferManager::AllocateGpuMemoryBuffer(const gfx::Size& size,
                                                     gfx::BufferFormat format,
                                                     gfx::BufferUsage usage) {
#ifdef DO_ZERO_COPY
  ZEROCOPY_LOG("ChildGpuMemoryBufferManager::AllocateGpuMemoryBuffer");
#endif
  TRACE_EVENT2("renderer",
               "ChildGpuMemoryBufferManager::AllocateGpuMemoryBuffer",
               "width",
               size.width(),
               "height",
               size.height());

  gfx::GpuMemoryBufferHandle handle;

#ifdef DO_ZERO_COPY
  if (swe::PrepareCreateTextureMemory() &&
      GpuMemoryBufferFactoryTextureMemory::IsGpuMemoryBufferConfigurationSupported(format,
        usage)) {
    handle.type = gfx::TEXTURE_MEMORY_BUFFER;
    handle.id.id = -1;

    WebTech::TextureMemory* texture = swe::CreateTextureMemory();

    if (texture) {
      texture->Init(size.width(), size.height(), FormatToPixelFormat(format));

      int fds[2];
      size_t serialize_size;
      serialize_size = texture->GetSerializedDataSize();
      handle.texture_memory_data.Allocate(serialize_size);
      texture->Serialize((uint8_t*)((handle.texture_memory_data.values_)), fds);
      handle.fd1.fd = (fds[0]);
      handle.fd2.fd = (fds[1]);

      scoped_ptr<GpuMemoryBufferImpl> buf(GpuMemoryBufferImpl::CreateFromHandle(
        handle,
        size,
        format,
        usage,
        base::Bind(&DeletedTextureMemoryGpuMemoryBuffer, sender_, handle.id)));

      ZEROCOPY_LOG("ChildGpuMemoryBufferManager::AllocateGpuMemoryBuffer creating TextureMemory buffer");
      return buf.Pass();
    }
  }
#endif

  IPC::Message* message = new ChildProcessHostMsg_SyncAllocateGpuMemoryBuffer(
      content::GetNextGenericSharedMemoryId(), size.width(), size.height(),
      format, usage, &handle);
  bool success = sender_->Send(message);
  if (!success || handle.is_null())
    return nullptr;

  scoped_ptr<GpuMemoryBufferImpl> buffer(GpuMemoryBufferImpl::CreateFromHandle(
      handle, size, format, usage,
      base::Bind(&DeletedGpuMemoryBuffer, sender_, handle.id)));
  if (!buffer) {
    sender_->Send(new ChildProcessHostMsg_DeletedGpuMemoryBuffer(handle.id, 0));
    return nullptr;
  }

  return buffer.Pass();
}

gfx::GpuMemoryBuffer*
ChildGpuMemoryBufferManager::GpuMemoryBufferFromClientBuffer(
    ClientBuffer buffer) {
  return GpuMemoryBufferImpl::FromClientBuffer(buffer);
}

void ChildGpuMemoryBufferManager::SetDestructionSyncPoint(
    gfx::GpuMemoryBuffer* buffer,
    uint32 sync_point) {
  static_cast<GpuMemoryBufferImpl*>(buffer)
      ->set_destruction_sync_point(sync_point);
}

}  // namespace content

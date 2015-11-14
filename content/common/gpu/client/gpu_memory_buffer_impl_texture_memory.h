// Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_GPU_CLIENT_GPU_MEMORY_BUFFER_IMPL_TEXTURE_MEMORY_H_
#define CONTENT_COMMON_GPU_CLIENT_GPU_MEMORY_BUFFER_IMPL_TEXTURE_MEMORY_H_

#include "ui/gfx/sweadreno_texture_memory.h"

#ifdef DO_ZERO_COPY

#include "content/common/gpu/client/gpu_memory_buffer_impl.h"
#include "content/common/TextureMemory.h"

namespace content {

// Provides implementation of a GPU memory buffer based
// on a texture memory handle.
class GpuMemoryBufferImplTextureMemory : public GpuMemoryBufferImpl {
 public:
  static scoped_ptr<GpuMemoryBufferImpl> CreateFromHandle(
      const gfx::GpuMemoryBufferHandle& handle,
      const gfx::Size& size,
      gfx::BufferFormat format,
      const DestructionCallback& callback);

  bool Initialize();
  bool Initialize(gfx::GpuMemoryBufferHandle handle);

  // Overridden from gfx::GpuMemoryBuffer:
  bool Map(void** data) override;
  void Unmap() override;
  void GetStride(int* stride) const override;
  gfx::GpuMemoryBufferHandle GetHandle() const override;

  WebTech::TextureMemory* GetTexture();

 private:
  GpuMemoryBufferImplTextureMemory(gfx::GpuMemoryBufferId id,
                                   const gfx::Size& size,
                                   gfx::BufferFormat format,
                                   const DestructionCallback& callback);
  virtual ~GpuMemoryBufferImplTextureMemory() override;

  WebTech::TextureMemory* texture_;
  void* addr_;
  size_t stride_;
  DISALLOW_COPY_AND_ASSIGN(GpuMemoryBufferImplTextureMemory);
};

}  // namespace content

#endif // DO_ZERO_COPY
#endif  // CONTENT_COMMON_GPU_CLIENT_GPU_MEMORY_BUFFER_IMPL_TEXTURE_MEMORY_H_

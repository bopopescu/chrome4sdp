// Copyright 2014-2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_GPU_GPU_MEMORY_BUFFER_FACTORY_TEXTURE_MEMROY_H_
#define CONTENT_COMMON_GPU_GPU_MEMORY_BUFFER_FACTORY_TEXTURE_MEMROY_H_

#include "base/containers/hash_tables.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "content/common/gpu/gpu_memory_buffer_factory.h"
#include "gpu/command_buffer/service/image_factory.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/gpu_memory_buffer.h"

#include "ui/gfx/sweadreno_texture_memory.h"

#ifdef DO_ZERO_COPY

namespace gfx {
class GLImage;
}

namespace content {

class GpuMemoryBufferFactoryTextureMemory : public GpuMemoryBufferFactory,
                                           public gpu::ImageFactory {
 public:
  GpuMemoryBufferFactoryTextureMemory();
  ~GpuMemoryBufferFactoryTextureMemory() override;

  static bool IsGpuMemoryBufferConfigurationSupported(gfx::BufferFormat format,
                                                      gfx::BufferUsage usage);

  // GpuMemoryBufferFactory functions
  void GetSupportedGpuMemoryBufferConfigurations(std::vector<Configuration>* configurations) override;
  gfx::GpuMemoryBufferHandle CreateGpuMemoryBuffer(gfx::GpuMemoryBufferId id,
                                                   const gfx::Size& size,
                                                   gfx::BufferFormat format,
                                                   gfx::BufferUsage usage,
                                                   int client_id,
                                                   gfx::PluginWindowHandle surface_handle) override;
  void DestroyGpuMemoryBuffer(gfx::GpuMemoryBufferId id,
                              int client_id) override;
  gpu::ImageFactory* AsImageFactory() override;

  // ImageFactory functions
  scoped_refptr<gfx::GLImage> CreateImageForGpuMemoryBuffer(const gfx::GpuMemoryBufferHandle& handle,
                                                            const gfx::Size& size,
                                                            gfx::BufferFormat format,
                                                            unsigned internalformat,
                                                            int client_id) override;

 private:
  typedef std::pair<int, int> TextureMemoryMapKey;
  typedef base::hash_map<TextureMemoryMapKey, WebTech::TextureMemory*> TextureMemoryMap;
  TextureMemoryMap texture_memory_map_;
  base::Lock texture_memory_map_lock_;
  DISALLOW_COPY_AND_ASSIGN(GpuMemoryBufferFactoryTextureMemory);
};

}  // namespace content

#endif //DO_ZERO_COPY

#endif  // CONTENT_COMMON_GPU_GPU_MEMORY_BUFFER_FACTORY_TEXTURE_MEMROY_H_

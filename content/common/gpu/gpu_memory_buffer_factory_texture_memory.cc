// Copyright 2014-2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/gpu/gpu_memory_buffer_factory_texture_memory.h"

#include "base/logging.h"
#include "content/common/gpu/client/gpu_memory_buffer_impl.h"
#include "ui/gl/gl_image.h"
#include "ui/gl/gl_image_egl.h"

#ifdef DO_ZERO_COPY
#include <unistd.h>

namespace content {

extern WebTech::PixelFormat FormatToPixelFormat(gfx::BufferFormat format);

const GpuMemoryBufferFactory::Configuration kSupportedConfigurations[] = {
  { gfx::BufferFormat::RGBA_8888, gfx::BufferUsage::PERSISTENT_MAP },
};

GpuMemoryBufferFactoryTextureMemory::GpuMemoryBufferFactoryTextureMemory() {
}

GpuMemoryBufferFactoryTextureMemory::~GpuMemoryBufferFactoryTextureMemory() {
}

// static
bool GpuMemoryBufferFactoryTextureMemory::
    IsGpuMemoryBufferConfigurationSupported(gfx::BufferFormat format,
                                            gfx::BufferUsage usage) {
  for (auto& configuration : kSupportedConfigurations) {
    if (configuration.format == format && configuration.usage == usage)
      return true;
  }

  return false;
}

void GpuMemoryBufferFactoryTextureMemory::
    GetSupportedGpuMemoryBufferConfigurations(
        std::vector<Configuration>* configurations) {
  configurations->assign(
      kSupportedConfigurations,
      kSupportedConfigurations + arraysize(kSupportedConfigurations));
}

gfx::GpuMemoryBufferHandle
GpuMemoryBufferFactoryTextureMemory::CreateGpuMemoryBuffer(
    gfx::GpuMemoryBufferId id,
    const gfx::Size& size,
    gfx::BufferFormat format,
    gfx::BufferUsage usage,
    int client_id,
    gfx::PluginWindowHandle surface_handle) {
  base::SharedMemory shared_memory;
  ZEROCOPY_LOG(" GpuMemoryBufferFactoryTextureMemory::CreateGpuMemoryBuffer and serializing");
  gfx::GpuMemoryBufferHandle handle;
  handle.type = gfx::TEXTURE_MEMORY_BUFFER;
  handle.id = id;

  swe::InitTextureMemory();
  WebTech::TextureMemory* texture = swe::CreateTextureMemory();
  if (!texture)
    return gfx::GpuMemoryBufferHandle();

  texture->Init(size.width(), size.height(), FormatToPixelFormat(format));

  {
    base::AutoLock lock(texture_memory_map_lock_);

    TextureMemoryMapKey key(id.id, client_id);
    DCHECK(texture_memory_map_.find(key) == texture_memory_map_.end());
    texture_memory_map_[key] = texture;
  }

  int fds[2];
  size_t serialize_size;
  serialize_size = texture->GetSerializedDataSize();
  handle.texture_memory_data.Allocate(serialize_size);
  texture->Serialize((uint8_t*)((handle.texture_memory_data.values_)), fds);
  handle.fd1.fd = (fds[0]);
  handle.fd2.fd = (fds[1]);

  return handle.ShareToProcess();
}

void GpuMemoryBufferFactoryTextureMemory::DestroyGpuMemoryBuffer(
    gfx::GpuMemoryBufferId id,
    int client_id) {
  ZEROCOPY_LOG(" GpuMemoryBufferFactoryTextureMemory::DestroyGpuMemoryBuffer ");
  base::AutoLock lock(texture_memory_map_lock_);

  TextureMemoryMapKey key(id.id, client_id);
  TextureMemoryMap::iterator it = texture_memory_map_.find(key);
  if (it != texture_memory_map_.end()) {
    WebTech::TextureMemory* texture = it->second;
    if (texture) {
      swe::DestroyTextureMemory(texture);
    }
    texture_memory_map_.erase(it);
  }
}

gpu::ImageFactory* GpuMemoryBufferFactoryTextureMemory::AsImageFactory() {
  return this;
}

scoped_refptr<gfx::GLImage>
GpuMemoryBufferFactoryTextureMemory::CreateImageForGpuMemoryBuffer(
    const gfx::GpuMemoryBufferHandle& handle,
    const gfx::Size& size,
    gfx::BufferFormat format,
    unsigned internalformat,
    int client_id) {
  ZEROCOPY_LOG(" GpuMemoryBufferFactoryTextureMemory::CreateImageForGpuMemoryBuffer ");
  WebTech::TextureMemory* texture = 0;
  if (handle.id.id != -1) {
    base::AutoLock lock(texture_memory_map_lock_);
    TextureMemoryMapKey key(handle.id.id, client_id);
    TextureMemoryMap::iterator it = texture_memory_map_.find(key);
    swe::InitTextureMemory();
    if (it != texture_memory_map_.end()) {
      texture = it->second;
      close(handle.fd1.fd);
      close(handle.fd2.fd);
    }
  }
  scoped_refptr<gfx::GLImageEGL> image(
      new gfx::GLImageEGL(size));
  if (!image->InitializeTextureMemory(handle, texture, internalformat))
    return scoped_refptr<gfx::GLImage>();

  return image;
}

}  // namespace content

#endif //DO_ZERO_COPY


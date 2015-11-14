// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GL_GL_IMAGE_EGL_H_
#define UI_GL_GL_IMAGE_EGL_H_

#include "base/threading/thread_checker.h"
#include "ui/gl/gl_bindings.h"
#include "ui/gl/gl_image.h"

#include "ui/gfx/gpu_memory_buffer.h"
#include "ui/gfx/sweadreno_texture_memory.h"

namespace gfx {

class GL_EXPORT GLImageEGL : public GLImage {
 public:
  explicit GLImageEGL(const gfx::Size& size);

  bool Initialize(EGLenum target, EGLClientBuffer buffer, const EGLint* attrs);
#ifdef DO_ZERO_COPY
  bool InitializeTextureMemory(gfx::GpuMemoryBufferHandle buffer, WebTech::TextureMemory* texture, unsigned internal_format);
#endif

  // Overridden from GLImage:
  void Destroy(bool have_context) override;
  gfx::Size GetSize() override;
  unsigned GetInternalFormat() override;
  bool BindTexImage(unsigned target) override;
  void ReleaseTexImage(unsigned target) override {}
  bool CopyTexSubImage(unsigned target,
                       const Point& offset,
                       const Rect& rect) override;
  void WillUseTexImage() override {}
  void DidUseTexImage() override {}
  void WillModifyTexImage() override {}
  void DidModifyTexImage() override {}
  bool ScheduleOverlayPlane(gfx::AcceleratedWidget widget,
                            int z_order,
                            OverlayTransform transform,
                            const Rect& bounds_rect,
                            const RectF& crop_rect) override;
  void OnMemoryDump(base::trace_event::ProcessMemoryDump* pmd,
                            uint64_t process_tracing_id,
                            const std::string& dump_name) override {};

 protected:
  ~GLImageEGL() override;

  EGLImageKHR egl_image_;
#ifdef DO_ZERO_COPY
  WebTech::TextureMemory* texture_;
  bool own_texture_;
#endif
  const gfx::Size size_;
  base::ThreadChecker thread_checker_;

 private:
  DISALLOW_COPY_AND_ASSIGN(GLImageEGL);
};

}  // namespace gfx

#endif  // UI_GL_GL_IMAGE_EGL_H_

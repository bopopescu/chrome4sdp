// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gl/gl_image_egl.h"

#include "ui/gl/egl_util.h"
#include "ui/gl/gl_surface_egl.h"

#ifdef DO_ZERO_COPY
#include "base/trace_event/trace_event.h"
#endif

namespace gfx {

GLImageEGL::GLImageEGL(const gfx::Size& size)
    : egl_image_(EGL_NO_IMAGE_KHR),
#ifdef DO_ZERO_COPY
      texture_(0),
      own_texture_(true),
#endif
    size_(size) {
}

#ifdef DO_ZERO_COPY
bool GLImageEGL::InitializeTextureMemory(gfx::GpuMemoryBufferHandle buffer, WebTech::TextureMemory* texture, unsigned internal_format) {
  //TRACE_EVENT0("gpu", "GLImageEGL::InitializeTextureMemory");
  ZEROCOPY_LOG("GLImageEGL::InitializeTextureMemory");
  EGLClientBuffer native_buffer;
  if (buffer.type == TEXTURE_MEMORY_BUFFER) {
    if (!texture) {
      swe::InitTextureMemory();
      texture_ = swe::CreateTextureMemory();
      if (!texture_) {
        ZEROCOPY_LOG(" CreateTextureMemory returned 0 ");
        return false;
      }
      int fds[2];
      fds[0] = buffer.fd1.fd;
      fds[1] = buffer.fd2.fd;
      texture_->Init((uint8_t*)(buffer.texture_memory_data.values_), buffer.texture_memory_data.size_, fds, 2);
      own_texture_ = true;
    } else {
      texture_ = texture;
      own_texture_ = false;
      texture_->Ref();
    }

    if (texture_)
      native_buffer = (EGLClientBuffer)(texture_->GetNativeHandle());
    else
      return false;

    EGLint attrs[] = {
      EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
      EGL_NONE,
    };
    return Initialize(EGL_NATIVE_BUFFER_ANDROID, native_buffer, attrs);
  }

  return false;
}

#endif

GLImageEGL::~GLImageEGL() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK_EQ(EGL_NO_IMAGE_KHR, egl_image_);
}

bool GLImageEGL::Initialize(EGLenum target,
                            EGLClientBuffer buffer,
                            const EGLint* attrs) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK_EQ(EGL_NO_IMAGE_KHR, egl_image_);
  egl_image_ = eglCreateImageKHR(GLSurfaceEGL::GetHardwareDisplay(),
                                 EGL_NO_CONTEXT,
                                 target,
                                 buffer,
                                 attrs);
  if (egl_image_ == EGL_NO_IMAGE_KHR) {
    DLOG(ERROR) << "Error creating EGLImage: " << ui::GetLastEGLErrorString();
    return false;
  }

  return true;
}

void GLImageEGL::Destroy(bool have_context) {
  DCHECK(thread_checker_.CalledOnValidThread());
  if (egl_image_ != EGL_NO_IMAGE_KHR) {
    EGLBoolean result =
        eglDestroyImageKHR(GLSurfaceEGL::GetHardwareDisplay(), egl_image_);
    if (result == EGL_FALSE) {
      DLOG(ERROR) << "Error destroying EGLImage: "
                  << ui::GetLastEGLErrorString();
    }
    egl_image_ = EGL_NO_IMAGE_KHR;
  }

#ifdef DO_ZERO_COPY
  if (texture_) {
    if (own_texture_)
      swe::DestroyTextureMemory(texture_);
    else
      texture_->Release();
    texture_ = 0;
  }
#endif
}

gfx::Size GLImageEGL::GetSize() { return size_; }

unsigned GLImageEGL::GetInternalFormat() { return GL_RGBA; }

bool GLImageEGL::BindTexImage(unsigned target) {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK_NE(EGL_NO_IMAGE_KHR, egl_image_);
  glEGLImageTargetTexture2DOES(target, egl_image_);
  DCHECK_EQ(static_cast<GLenum>(GL_NO_ERROR), glGetError());
  return true;
}

bool GLImageEGL::CopyTexSubImage(unsigned target,
                                 const Point& offset,
                                 const Rect& rect) {
  return false;
}

bool GLImageEGL::ScheduleOverlayPlane(gfx::AcceleratedWidget widget,
                                      int z_order,
                                      OverlayTransform transform,
                                      const Rect& bounds_rect,
                                      const RectF& crop_rect) {
  return false;
}

}  // namespace gfx

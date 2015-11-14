/** ---------------------------------------------------------------------------
 Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above
       copyright notice, this list of conditions and the following
       disclaimer in the documentation and/or other materials provided
       with the distribution.
     * Neither the name of The Linux Foundation nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------**/

#ifndef CONTENT_COMMON_SWEADRENO_TEXTURE_MEMORY_H_
#define CONTENT_COMMON_SWEADRENO_TEXTURE_MEMORY_H_

#ifndef NO_SWE_ADRENO_OPT

//undef these to disable zero-copy and/or texture atlas
#if defined ANDROID
#define DO_ZERO_COPY
#define DO_TEXTURE_ATLAS
#define DO_PARTIAL_COMPOSITION
#define DO_PARTIAL_SWAP

#include <android/log.h>
#define LOCAL_TAG_SWE "libsweadrenoext"

#endif

#if defined DO_PARTIAL_COMPOSITION
#define DO_TILED_RENDERING
#endif

#if (defined DO_ZERO_COPY && defined DO_TEXTURE_ATLAS)
#define DO_ZERO_COPY_WITH_ATLAS
#endif

#ifndef DO_ZERO_COPY

#define ZEROCOPY_LOG(...)
#define ZEROCOPY_LOG_VERBOSE(...)
#define ZEROCOPY_LOG_ERROR(...)

#else //DO_ZERO_COPY

#include "content/common/TextureMemory.h"
#include "ui/gfx/geometry/size.h"

#undef ZEROCOPY_LOG
#define ZEROCOPY_LOG(...)
#define ZEROCOPY_LOG_VERBOSE(...) __android_log_print(ANDROID_LOG_DEBUG, LOCAL_TAG_SWE, __VA_ARGS__);
#define ZEROCOPY_LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOCAL_TAG_SWE, __VA_ARGS__);

namespace swe {

class SerializedTextureMemory {
public:
  SerializedTextureMemory();
  SerializedTextureMemory(const SerializedTextureMemory& other);
  ~SerializedTextureMemory();
  void Allocate(size_t size);
  SerializedTextureMemory& operator= (const SerializedTextureMemory& other);
  size_t size_;
  uint8_t* values_;
};

bool InitTextureMemory();
WebTech::TextureMemory* CreateTextureMemory();
void DestroyTextureMemory(WebTech::TextureMemory* texture);
bool PrepareCreateTextureMemory();
void CancelPrepareCreateTextureMemory();
WebTech::TextureMemory* GetLastTextureMemory();
void ResetLastTextureMemory();
int GetTextureMemoryLimit();
#ifdef DO_ZERO_COPY_WITH_ATLAS
bool ShouldUseTextureAtlas();
void GetDesiredAtlasProperties(gfx::Size* atlas_size, gfx::Size* max_texture_size, int* padding);
#endif
}  // namespace content

#endif

#ifdef DO_PARTIAL_COMPOSITION
#define ZEROCOPY_LOG_PARTIAL_COMPOSITION(...)
#endif

#ifdef DO_PARTIAL_SWAP
#define ZEROCOPY_LOG_PARTIAL_SWAP(...)
#endif

#else

#define ZEROCOPY_LOG(...)
#define ZEROCOPY_LOG_VERBOSE(...)
#define ZEROCOPY_LOG_ERROR(...)
#define ZEROCOPY_LOG_PARTIAL_COMPOSITION(...)
#define ZEROCOPY_LOG_PARTIAL_SWAP(...)

#endif // NO_SWE_ADRENO_OPT

#endif  // CONTENT_COMMON_SWEADRENO_TEXTURE_MEMORY_H_

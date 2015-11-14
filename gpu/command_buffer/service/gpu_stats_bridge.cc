/** ---------------------------------------------------------------------------
 Copyright (c) 2015 The Linux Foundation. All rights reserved.

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
// This is an android-specific source file
#include <android/log.h>
#include <dlfcn.h>
#include <sys/system_properties.h>

#include "base/android/build_info.h"
#include "gpu_stats_bridge.h"
#include "GpuStats.h"

#define GPU_STATS_LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "GpuStats", __VA_ARGS__);
#define GPU_STATS_LOG_VERBOSE(...)
// #define GPU_STATS_LOG_VERBOSE(...) __android_log_print(ANDROID_LOG_DEBUG, "GpuStats", __VA_ARGS__);
#define GPU_STATS_LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, "GpuStats", __VA_ARGS__);

WebTech::CreateGpuStatsFunc GpuStatsBridge::create_gpu_stats_ = nullptr;

GpuStatsBridge::GpuStatsBridge() : gpu_stats_(nullptr) {
  if (create_gpu_stats_) {
    gpu_stats_ = std::unique_ptr<WebTech::GpuStats>(create_gpu_stats_());
  }
}

void GpuStatsBridge::LogCommand(unsigned int command, int limit) {
  if (gpu_stats_) {
    gpu_stats_->LogCommand(command, limit);
  }
}

void GpuStatsBridge::LoadSharedLib(const char* libname) {
  GPU_STATS_LOG("loading %s", libname);

  void* extlib = dlopen(libname, RTLD_LAZY);
  if (extlib) {
    WebTech::CheckGpuStatsVersionFunc check_version;
    check_version = (WebTech::CheckGpuStatsVersionFunc)(dlsym(extlib, CHECK_GPU_STATS_VERSION_FUNC_NAME));
    if (check_version) {
      GPU_STATS_LOG_VERBOSE("loading function CheckGpuStatsVersion succeeded.");
      if (check_version(GPU_STATS_VERSION, GPU_STATS_DEFINITION_TO_STRING(GPU_STATS_DEFINITION))) {
        create_gpu_stats_ = (WebTech::CreateGpuStatsFunc)(dlsym(extlib, CREATE_GPU_STATS_FUNC_NAME));
        if (create_gpu_stats_) {
          GPU_STATS_LOG_VERBOSE("loading function CreateGpuStats succeeded.");
          GPU_STATS_LOG_VERBOSE("loading %s succeeded", libname);
        } else {
          GPU_STATS_LOG_ERROR("loading function CreateGpuStats failed.");
        }
      }
    }
    else {
      GPU_STATS_LOG_ERROR("loading function CheckGpuStatsVersion failed.");
    }
  }
  else {
    GPU_STATS_LOG_ERROR("loading %s failed (%s) ", libname, dlerror());
  }
}

// This should be called once, by the GPU process, before any threads are created.
void GpuStatsBridge::Init() {
  base::android::BuildInfo* build_info = base::android::BuildInfo::GetInstance();
  if (build_info) {
    int sdk_number = build_info->sdk_int();
    if (sdk_number == 23) {
      LoadSharedLib("libsweadrenoext_23_plugin.so");
    } else if (sdk_number == 20 || sdk_number == 21 || sdk_number == 22) {
      LoadSharedLib("libsweadrenoext_22_plugin.so");
    }
  }
}

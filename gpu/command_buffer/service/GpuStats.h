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

#ifndef WEBTECH_GPU_STATS_H_
#define WEBTECH_GPU_STATS_H_

//interface GpuStats
//define the GpuStats class definition in the GPU_STATS_DEFINITION macro
//so that it can be stringified.  A string that contains the definition of the class
//can be created using GPU_STATS_DEFINITION_TO_STRING(GPU_STATS_DEFINITION)
//defined below.  The string can be passed from the client to the CheckGpuStatsVersionFunc()
//function below so that the GpuStats implementation can make sure it matches the definition
//used in the implementation.  The GpuStats implementation resides in a dynamic
//library.  This string matching ensure that the .so and the client application use
//the same GpuStats definition.

#define GPU_STATS_DEFINITION \
class GpuStats {\
public:\
    virtual ~GpuStats() {};\
    virtual void LogCommand(unsigned int command, int limit = 0) = 0;\
};\
typedef bool (*CheckGpuStatsVersionFunc)(int version, const char* definition_string);\
typedef WebTech::GpuStats* (*CreateGpuStatsFunc)();

#define GPU_STATS_DEFINITION_TO_STRING2(s) #s
#define GPU_STATS_DEFINITION_TO_STRING(s) GPU_STATS_DEFINITION_TO_STRING2(s)

//the actual definition of GpuStats class
namespace WebTech {
GPU_STATS_DEFINITION
}

#define GPU_STATS_VERSION 1
#define CHECK_GPU_STATS_VERSION_FUNC_NAME "CheckGpuStatsVersion"
#define CREATE_GPU_STATS_FUNC_NAME "CreateGpuStats"

#endif

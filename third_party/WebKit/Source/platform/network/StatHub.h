/*
* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef StatHub_h
#define StatHub_h

#include <map>
#include <wtf/text/CString.h>
#include "platform/SharedBuffer.h"
#include "platform/network/ResourceResponse.h"
#include "platform/weborigin/KURL.h"

namespace blink {

class LocalFrame;
class Page;

typedef enum {
    SH_CMD_MM_CACHE_CLEAR,
    SH_CMD_MM_CACHE_SET_STATUS,
    SH_CMD_MAIN_URL_WILL_START,
    SH_CMD_MAIN_URL_DID_FINISH,
    SH_CMD_RESOURCE_WILL_START_LOAD,
    SH_CMD_RESOURCE_DID_FINISH_LOAD,
    SH_CMD_RESOURCE_JS_SEQ,
    SH_CMD_RESOURCE_WILL_SEND_REQUEST,
    SH_CMD_PAGE_WILL_START_LOAD,
    SH_CMD_PAGE_DID_START_LOAD,
    SH_CMD_PAGE_DID_FINISH_LOAD,
    SH_CMD_PAGE_ON_LOAD,
    SH_CMD_PAGE_FIRST_PIXEL,
    SH_CMD_PAGE_PROGRESS_UPDATE,
    SH_CMD_INSPECTOR_SPY_TL_MSG
} StatHubCmdId;

typedef enum {
    SH_PAGELOAD_PR_DID_START_LOAD,
    SH_PAGELOAD_PR_DID_FINISH_LOAD,
    SH_PAGELOAD_PR_FAILD_LOAD,
    SH_PAGELOAD_PR_ON_LOAD,
    SH_PAGELOAD_PR_FIRST_PIXEL,
    SH_PAGELOAD_PR_PROGRESS_UPDATE,
} StatHubPageLoadProgressReportId;

typedef void* StatHubCmd;

class StatHub {
public:
    StatHub() { };
    virtual ~StatHub() { };

    static void InitOnce();
    static StatHubCmd cmdCreate(StatHubCmdId cmdId);
    static void cmdAddParamAsString(StatHubCmd cmd, const char* param);
    static void cmdAddParamAsBool(StatHubCmd cmd, bool param);
    static void cmdAddParamAsUint32(StatHubCmd cmd, unsigned int param);
    static void cmdAddParamAsPtr(StatHubCmd cmd, void* param);
    static void cmdAddParamAsBuf(StatHubCmd cmd, const void* param, unsigned int size);
    static bool cmdCommit(StatHubCmd cmd);
    static bool cmdCommitDelayed(StatHubCmd cmd, unsigned int delay);

    static unsigned long hash(const KURL& url);
    static unsigned int isPreloaded(const KURL& url);
    static bool getPreloaded(const KURL& url, unsigned int get_from,
        ResourceResponse& response, RefPtr<SharedBuffer>& data);
    static bool releasePreloaded(const KURL& url);
    static bool isPreloaderEnabled();
    static bool isPerfEnabled();
    static void pageLoadProgressReport(StatHubPageLoadProgressReportId reportId,
        LocalFrame* frame, bool main, unsigned int progress=0, const char* url=NULL);

    // ----------------------------- Inspector Spy ----------------------------------
    static unsigned long timeLineIdentifier(unsigned long identifier);
    static unsigned long timeLineIdentifier(const KURL& url, unsigned long identifier);
    static String timeLineRequestId(unsigned long identifier);

private:
    typedef std::map<unsigned long, unsigned long> RequestIdMapType;
    static RequestIdMapType* request_id_map_;
};

} // namespace blink

#endif

/** ---------------------------------------------------------------------------
 Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.

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
#ifndef WebStatHub_h
#define WebStatHub_h

#include "WebCommon.h"

namespace blink {

class WebData;
class WebString;
class WebURL;
class WebURLResponse;

typedef enum {
    WEBSH_CMD_MM_CACHE_CLEAR,               // 0
    WEBSH_CMD_MM_CACHE_SET_STATUS,          // 1
    WEBSH_CMD_MAIN_URL_WILL_START,          // 2
    WEBSH_CMD_MAIN_URL_DID_FINISH,          // 3
    WEBSH_CMD_RESOURCE_WILL_START_LOAD,     // 4
    WEBSH_CMD_RESOURCE_DID_FINISH_LOAD,     // 5
    WEBSH_CMD_RESOURCE_JS_SEQ,              // 6
    WEBSH_CMD_RESOURCE_WILL_SEND_REQUEST,   // 7
    WEBSH_CMD_PAGE_WILL_START_LOAD,         // 8
    WEBSH_CMD_PAGE_DID_START_LOAD,          // 9
    WEBSH_CMD_PAGE_DID_FINISH_LOAD,         // 10
    WEBSH_CMD_PAGE_ON_LOAD,                 // 11
    WEBSH_CMD_PAGE_FIRST_PIXEL,             // 12
    WEBSH_CMD_PAGE_PROGRESS_UPDATE,         // 13
    WEBSH_CMD_INSPECTOR_SPY_TL_MSG          // 14
} WebStatHubCmdId;

typedef void* WebStatHubCmd;

class WebStatHub {
public:
    WebStatHub() { }
    virtual ~WebStatHub() { }

    virtual WebStatHubCmd cmdCreate(WebStatHubCmdId cmdId) {return NULL;}
    virtual void cmdAddParamAsString(WebStatHubCmd cmd, const char* param) {};
    virtual void cmdAddParamAsBool(WebStatHubCmd cmd, bool param) {};
    virtual void cmdAddParamAsUint32(WebStatHubCmd cmd, unsigned int param) {};
    virtual void cmdAddParamAsPtr(WebStatHubCmd cdm, void* param) {};
    virtual void cmdAddParamAsBuf(WebStatHubCmd cdm, const void* param, unsigned int size) {};
    virtual bool cmdCommit(WebStatHubCmd cmd) { return false; }
    virtual bool cmdCommitDelayed(WebStatHubCmd cmd, unsigned int delay) { return false; }

    virtual unsigned long hash(const WebURL& url) { return 0; }
    virtual unsigned int isPreloaded(const WebURL& url) { return 0; }
    virtual bool getPreloaded(const WebURL& url, unsigned int get_from,
        WebURLResponse& response, WebData& data) { return false; }
    virtual bool releasePreloaded(const WebURL& url) { return false; }
    virtual bool isPreloaderEnabled() { return false; }
    virtual bool isPerfEnabled() {return false;}

    virtual bool isInspectorSpyEnabled() {return false;}
    virtual bool inspectorSpyEnable() {return false;}
    virtual bool inspectorSpyDisable()  {return false;}

};

} // namespace blink

#endif // WebStatHub_h

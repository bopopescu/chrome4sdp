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

#include "config.h"
#include "platform/network/StatHub.h"

#include "core/inspector/IdentifiersFactory.h"
#include "core/inspector/InspectorFrontendChannel.h"
#include "core/inspector/InspectorInstrumentation.h"
#include "core/inspector/InspectorState.h"
#include "core/inspector/InspectorTimelineAgent.h"
#include "core/inspector/InstrumentingAgents.h"
#include <wtf/text/StringBuilder.h>

#include "core/frame/LocalFrame.h"
#include "core/page/Page.h"
#include "core/loader/ProgressTracker.h"
#include "public/platform/Platform.h"
#include "public/platform/WebData.h"
#include "public/platform/WebStatHub.h"
#include "public/platform/WebString.h"
#include "public/platform/WebURL.h"
#include "public/platform/WebURLResponse.h"
#include <wtf/text/WTFString.h>
#include <wtf/CurrentTime.h>


namespace blink {

StatHub::RequestIdMapType* StatHub::request_id_map_ = NULL;

void StatHub::InitOnce() {
    if (!request_id_map_) {
        request_id_map_ = new RequestIdMapType;
    }
}

StatHubCmd StatHub::cmdCreate(StatHubCmdId cmdId) {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        return stat_hub->cmdCreate((WebStatHubCmdId)cmdId);
    }
    return NULL;
}

void StatHub::cmdAddParamAsString(StatHubCmd cmd, const char* param) {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        stat_hub->cmdAddParamAsString(cmd, param);
    }
}

void StatHub::cmdAddParamAsBool(StatHubCmd cmd, bool param) {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        stat_hub->cmdAddParamAsBool(cmd, param);
    }
}

void StatHub::cmdAddParamAsUint32(StatHubCmd cmd, unsigned int param) {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        stat_hub->cmdAddParamAsUint32(cmd, param);
    }
}

void StatHub::cmdAddParamAsPtr(StatHubCmd cmd, void* param){
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        stat_hub->cmdAddParamAsPtr(cmd, param);
    }
}

void StatHub::cmdAddParamAsBuf(StatHubCmd cmd, const void* param, unsigned int size){
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        stat_hub->cmdAddParamAsBuf(cmd, param, size);
    }
}

bool StatHub::cmdCommit(StatHubCmd cmd) {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        return stat_hub->cmdCommit(cmd);
    }
    return false;
}

bool StatHub::cmdCommitDelayed (void* cmd, unsigned int delay) {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        return stat_hub->cmdCommitDelayed(cmd, delay);
    }
    return false;
}

unsigned long StatHub::hash(const KURL& url) {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        return stat_hub->hash(WebURL(url));
    }
    return 0;
}

unsigned int StatHub::isPreloaded(const KURL& url) {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        return stat_hub->isPreloaded(WebURL(url));
    }
    return 0;
}

bool StatHub::getPreloaded(const KURL& url, unsigned int get_from,
    ResourceResponse& response, RefPtr<SharedBuffer>& data) {
    bool ret = false;

    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        WebURLResponse web_response;
        WebData web_data;

        ret = stat_hub->getPreloaded(WebURL(url), get_from, web_response, web_data);
        if (ret) {
            response = web_response.toResourceResponse();
            data = SharedBuffer::create(web_data.data(), web_data.size());
        }
    }
    return ret;
}

bool StatHub::releasePreloaded(const KURL& url) {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        return stat_hub->releasePreloaded(WebURL(url));
    }
    return false;
}

bool StatHub::isPreloaderEnabled() {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        return stat_hub->isPreloaderEnabled();
    }
    return false;
}

bool StatHub::isPerfEnabled() {
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub) {
        return stat_hub->isPerfEnabled();
    }
    return false;
}

void StatHub::pageLoadProgressReport(StatHubPageLoadProgressReportId reportId,
    LocalFrame* frame,  bool main, unsigned int progress, const char* url) {
    StatHubCmd cmd;

    if (!frame)
        return;
    switch(reportId) {
        case SH_PAGELOAD_PR_DID_START_LOAD:
            cmd = StatHub::cmdCreate(SH_CMD_PAGE_DID_START_LOAD);
            if (cmd) {
                WebStatHub* stat_hub = Platform::current()->statHub();
                if (stat_hub) {
                    stat_hub->inspectorSpyEnable();
                }
                StatHub::cmdAddParamAsPtr(cmd, frame);
                StatHub::cmdAddParamAsBool(cmd, main);
                StatHub::cmdAddParamAsString(cmd, url);
            }
            break;
        case SH_PAGELOAD_PR_DID_FINISH_LOAD:
        case SH_PAGELOAD_PR_FAILD_LOAD:
            cmd = StatHub::cmdCreate(SH_CMD_PAGE_DID_FINISH_LOAD);
            if (cmd) {
                WebStatHub* stat_hub = Platform::current()->statHub();
                if (stat_hub) {
                    stat_hub->inspectorSpyDisable();
                }
                StatHub::cmdAddParamAsPtr(cmd, frame);
                StatHub::cmdAddParamAsBool(cmd, main);
                if (frame->document())
                    StatHub::cmdAddParamAsString(cmd, frame->document()->baseURL().string().utf8().data());
                else
                    StatHub::cmdAddParamAsString(cmd, "N/A");
                StatHub::cmdAddParamAsBool(cmd, (SH_PAGELOAD_PR_FAILD_LOAD==reportId));
            }
            break;
        case SH_PAGELOAD_PR_ON_LOAD:
            cmd = StatHub::cmdCreate(SH_CMD_PAGE_ON_LOAD);
            if (cmd) {
                StatHub::cmdAddParamAsPtr(cmd, frame);
                StatHub::cmdAddParamAsBool(cmd, main);
                if (frame->document())
                    StatHub::cmdAddParamAsString(cmd, frame->document()->baseURL().string().utf8().data());
                if (frame->page())
                    StatHub::cmdAddParamAsUint32(cmd, frame->loader().progress().totalBytesReceived());
            }
            break;
        case SH_PAGELOAD_PR_FIRST_PIXEL:
            cmd = StatHub::cmdCreate(SH_CMD_PAGE_FIRST_PIXEL);
            if (cmd) {
                StatHub::cmdAddParamAsPtr(cmd, frame);
                StatHub::cmdAddParamAsBool(cmd, main);
                if (frame->document())
                    StatHub::cmdAddParamAsString(cmd, frame->document()->baseURL().string().utf8().data());
                if (frame->page())
                    StatHub::cmdAddParamAsUint32(cmd, frame->loader().progress().totalBytesReceived());
            }
            break;
        case SH_PAGELOAD_PR_PROGRESS_UPDATE:
            cmd = StatHub::cmdCreate(SH_CMD_PAGE_PROGRESS_UPDATE);
            if (cmd) {
                StatHub::cmdAddParamAsPtr(cmd, frame);
                StatHub::cmdAddParamAsBool(cmd, main);
                if (frame->page())
                    StatHub::cmdAddParamAsUint32(cmd, frame->loader().progress().totalBytesReceived());
                else
                    StatHub::cmdAddParamAsUint32(cmd, 0);
                StatHub::cmdAddParamAsUint32(cmd, progress);
            }
            break;
        default:
            return;
    }
    if (cmd) {
        StatHub::cmdCommit(cmd);
    }
}


// ----------------------------- Inspector Spy ----------------------------------

unsigned long StatHub::timeLineIdentifier(unsigned long identifier) {
    InitOnce();
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub && stat_hub->isInspectorSpyEnabled()) {
        RequestIdMapType::iterator request_id_iter = request_id_map_->find(identifier);
        if (request_id_iter != request_id_map_->end() ) {
            identifier = request_id_iter->second;
        }
    }
    return identifier;
}

unsigned long StatHub::timeLineIdentifier(const KURL& url, unsigned long identifier) {
    InitOnce();
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub && stat_hub->isInspectorSpyEnabled()) {
        unsigned long reqestId = (unsigned long)StatHub::hash(url);
        RequestIdMapType::iterator request_id_iter = request_id_map_->find(identifier);
        if (request_id_iter == request_id_map_->end()) {
            request_id_map_->insert(std::pair<unsigned long, unsigned long>(identifier, reqestId));
        }
        else {
            request_id_iter->second = reqestId;
        }
        identifier = reqestId;
    }
    return identifier;
}

String StatHub::timeLineRequestId(unsigned long identifier) {
    InitOnce();
    WebStatHub* stat_hub = Platform::current()->statHub();
    if (stat_hub && stat_hub->isInspectorSpyEnabled()) {
        StringBuilder builder;
        builder.appendNumber(timeLineIdentifier(identifier));
        return builder.toString();
    }
    return IdentifiersFactory::requestId(identifier);
}

} // namespace blink

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

#include "base/format_macros.h"
#include "base/time/time.h"
#include "base/memory/singleton.h"
#include "base/strings/stringprintf.h"
#include "content/child/webstathub_impl.h"
#include "third_party/WebKit/public/platform/WebData.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebURL.h"
#include "third_party/WebKit/public/platform/WebURLResponse.h"

//Enable IPC API
#define STAT_HUB_API_BY_IPC

#include "net/stat_hub/stat_hub_api.h"
#include "net/stat_hub/stat_hub_cmd_api.h"
#include "net/http/http_response_headers.h"

//Disable IPC API
#undef STAT_HUB_API_BY_IPC

namespace content {

WebStatHubImpl::WebStatHubImpl():
  inspector_spy_on_(0) {
}

WebStatHubImpl::~WebStatHubImpl() {
}

WebStatHubImpl* WebStatHubImpl::GetInstance() {
  return Singleton<WebStatHubImpl>::get();
}

blink::WebStatHubCmd WebStatHubImpl::cmdCreate(blink::WebStatHubCmdId cmdId) {
    StatHubCmdType cmd;
    StatHubActionType action;

    switch(cmdId) {
        case blink::WEBSH_CMD_MM_CACHE_CLEAR:
            cmd = SH_CMD_WK_MEMORY_CACHE;
            action = SH_ACTION_CLEAR;
            break;
        case blink::WEBSH_CMD_MM_CACHE_SET_STATUS:
            cmd = SH_CMD_WK_MEMORY_CACHE;
            action = SH_ACTION_STATUS;
            break;
        case blink::WEBSH_CMD_MAIN_URL_WILL_START:
            cmd = SH_CMD_WK_MAIN_URL;
            action = SH_ACTION_WILL_START;
            break;
        case blink::WEBSH_CMD_MAIN_URL_DID_FINISH:
            cmd = SH_CMD_WK_MAIN_URL;
            action = SH_ACTION_DID_FINISH;
            break;
        case blink::WEBSH_CMD_RESOURCE_WILL_START_LOAD:
            cmd = SH_CMD_WK_RESOURCE;
            action = SH_ACTION_WILL_START_LOAD;
            break;
        case blink::WEBSH_CMD_RESOURCE_DID_FINISH_LOAD:
            cmd = SH_CMD_WK_RESOURCE;
            action = SH_ACTION_DID_FINISH_LOAD;
            break;
        case blink::WEBSH_CMD_RESOURCE_JS_SEQ:
            cmd = SH_CMD_WK_RESOURCE;
            action = SH_ACTION_JS_SEQ;
            break;
        case blink::WEBSH_CMD_RESOURCE_WILL_SEND_REQUEST:
            cmd = SH_CMD_WK_RESOURCE;
            action = SH_ACTION_WILL_SEND_REQUEST;
            break;
        case blink::WEBSH_CMD_PAGE_WILL_START_LOAD:
            cmd = SH_CMD_WK_PAGE;
            action = SH_ACTION_WILL_START_LOAD;
            break;
        case blink::WEBSH_CMD_PAGE_DID_START_LOAD:
            cmd = SH_CMD_WK_PAGE;
            action = SH_ACTION_DID_START_LOAD;
            break;
        case blink::WEBSH_CMD_PAGE_DID_FINISH_LOAD:
            cmd = SH_CMD_WK_PAGE;
            action = SH_ACTION_DID_FINISH_LOAD;
            break;
        case blink::WEBSH_CMD_PAGE_ON_LOAD:
            cmd = SH_CMD_WK_PAGE;
            action = SH_ACTION_ON_LOAD;
            break;
        case blink::WEBSH_CMD_PAGE_FIRST_PIXEL:
            cmd = SH_CMD_WK_PAGE;
            action = SH_ACTION_FIRST_PIXEL;
            break;
        case blink::WEBSH_CMD_PAGE_PROGRESS_UPDATE:
            cmd = SH_CMD_WK_PAGE;
            action = SH_ACTION_PROGRESS_UPDATE;
            break;
        case blink::WEBSH_CMD_INSPECTOR_SPY_TL_MSG:
            cmd = SH_CMD_WK_INSPECTOR_RECORD;
            action = SH_ACTION_NONE;
            break;
        default:
            return NULL;
    }
    return (blink::WebStatHubCmd)STAT_HUB_API(CmdCreate)(cmd, action, 0);
}

void WebStatHubImpl::cmdAddParamAsString(blink::WebStatHubCmd cmd, const char* param) {
    if(cmd) {
        StatHubCmd* stat_hub_cmd = (StatHubCmd*)cmd;
        stat_hub_cmd->AddParamAsString(param);
    }
}

void WebStatHubImpl::cmdAddParamAsBool(blink::WebStatHubCmd cmd, bool param) {
    if(cmd) {
        StatHubCmd* stat_hub_cmd = (StatHubCmd*)cmd;
        stat_hub_cmd->AddParamAsBool(param);
    }
}

void WebStatHubImpl::cmdAddParamAsUint32(blink::WebStatHubCmd cmd, unsigned int param) {
    if(cmd) {
        StatHubCmd* stat_hub_cmd = (StatHubCmd*)cmd;
        stat_hub_cmd->AddParamAsUint32(param);
    }
}

void WebStatHubImpl::cmdAddParamAsPtr(blink::WebStatHubCmd cmd, void* param) {
    if(cmd) {
        StatHubCmd* stat_hub_cmd = (StatHubCmd*)cmd;
        stat_hub_cmd->AddParamAsPtr(param);
    }
}

void WebStatHubImpl::cmdAddParamAsBuf(blink::WebStatHubCmd cmd, const void* param, unsigned int size) {
    if(cmd) {
        StatHubCmd* stat_hub_cmd = (StatHubCmd*)cmd;
        stat_hub_cmd->AddParamAsBuf(param, size);
    }
}

bool WebStatHubImpl::cmdCommit(blink::WebStatHubCmd cmd) {
    return STAT_HUB_API(CmdCommit)((StatHubCmd*)cmd);
}

bool WebStatHubImpl::cmdCommitDelayed (blink::WebStatHubCmd cmd, unsigned int delay) {
    return STAT_HUB_API(CmdCommitDelayed)((StatHubCmd*)cmd, delay);
}

unsigned long WebStatHubImpl::hash(const blink::WebURL& url){
  return STAT_HUB_API(Hash)(url.spec().data());
}

unsigned int WebStatHubImpl::isPreloaded(const blink::WebURL& url) {
  return STAT_HUB_API(IsPreloaded)(url.spec().data());
}

bool WebStatHubImpl::getPreloaded(const blink::WebURL& url, unsigned int get_from,
    blink::WebURLResponse& response, blink::WebData& data) {
  std::string data_tmp;
  unsigned int size;
  std::string headers;

  if (STAT_HUB_API(GetPreloaded)(url.spec().data(), get_from, headers, data_tmp, size)) {
      scoped_refptr<net::HttpResponseHeaders> responseHeaders;
      responseHeaders = new net::HttpResponseHeaders(headers);

      std::string mime;
      std::string encoding;
      responseHeaders->GetMimeType(&mime);
      responseHeaders->GetCharset(&encoding);

      response.initialize();
      response.setURL(url);
      response.setMIMEType(blink::WebString::fromUTF8(mime));
      response.setTextEncodingName(blink::WebString::fromUTF8(encoding));
      response.setExpectedContentLength(size);

      blink::WebURLResponse::HTTPVersion version = blink::WebURLResponse::Unknown;
      if (responseHeaders->GetHttpVersion() == net::HttpVersion(0, 9))
        version = blink::WebURLResponse::HTTP_0_9;
      else if (responseHeaders->GetHttpVersion() == net::HttpVersion(1, 0))
        version = blink::WebURLResponse::HTTP_1_0;
      else if (responseHeaders->GetHttpVersion() == net::HttpVersion(1, 1))
        version = blink::WebURLResponse::HTTP_1_1;
      int httpStatusCode = responseHeaders->response_code();
      std::string httpStatusText = responseHeaders->GetStatusText();
      response.setHTTPVersion(version);
      response.setHTTPStatusCode(httpStatusCode);
      response.setHTTPStatusText(blink::WebString::fromUTF8(httpStatusText));

      base::Time time_val;
      if (responseHeaders->GetLastModifiedValue(&time_val)) {
        response.setLastModifiedDate(time_val.ToDoubleT());
      }

      // Build up the header map.
      void* iter = NULL;
      std::string value;
      std::string name;
      while (responseHeaders->EnumerateHeaderLines(&iter, &name, &value)) {
          response.addHTTPHeaderField(blink::WebString::fromUTF8(name),
              blink::WebString::fromUTF8(value));
      }

      data.assign(data_tmp.data(), size);
      return true;
  }
  return false;
}

bool WebStatHubImpl::releasePreloaded(const blink::WebURL& url) {
    return STAT_HUB_API(ReleasePreloaded)(url.spec().data());
}

bool WebStatHubImpl::isPreloaderEnabled() {
    return STAT_HUB_API(IsPreloaderEnabled)();
}

bool WebStatHubImpl::isPerfEnabled() {
    return STAT_HUB_API(IsPerfEnabled)();
}

bool WebStatHubImpl::isInspectorSpyEnabled() {
    unsigned int mask = STAT_HUB_API(GetCmdMask)();
    return (mask & (1<<SH_CMD_WK_INSPECTOR_RECORD));
}

 class MyData : public base::trace_event::ConvertableToTraceFormat {
public:
     MyData() {}
     virtual void AppendAsTraceFormat(std::string* out) const override {
       out->append("{\"foo\":1}");
     }
private:
     virtual ~MyData() {}
     DISALLOW_COPY_AND_ASSIGN(MyData);
};

typedef std::map<std::string, std::string> TraceMsgMapType;
static TraceMsgMapType* trace_msg_map = NULL;

static void inspectorSpyCallback(base::TraceTicks timestamp,
        char phase,
        const unsigned char* category_group_enabled,
        const char* name,
        unsigned long long id,
        int num_args,
        const char* const arg_names[],
        const unsigned char arg_types[],
        const unsigned long long arg_values[],
        const scoped_refptr<base::trace_event::ConvertableToTraceFormat>* convertable_values,
        unsigned int flags) {

    if (!trace_msg_map) {
        return;
    }

    std::string msg;
    std::string out;

    msg += "";
    std::string name_str = name;
    std::replace(name_str.begin(), name_str.end(), ' ', '_');
    base::StringAppendF(&msg, "\"type\":\"%s\",", name_str.c_str());

    //TBD: base::StringAppendF(&msg, "\"startTime\":%" PRId64, timestamp.ToInternalValue());
    StatHubTimeStamp start_timestamp = StatHubTimeStamp::NowFromSystemTime();
    base::StringAppendF(&msg, "\"startTime\":%f", start_timestamp.ToDoubleT()*1000);

    base::trace_event::TraceEvent::TraceValue arg_value;
    for (int i = 0; i < num_args; ++i) {
        name_str = arg_names[i];
        std::replace(name_str.begin(), name_str.end(), ' ', '_');
        msg += ",";
        out = "";
        if (arg_types[i] == TRACE_VALUE_TYPE_CONVERTABLE) {
            convertable_values[i]->AppendAsTraceFormat(&out);
        }
        else {
            arg_value.as_uint = arg_values[i];
            base::trace_event::TraceEvent::TraceEvent::AppendValueAsJSON(arg_types[i], arg_value, &out);
        }
        base::StringAppendF(&msg, "\"%s\":", arg_names[i]);
        msg += out;
    }
    //base::TimeTicks end_timestamp = timestamp;
    StatHubTimeStamp end_timestamp = StatHubTimeStamp::NowFromSystemTime();

    switch(phase) {
        case TRACE_EVENT_PHASE_BEGIN:
            //push message
            trace_msg_map->insert(std::pair<std::string, std::string>(
                    name, msg));
            break;
        case TRACE_EVENT_PHASE_END:
            //pop message
            {
                TraceMsgMapType::iterator trace_msg_iter = trace_msg_map->find(name);
                if (trace_msg_iter == trace_msg_map->end() ) {
                    break;
                }
                msg = trace_msg_iter->second;
                trace_msg_map->erase(trace_msg_iter);
            }
        case TRACE_EVENT_PHASE_INSTANT:
            //TBD: base::StringAppendF(&msg, ",\"endTime\":%" PRId64, end_timestamp.ToInternalValue());
            base::StringAppendF(&msg, ",\"endTime\":%f", end_timestamp.ToDoubleT()*1000);

            {
                StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_WK_INSPECTOR_RECORD, SH_ACTION_NONE, 0);
                if (NULL!=cmd) {
                    cmd->AddParamAsString(msg.c_str());
                    STAT_HUB_API(CmdCommit)(cmd);
                }
            }
            break;
        default:
            break;
    }

}

bool WebStatHubImpl::inspectorSpyEnable() {
    if (isInspectorSpyEnabled()) {
        if (!inspector_spy_on_) {
            if (!trace_msg_map) {
                trace_msg_map = new TraceMsgMapType;
            }
            std::string filter;
            //-*,disabled-by-default-devtools.timeline,disabled-by-default-devtools.timeline.frame,blink.console,disabled-by-default-devtools.timeline.stack
            filter += "*-,";
            filter += TRACE_DISABLED_BY_DEFAULT("devtools.timeline");
            filter += ",";
            filter += TRACE_DISABLED_BY_DEFAULT("devtools.timeline.frame");
            filter += ",";
            filter += "blink.console";
            filter += ",";
            filter += TRACE_DISABLED_BY_DEFAULT("devtools.timeline.stack");
            base::trace_event::TraceLog::GetInstance()->SetEventCallbackEnabled(
                    base::trace_event::TraceConfig(filter,""),
                    inspectorSpyCallback
            );
        }
        inspector_spy_on_++;
    }
    return true;
}

bool WebStatHubImpl::inspectorSpyDisable() {
    if (isInspectorSpyEnabled() && inspector_spy_on_) {
        inspector_spy_on_--;
        if(!inspector_spy_on_) {
            base::trace_event::TraceLog::GetInstance()->SetEventCallbackDisabled();
            if (!trace_msg_map) {
                delete trace_msg_map;
                trace_msg_map = NULL;
            }
        }
    }
    return true;
}

}

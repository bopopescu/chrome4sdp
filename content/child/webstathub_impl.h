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
#ifndef CONTENT_CHILD_WEBSTATHUB_IMPL_H_
#define CONTENT_CHILD_WEBSTATHUB_IMPL_H_

#include "base/macros.h"
#include "base/trace_event/trace_event.h"
#include "content/common/content_export.h"
#include "third_party/WebKit/public/platform/WebStatHub.h"

namespace blink {
class WebData;
class WebString;
class WebURL;
class WebURLResponse;
}

template <typename T> struct DefaultSingletonTraits;

namespace content {

class CONTENT_EXPORT WebStatHubImpl : public NON_EXPORTED_BASE(blink::WebStatHub) {
 public:

  virtual ~WebStatHubImpl();

  static WebStatHubImpl* GetInstance();

  virtual blink::WebStatHubCmd cmdCreate(blink::WebStatHubCmdId cmdId);
  virtual void cmdAddParamAsString(blink::WebStatHubCmd cmd, const char* param);
  virtual void cmdAddParamAsBool(blink::WebStatHubCmd cmd, bool param);
  virtual void cmdAddParamAsUint32(blink::WebStatHubCmd cmd, unsigned int param);
  virtual void cmdAddParamAsPtr(blink::WebStatHubCmd cmd, void* param);
  virtual void cmdAddParamAsBuf(blink::WebStatHubCmd cmd, const void* param, unsigned int size);
  virtual bool cmdCommit(blink::WebStatHubCmd cmd);
  virtual bool cmdCommitDelayed(blink::WebStatHubCmd cmd, unsigned int delay);

  virtual unsigned long hash(const blink::WebURL& url);
  virtual unsigned int isPreloaded(const blink::WebURL& url);
  virtual bool getPreloaded(const blink::WebURL& url, unsigned int get_from,
      blink::WebURLResponse& response, blink::WebData& data);
  virtual bool releasePreloaded(const blink::WebURL& url);
  virtual bool isPreloaderEnabled();
  virtual bool isPerfEnabled();

  virtual bool isInspectorSpyEnabled();
  virtual bool inspectorSpyEnable();
  virtual bool inspectorSpyDisable();

 private:
  WebStatHubImpl();
  friend struct DefaultSingletonTraits<WebStatHubImpl>;

  int inspector_spy_on_;

  DISALLOW_COPY_AND_ASSIGN(WebStatHubImpl);
};

} // namespace webkit_glue

#endif //CONTENT_CHILD_WEBSTATHUB_IMPL_H_

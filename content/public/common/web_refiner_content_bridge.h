/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
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

#ifndef WEB_REFINER_CONTENT_BRIDGE_H_
#define WEB_REFINER_CONTENT_BRIDGE_H_

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/process/kill.h"
#include "url/gurl.h"

#include "content/public/common/web_refiner_export.h"

namespace net {
class URLRequest;
}

namespace IPC {
class Message;
}

namespace blink {
class WebLocalFrame;
struct WebURLError;
}

namespace content {
class WebRefiner;
class RenderView;
class ResourceRequestInfo;
class RenderProcessHost;
class WebContents;
class ContentViewCore;
class RenderFrameHost;
class RenderThread;

class RenderViewObserverBridge;
class RenderProcessObserverBridge;
class RenderProcessHostObserverBridge;
class WebContentsObserverBridge;

/************ BrowserThread wrappers **************/
WEB_REFINER_EXPORT extern const int BrowserThread_ID_UI;
WEB_REFINER_EXPORT extern const int BrowserThread_ID_DB;
WEB_REFINER_EXPORT extern const int BrowserThread_ID_FILE;
WEB_REFINER_EXPORT extern const int BrowserThread_ID_IO;
WEB_REFINER_EXPORT extern const int ChildProcessHost_kInvalidUniqueID;

WEB_REFINER_EXPORT void Assert_Currently_On(int id);
WEB_REFINER_EXPORT void BrowserThread_PostTask(int identifier,
                       const tracked_objects::Location& from_here,
                       const base::Closure& task);


/************ ResourceRequestInfo wrappers **************/
WEB_REFINER_EXPORT const ResourceRequestInfo* ResourceRequestInfo_ForRequest(const net::URLRequest*);


/************ RenderProcessObserver wrappers **************/
class WEB_REFINER_EXPORT RenderProcessObserverExt {
public:
    RenderProcessObserverExt(RenderThread* render_thread);
    virtual ~RenderProcessObserverExt();
    virtual bool OnControlMessageReceived(const IPC::Message& message) { return false; };
    virtual void OnRenderProcessShutdown() {}
    virtual void WebKitInitialized() {}
private:
    DISALLOW_COPY_AND_ASSIGN(RenderProcessObserverExt);
    RenderProcessObserverBridge* bridge_;
};


/************ RenderProcessHostObserver wrappers **************/
class WEB_REFINER_EXPORT RenderProcessHostObserverExt {
public:
    RenderProcessHostObserverExt();
    virtual ~RenderProcessHostObserverExt();
    virtual void RenderProcessWillExit(RenderProcessHost* host) {}
    virtual void RenderProcessExited(RenderProcessHost* host, base::TerminationStatus status, int exit_code) {}
    virtual void RenderProcessHostDestroyed(RenderProcessHost* host) {}
    void AddAsObserverFor(RenderProcessHost* process_host);
private:
    DISALLOW_COPY_AND_ASSIGN(RenderProcessHostObserverExt);
    RenderProcessHostObserverBridge* bridge_;
};
WEB_REFINER_EXPORT int RenderProcessHost_GetID(RenderProcessHost*);
WEB_REFINER_EXPORT bool RenderProcessHost_Send(RenderProcessHost*, IPC::Message* msg);


/************ WebContentsObserver wrappers **************/
class WEB_REFINER_EXPORT WebContentsObserverExt {
public:
    WebContentsObserverExt(WebContents* web_contents);
    virtual ~WebContentsObserverExt();
    virtual void DidStartLoading() {}
    virtual void DidStopLoading() {}
    virtual void DidStartProvisionalLoadForFrame(
        RenderFrameHost* render_frame_host, const GURL& validated_url,
        bool is_error_page, bool is_iframe_srcdoc) {}
    virtual void DidFinishLoad(RenderFrameHost* render_frame_host,
                               const GURL& validated_url) {}
    virtual void DidFailLoad(RenderFrameHost* render_frame_host,
                             const GURL& validated_url,
                             int error_code, const base::string16& error_description,
                             bool was_ignored_by_handler) {}
    virtual void WebContentsDestroyed() {}
protected:
    WebContents* web_contents() const;
    int GetProcessID() const;
    int GetRoutingID() const;
    bool IsOffTheRecord() const;
private:
    DISALLOW_COPY_AND_ASSIGN(WebContentsObserverExt);
    WebContentsObserverBridge* bridge_;
};
WEB_REFINER_EXPORT WebContents* ContentViewCore_GetWebContents(content::ContentViewCore*);
WEB_REFINER_EXPORT int WebContents_GetProcessID(WebContents*);
WEB_REFINER_EXPORT int WebContents_GetRoutingID(WebContents*);
WEB_REFINER_EXPORT RenderFrameHost* RenderFrameHost_GetParent(RenderFrameHost*);


/************ RenderViewObserver wrappers **************/
class WEB_REFINER_EXPORT RenderViewObserverExt {
public:
    RenderViewObserverExt(RenderView* render_view);
    virtual ~RenderViewObserverExt();
    virtual bool OnMessageReceived(const IPC::Message& message) { return false; }
    virtual void DidStartLoading() {}
    virtual void DidStopLoading() {}
    virtual void DidFailLoad(blink::WebLocalFrame* frame, const blink::WebURLError& error) {}
    virtual void DidFinishLoad(blink::WebLocalFrame* frame) {}
    virtual void DidFailProvisionalLoad(blink::WebLocalFrame* frame, const blink::WebURLError& error) {}
private:
    DISALLOW_COPY_AND_ASSIGN(RenderViewObserverExt);
    RenderViewObserverBridge* bridge_;
};

} // namespace content

#endif // WEB_REFINER_CONTENT_BRIDGE_H_

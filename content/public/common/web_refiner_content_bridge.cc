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

#include "web_refiner_content_bridge.h"

#include "content/public/browser/android/content_view_core.h"
#include "content/browser/frame_host/render_frame_host_impl.h"
#include "content/browser/renderer_host/render_process_host_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/common/child_process_host.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_process_host_observer.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/renderer/render_process_observer.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "content/renderer/render_thread_impl.h"
#include "content/renderer/render_view_impl.h"


namespace content {

/************ BrowserThread wrappers **************/
const int BrowserThread_ID_UI = BrowserThread::UI;
const int BrowserThread_ID_DB = BrowserThread::DB;
const int BrowserThread_ID_FILE = BrowserThread::FILE;
const int BrowserThread_ID_IO = BrowserThread::IO;
const int ChildProcessHost_kInvalidUniqueID = ChildProcessHost::kInvalidUniqueID;

void Assert_Currently_On(int id) {
    DCHECK_CURRENTLY_ON(static_cast<content::BrowserThread::ID>(id));
}

void BrowserThread_PostTask(int identifier,
                       const tracked_objects::Location& from_here,
                       const base::Closure& task) {
    BrowserThread::PostTask(static_cast<content::BrowserThread::ID>(identifier), from_here, task);
}

/************ ResourceRequestInfo wrappers **************/
const ResourceRequestInfo* ResourceRequestInfo_ForRequest(const net::URLRequest* request) {
    return ResourceRequestInfo::ForRequest(request);
}


/************ RenderProcessObserver wrappers **************/
class RenderProcessObserverBridge : public content::RenderProcessObserver {
public:
    RenderProcessObserverBridge(RenderProcessObserverExt* ext) : ext_(ext) {}
    ~RenderProcessObserverBridge() override {}
    bool OnControlMessageReceived(const IPC::Message& message) override { return ext_->OnControlMessageReceived(message); };
    void OnRenderProcessShutdown() override { ext_->OnRenderProcessShutdown(); }
    void WebKitInitialized() override { ext_->WebKitInitialized(); }
private:
    RenderProcessObserverExt* ext_;
};

RenderProcessObserverExt::RenderProcessObserverExt(RenderThread* render_thread) {
    bridge_ = new RenderProcessObserverBridge(this);
    render_thread->AddObserver(bridge_);
}

RenderProcessObserverExt::~RenderProcessObserverExt() {
    delete bridge_;
}


/************ RenderProcessHostObserver wrappers **************/

class RenderProcessHostObserverBridge : public RenderProcessHostObserver {
public:
    RenderProcessHostObserverBridge(RenderProcessHostObserverExt* ext) : ext_(ext) {}
    ~RenderProcessHostObserverBridge() override {}
    void RenderProcessWillExit(RenderProcessHost* host) override { ext_->RenderProcessWillExit(host); }
    void RenderProcessExited(RenderProcessHost* host, base::TerminationStatus status,
                                   int exit_code) override { ext_->RenderProcessExited(host, status, exit_code); }
    void RenderProcessHostDestroyed(RenderProcessHost* host) override { ext_->RenderProcessHostDestroyed(host); }
private:
    RenderProcessHostObserverExt* ext_;
};

RenderProcessHostObserverExt::RenderProcessHostObserverExt() {
    bridge_ = new RenderProcessHostObserverBridge(this);
}

RenderProcessHostObserverExt::~RenderProcessHostObserverExt() {
    delete bridge_;;
}

void RenderProcessHostObserverExt::AddAsObserverFor(RenderProcessHost* process_host) {
    process_host->AddObserver(bridge_);
}

int RenderProcessHost_GetID(RenderProcessHost* render_process_host) {
    return render_process_host ? render_process_host->GetID() : ChildProcessHost::kInvalidUniqueID;
}

bool RenderProcessHost_Send(RenderProcessHost* render_process_host, IPC::Message* msg) {
    return render_process_host ? render_process_host->Send(msg) : false;
}


/************ WebContentsObserver wrappers **************/

class WebContentsObserverBridge : public WebContentsObserver {
public:
    WebContentsObserverBridge(WebContents* web_contents, WebContentsObserverExt* ext)
        : WebContentsObserver(web_contents)
        , ext_(ext) {}
    ~WebContentsObserverBridge() override {}
    void DidStartLoading() override { ext_->DidStartLoading(); }
    void DidStopLoading() override { ext_->DidStopLoading(); }
    void DidStartProvisionalLoadForFrame(
        RenderFrameHost* render_frame_host, const GURL& validated_url,
        bool is_error_page, bool is_iframe_srcdoc) override {
        ext_->DidStartProvisionalLoadForFrame(render_frame_host, validated_url, is_error_page, is_iframe_srcdoc);
    }
    void DidFinishLoad(RenderFrameHost* render_frame_host,
                               const GURL& validated_url) override { ext_->DidFinishLoad(render_frame_host, validated_url); }
    void DidFailLoad(RenderFrameHost* render_frame_host,
                             const GURL& validated_url,
                             int error_code, const base::string16& error_description,
                             bool was_ignored_by_handler) override {
        ext_->DidFailLoad(render_frame_host, validated_url, error_code, error_description, was_ignored_by_handler);
    }
    void WebContentsDestroyed() override { ext_->WebContentsDestroyed(); }
private:
    WebContentsObserverExt* ext_;
};

WebContentsObserverExt::WebContentsObserverExt(WebContents* web_contents) {
    bridge_ = new WebContentsObserverBridge(web_contents, this);
}

WebContentsObserverExt::~WebContentsObserverExt() {
    delete bridge_;
}

WebContents* WebContentsObserverExt::web_contents() const {
    return bridge_->web_contents();
}

int WebContentsObserverExt::GetProcessID() const {
    return WebContents_GetProcessID(web_contents());
}

int WebContentsObserverExt::GetRoutingID() const {
    return WebContents_GetRoutingID(web_contents());
}

bool WebContentsObserverExt::IsOffTheRecord() const {
    if (web_contents() && web_contents()->GetBrowserContext())
        return web_contents()->GetBrowserContext()->IsOffTheRecord();

    return false;
}


WebContents* ContentViewCore_GetWebContents(content::ContentViewCore* content_view_core) {
    return content_view_core ? content_view_core->GetWebContents() : 0;
}

int WebContents_GetProcessID(WebContents* web_contents) {
    if (web_contents && web_contents->GetRenderProcessHost())
        return web_contents->GetRenderProcessHost()->GetID();

    return ChildProcessHost::kInvalidUniqueID;
}

int WebContents_GetRoutingID(WebContents* web_contents) {
    return web_contents ? web_contents->GetRoutingID() : MSG_ROUTING_NONE;
}

RenderFrameHost* RenderFrameHost_GetParent(RenderFrameHost* render_frame_host) {
    return render_frame_host ? render_frame_host->GetParent() : 0;
}

/************ RenderViewObserver wrappers **************/
class RenderViewObserverBridge : public RenderViewObserver {
public:
    RenderViewObserverBridge(RenderView* render_view, RenderViewObserverExt* ext)
        : RenderViewObserver(render_view)
        , ext_(ext) {}
    ~RenderViewObserverBridge() override {}
    bool OnMessageReceived(const IPC::Message& message) override { return ext_->OnMessageReceived(message); }
    void DidStartLoading() override { ext_->DidStartLoading(); }
    void DidStopLoading() override { ext_->DidStopLoading(); }
    void DidFailLoad(blink::WebLocalFrame* frame, const blink::WebURLError& error) override { ext_->DidFailLoad(frame, error); }
    void DidFinishLoad(blink::WebLocalFrame* frame) override { ext_->DidFinishLoad(frame); }
    void DidFailProvisionalLoad(blink::WebLocalFrame* frame, const blink::WebURLError& error) override { ext_->DidFailProvisionalLoad(frame, error); }
private:
    RenderViewObserverExt* ext_;
};

RenderViewObserverExt::RenderViewObserverExt(RenderView* render_view) {
    bridge_ = new RenderViewObserverBridge(render_view, this);
    static_cast<RenderViewImpl*>(render_view)->RemoveObserver(bridge_);
    static_cast<RenderViewImpl*>(render_view)->AddObserver(bridge_);
}

RenderViewObserverExt::~RenderViewObserverExt() {
    delete bridge_;
}

} //namespace content

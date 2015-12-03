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

#include "web_refiner.h"

#include "base/lazy_instance.h"
#include "base/time/time.h"

namespace content {

class MockWebRefiner :  public WebRefiner {

public:
    MockWebRefiner() { }
    ~MockWebRefiner() override { }

    void RegisterRenderViewImpl(RenderView*) override { }

    void RegisterRenderThreadImpl(content::RenderThread*) override { }

    void RegisterRenderProcessHostImpl(RenderProcessHost*) override { }

    void RegisterRenderFrameHostImpl(RenderFrameHost*) override { }

    void UnRegisterRenderFrameHostImpl(RenderFrameHost*) override { }

    void RegisterWebContentsImpl(content::WebContents*) override { }

    void OnBeforeURLRequest(net::URLRequest*, int*) override { }

    void OnHeadersReceived(net::URLRequest*, const net::HttpResponseHeaders*, scoped_refptr<net::HttpResponseHeaders>*) override { }

    void OnCompleted(net::URLRequest*, bool) override { }

    bool OnCanGetCookies(const net::URLRequest*, const net::CookieList&) override { return true; }

    bool AllowGetCookies(int, int, int, const GURL&, const GURL&, const net::CookieList&) override { return true; }

    bool OnCanSetCookie(const net::URLRequest*, const std::string&, net::CookieOptions*) override { return true; }

    bool AllowSetCookie(int, int, int, const GURL&, const GURL&, const std::string&, net::CookieOptions*) override { return true; }
};

static base::LazyInstance<MockWebRefiner> g_mock_web_refiner = LAZY_INSTANCE_INITIALIZER;
static WebRefiner* g_registered_web_refiner = 0;

WebRefiner* WebRefiner::Get() {
    if (!g_registered_web_refiner)
        return g_mock_web_refiner.Pointer();

    return g_registered_web_refiner;
}

void WebRefiner::Register(WebRefiner* instance) {
    g_registered_web_refiner = instance;
}

}

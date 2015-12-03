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

#ifndef WEB_REFINER_CONTENT_PUBLIC_COMMON_H_
#define WEB_REFINER_CONTENT_PUBLIC_COMMON_H_

#include "base/supports_user_data.h"
#include "content/public/common/resource_type.h"
#include "content/public/common/web_refiner_export.h"

namespace net {
class URLRequest;
}

namespace content {

class WEB_REFINER_EXPORT URLRequestID : public base::SupportsUserData::Data {
  public:
    // Returns the URLRequestID associated with the given URLRequest.
    static URLRequestID* ForRequest(const net::URLRequest* request);
    URLRequestID();
    URLRequestID(int render_process_id, int route_id, int render_frame_id, int parent_render_frame_id, content::ResourceType resource_type);
    URLRequestID& operator=(const URLRequestID& that);
    void AssociateWithRequest(net::URLRequest* request);
    int GetRenderProcessID() const;
    int GetRouteID() const;
    int GetRenderFrameID() const;
    int GetParentRenderFrameID() const;
    content::ResourceType GetResourceType() const;

  private:
    int render_process_id_;
    int route_id_;
    int render_frame_id_;
    int parent_render_frame_id_;
    content::ResourceType resource_type_;

    static const void* kUserDataKey;
};

} // namespace content

#endif // WEB_REFINER_CONTENT_PUBLIC_COMMON_H_

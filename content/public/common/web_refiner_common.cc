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

#include "web_refiner_common.h"

#include "content/public/common/child_process_host.h"
#include "net/url_request/url_request.h"

namespace content {

///////////////////////////////////////////////////////////////////////////////
// URLRequestID

// static
const void* URLRequestID::kUserDataKey = static_cast<const void*>(&URLRequestID::kUserDataKey);

URLRequestID* URLRequestID::ForRequest(const net::URLRequest* request) {
  return static_cast<URLRequestID*>(request->GetUserData(URLRequestID::kUserDataKey));
}

URLRequestID::URLRequestID()
  : render_process_id_(ChildProcessHost::kInvalidUniqueID)
  , route_id_(MSG_ROUTING_NONE)
  , render_frame_id_(-1)
  , parent_render_frame_id_(-1)
  , resource_type_(ResourceType::RESOURCE_TYPE_MAIN_FRAME) { }

URLRequestID::URLRequestID(int render_process_id, int route_id, int render_frame_id, int parent_render_frame_id, content::ResourceType resource_type)
  : render_process_id_(render_process_id)
  , route_id_(route_id)
  , render_frame_id_(render_frame_id)
  , parent_render_frame_id_(parent_render_frame_id)
  , resource_type_(resource_type) {
}

URLRequestID& URLRequestID::operator=(const URLRequestID& that) {
    render_process_id_ = that.render_process_id_;
    route_id_ = that.route_id_;
    render_frame_id_ = that.render_frame_id_;
    parent_render_frame_id_ = that.parent_render_frame_id_;
    resource_type_ = that.resource_type_;
    return *this;
}

void URLRequestID::AssociateWithRequest(net::URLRequest* request) {
  if (request)
    request->SetUserData(URLRequestID::kUserDataKey, this);
}

int URLRequestID::GetRenderProcessID() const {
  return render_process_id_;
}

int URLRequestID::GetRouteID() const {
  return route_id_;
}

int URLRequestID::GetRenderFrameID() const {
  return render_frame_id_;
}

int URLRequestID::GetParentRenderFrameID() const {
  return parent_render_frame_id_;
}

content::ResourceType URLRequestID::GetResourceType() const {
  return resource_type_;
}

}

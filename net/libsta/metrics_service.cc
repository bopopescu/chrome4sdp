/*
Copyright (c) 2015, The Linux Foundation. All rights reserved.

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
*/

#include "net/libsta/common/interfaces/metrics_service.h"
#include "net/libsta/common/interfaces/metrics_provider.h"
#include "net/libsta/common/interfaces/transport_layer_metrics.h"

#include "base/logging.h"

#define LIBNETXT_API_BY_PTR
#include "net/libnetxt/plugin_api_ptr.h"

using namespace sta;
using namespace net;

// The singelton object
MetricsService* MetricsService::this_ = NULL;

MetricsService::MetricsService():
   provider_(NULL) {
}

MetricsService::~MetricsService() {
}

MetricsService* MetricsService::getInstance(){
   if(this_ == NULL){ // safe because all actions are single threaded
      this_ = new MetricsService();
   }
   return this_;
}

void MetricsService::RegisterMetricsProvider(MetricsProvider* provider) {
   DCHECK(provider);
   LIBNETXT_LOGI("Register MetricsProvider");
   provider_ = provider;
}

void MetricsService::GetTransportLayerMetrics(TransportLayerMetrics* metrics) {
   DCHECK(metrics);
   LIBNETXT_LOGI("%s", __FUNCTION__);

   if (provider_ != NULL) {
      LIBNETXT_LOGI("get enhanced metrics from STA");
      // MetricsProvider is registered, so we query the AccelerationService
      // for the metrics
      provider_->GetMetrics(metrics);

      return;
   }

   // No AccelerationService is registered, return unmodified metrics object
   LIBNETXT_LOGI("get basic metrics");
}

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

#ifndef STA_METRICS_SERVICE_H_
#define STA_METRICS_SERVICE_H_

#include <stdint.h>
#include "base/basictypes.h"
#include "net/libsta/common/interfaces/transport_layer_metrics.h"

namespace sta
{
class MetricsProvider;

class MetricsService
{
public:

    MetricsService();

    virtual ~MetricsService();

    // Return singleton instance for MetricsService, create one if not present
    static MetricsService* getInstance();

    // Method to get transport layer metrics. If STA instance does NOT exist,
    // simply return empty metrics object. If STA is available, the metrics
    // query is delegated to STA internal modules and metrics object is
    // populate when this function returns
    //
    // @param[In/Out] metrics: the TransportLayerMetrics instance to store
    //                         all the metrics. The lifetime of metrics is
    //                         managed by the caller.
    void GetTransportLayerMetrics(TransportLayerMetrics* metrics);

    // Method to register the MetricsProvider to MetricsService.
    // This is called when STA is instantiated and STA instance registers
    // itself to the MetricsService instance
    void RegisterMetricsProvider(MetricsProvider* provider);

protected:

    MetricsProvider* provider_;

private:
    static MetricsService* this_;

    DISALLOW_COPY_AND_ASSIGN(MetricsService);
};

} //namespace sta

#endif //STA_METRICS_SERVICE_H_

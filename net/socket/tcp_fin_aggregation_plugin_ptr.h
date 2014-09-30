/**
 * Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other *materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.

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
 **/

#ifndef TCP_FIN_AGGREGATION_PLUGIN_PTR_H_
#define TCP_FIN_AGGREGATION_PLUGIN_PTR_H_

#include "net/socket/tcp_fin_aggregation_bridge.h"

class LibnetxtPluginApi;

class TcpFinAggLibnetxtPluginApi {
public:
    LIBNETXT_API_PTR_DEF_1(TCPFin, DecrementIdleCount, void, net::internal::ClientSocketPoolBaseHelper*)
    LIBNETXT_API_PTR_DEF_2(TCPFin, RemoveGroup, void, net::internal::ClientSocketPoolBaseHelper*, const std::string&)
    LIBNETXT_API_PTR_DEF_3(TCPFin, ShouldCleanup, bool, net::internal::IdleSocket*, base::TimeTicks, base::TimeDelta)
    LIBNETXT_API_PTR_DEF_0(TCPFin, GetCurrentTime, base::TimeTicks)
    LIBNETXT_API_PTR_DEF_1(TCPFin, GetGroupMap, net::internal::ClientSocketPoolBaseHelper::GroupMap, net::internal::ClientSocketPoolBaseHelper*)
    LIBNETXT_API_PTR_DEF_1(TCPFin, IdleSocketCount, int , net::internal::ClientSocketPoolBaseHelper*)
};

void InitTcpFinAggLibnetxtPluginApi(LibnetxtPluginApi* plugin_api);

#endif /* TCP_FIN_AGGREGATION_PLUGIN_PTR_H_ */
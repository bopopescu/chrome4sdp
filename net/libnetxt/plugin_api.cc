/*
* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
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

#include "base/compiler_specific.h"
#include "build/build_config.h"

#include <unistd.h>
#include <string>
#include <set>
#include <stdio.h>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/process/process_metrics.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time.h"
#include "net/base/address_list.h"
#include "net/base/completion_callback.h"
#include "net/base/host_port_pair.h"
#include "net/base/request_priority.h"
#include "net/base/load_timing_info.h"
#include "net/stat_hub/stat_hub_api.h"
#include "net/dns/host_resolver.h"
#include "net/http/http_byte_range.h"
#include "net/http/http_cache_transaction.h"
#include "net/http/http_network_transaction.h"
#include "net/http/http_request_info.h"
#include "net/http/http_request_headers.h"
#include "net/http/http_response_info.h"
#include "net/http/http_response_headers.h"
#include "net/http/preconnect.h"
#include "net/socket/client_socket_pool_manager.h"
#include "url/gurl.h"

#include "net/libnetxt/plugin_api.h"

// ================================ net::HttpRequestHeaders  ====================================
LIBNETXT_API_CPP_FORWARDER_CON_0(LibNetXt, net, HttpRequestHeaders)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, net, HttpRequestHeaders)
LIBNETXT_API_CPP_FORWARDER_1V(LibNetXt, net, HttpRequestHeaders, AddHeadersFromString, void, const base::StringPiece&)
LIBNETXT_API_CPP_FORWARDER_1V(LibNetXt, net, HttpRequestHeaders, AddHeaderFromString, void, const base::StringPiece&)
LIBNETXT_API_CPP_FORWARDER_2(LibNetXt, net, HttpRequestHeaders, GetHeader, bool, const std::string&, std::string*)
LIBNETXT_API_CPP_FORWARDER_2V(LibNetXt, net, HttpRequestHeaders, SetHeader, void, const std::string&, std::string&)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpRequestHeaders, ToString, std::string)

// ================================ net::HttpRequestInfo  ====================================
LIBNETXT_API_CPP_FORWARDER_CON_0(LibNetXt, net, HttpRequestInfo)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, net, HttpRequestInfo)

// ================================ net::HttpResponseInfo  ====================================
LIBNETXT_API_CPP_FORWARDER_CON_0(LibNetXt, net, HttpResponseInfo)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, net, HttpResponseInfo)
LIBNETXT_API_CPP_FORWARDER_COP_CON_1(LibNetXt, net, HttpResponseInfo, const net::HttpResponseInfo&)

// ================================ net::HttpResponseHeaders  ====================================
LIBNETXT_API_CPP_FORWARDER_CON_1(LibNetXt, net, HttpResponseHeaders, std::string)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpResponseHeaders, GetContentLength, int64)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpResponseHeaders, GetHttpVersion, net::HttpVersion)
LIBNETXT_API_CPP_FORWARDER_1(LibNetXt, net, HttpResponseHeaders, IsRedirect, bool, std::string*)
LIBNETXT_API_CPP_FORWARDER_1(LibNetXt, net, HttpResponseHeaders, ReplaceStatusLine, void, std::string const&)
LIBNETXT_API_CPP_FORWARDER_1(LibNetXt, net, HttpResponseHeaders, RemoveHeader, void, std::string const&)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpResponseHeaders, IsChunkEncoded, bool)
LIBNETXT_API_CPP_FORWARDER_2(LibNetXt, net, HttpResponseHeaders, HasHeaderValue,bool,const base::StringPiece&,const base::StringPiece&)
LIBNETXT_API_CPP_FORWARDER_1(LibNetXt, net, HttpResponseHeaders, HasHeader, bool, base::BasicStringPiece<std::string> const&)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpResponseHeaders, GetStatusLine, std::string)
LIBNETXT_API_CPP_FORWARDER_3(LibNetXt, net, HttpResponseHeaders, GetContentRange,bool,int64*,int64*,int64*)
LIBNETXT_API_CPP_FORWARDER_1(LibNetXt, net, HttpResponseHeaders, AddHeader, void, std::string)
LIBNETXT_API_CPP_FORWARDER_3(LibNetXt, net, HttpResponseHeaders, EnumerateHeader ,bool, void**, const std::string&, std::string*)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpResponseHeaders, response_code, int)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpResponseHeaders, raw_headers, const std::string&)
LIBNETXT_API_CPP_FORWARDER_3(LibNetXt, net, HttpResponseHeaders, EnumerateHeaderLines, bool , void**, std::string*, std::string*)

// ================================ net::HttpByteRange =================================
LIBNETXT_API_CPP_FORWARDER_CON_0(LibNetXt, net, HttpByteRange)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, net, HttpByteRange)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpByteRange, IsSuffixByteRange, bool)

// ================================ net::HttpNetworkTransaction =================================
LIBNETXT_API_CPP_FORWARDER_CON_2(LibNetXt, net, HttpNetworkTransaction, net::RequestPriority, net::HttpNetworkSession* )
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, net, HttpNetworkTransaction)

// ================================ net::LoadTimingInfo =================================
LIBNETXT_API_CPP_FORWARDER_CON_0(LibNetXt, net, LoadTimingInfo)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, net, LoadTimingInfo)

// ================================ net::HttpCache ====================================
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HttpCache, GetSession, net::HttpNetworkSession*)

// ================================ ::GURL  ====================================
LIBNETXT_API_CPP_FORWARDER_CON_1(LibNetXt, , GURL, std::string&)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, , GURL)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, , GURL, GetOrigin, GURL)
LIBNETXT_API_CPP_FORWARDER_1(LibNetXt, , GURL, Resolve, GURL, const std::string&)
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, , GURL, ExtractFileName, std::string )

// ================================ net::HostPortPair ====================================
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, net, HostPortPair, ToString, std::string)

// ================================ base::IOBufferWithSize ====================================
LIBNETXT_API_CPP_FORWARDER_CON_1(LibNetXt,net, IOBufferWithSize , int)

// ================================ base::SystemMemoryInfoKB ====================================
LIBNETXT_API_CPP_FORWARDER_CON_0(LibNetXt, base, SystemMemoryInfoKB)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt, base, SystemMemoryInfoKB)

// ================================ logging::LogMessage ====================================
LIBNETXT_API_CPP_FORWARDER_CON_3(LibNetXt,logging, LogMessage, const char*,int, logging::LogSeverity)
LIBNETXT_API_CPP_FORWARDER_DES(LibNetXt,logging, LogMessage)

// ================================ base::Time =============================================
LIBNETXT_API_CPP_FORWARDER_0(LibNetXt, base, Time, ToDoubleT, double)

// ======================================= Common ==============================================
std::string LIBNETXT_API(EscapeForHTML)(const std::string& text){
    return net::EscapeForHTML(text);
}

bool LIBNETXT_API(GetSystemMemoryInfo)(base::SystemMemoryInfoKB* meminfo){
    return base::GetSystemMemoryInfo(meminfo);
}

base::Time LIBNETXT_API(GetSystemTime)() {
    return base::Time::NowFromSystemTime();
}

int LIBNETXT_API(GetTimeDeltaInMs)(const base::Time& start_time, const base::Time& finish_time) {
    base::TimeDelta delta = finish_time - start_time;
    return (int)delta.InMilliseconds(); //int64
}

//Return host (without scheme) from a url
const char* LIBNETXT_API(GetHostFromUrl)(const std::string& url, std::string& host) {
    GURL dest(url);
    host = dest.HostNoBrackets();
    return host.c_str();
}

//Return scheme, host, and port from a url
const char* LIBNETXT_API(GetHostPortFromUrl)(const std::string& url, std::string& host_port) {
    GURL dest(url);
    host_port = dest.GetOrigin().spec();
    return host_port.c_str();
}

int LIBNETXT_API(GetMaxSocketsPerGroup)() {
    return net::ClientSocketPoolManager::max_sockets_per_group(net::HttpNetworkSession::NORMAL_SOCKET_POOL);
}

int LIBNETXT_API(SysPropertyGet)(const char *key, char *value, const char *default_value) {
    return LIBNETXT_PROPERTY_GET(key, value, default_value);
}

int LIBNETXT_API(DebugLog)(const char* str) {
    LIBNETXT_LOGD("%s", str);
    return strlen(str);
}

bool LIBNETXT_API(IsVerboseEnabled)() {
    return LIBNETXT_IS_VERBOSE;
}

LibnetxtVerboseLevel LIBNETXT_API(GetVerboseLevel)() {
    return LIBNETXT_GET_VERBOSE_LEVEL;
}

bool LIBNETXT_API(ParseHostAndPort)(const GURL& url, std::string *host, int* port){
    std::string host_and_port;
    net::HostPortPair host_port_pair =  net::HostPortPair::FromURL(url);
    return net::ParseHostAndPort(host_port_pair.ToString(), host,port);
}

bool LIBNETXT_API(ParseRangeHeader)(const std::string& hdr, std::vector<net::HttpByteRange>* ranges){
    return net::HttpUtil::ParseRangeHeader(hdr, ranges);
}

// Look for Range request header
//return value - false if we the req-range is not in the "bytes: 199-500" form
bool LIBNETXT_API(GetRequestRange)(const net::HttpRequestHeaders& req_headers, int64& req_range_start, int64& req_range_end){
    std::string range_header;
    net::HttpByteRange byte_range;

    if (!req_headers.GetHeader(net::HttpRequestHeaders::kRange, &range_header))
        return false;
    std::vector<net::HttpByteRange> ranges;
    if (!net::HttpUtil::ParseRangeHeader(range_header, &ranges))
        return false;
    if (!(ranges.size() == 1))
        return false;
    byte_range = ranges[0];
    req_range_start = byte_range.first_byte_position();
    req_range_end = byte_range.last_byte_position();

    if (byte_range.IsSuffixByteRange()) { // Example: "bytes:-500"
        req_range_start = 0;
        req_range_end = byte_range.suffix_length();
    } else if (req_range_end == -1)
        return false;
    return true;  // Example: "bytes: 199-500"
}

bool LIBNETXT_API(PathExists)( const std::string& p){
    base::FilePath path(p);
    base::ThreadRestrictions::ScopedAllowIO allowIO; // see important comment in thread_restrictions.h
    return base::PathExists(path);
}

void onResolveDone(int result) {
}

void DoDnsResolve (std::string* url) {
    //we allocate this only once and hence do need to delete. This might cause false memory leak alert.
    static net::AddressList* adList=new net::AddressList;
    net::HostPortPair hpPair;
    hpPair.set_host(url[0]);
    hpPair.set_port(80);
    net::HostResolver::RequestInfo reqInfo(hpPair);
    net::BoundNetLog nl;
    net::HttpCache* cache = StatHubGetHttpCache();
    if (cache) {
        net::HttpNetworkSession* session = cache->GetSession();
        if (session) {
            net::HostResolver* hr = session->params().host_resolver;
            if(hr) {
                hr->Resolve(reqInfo, net::RequestPriority::DEFAULT_PRIORITY, adList, base::Bind(&onResolveDone), 0, nl);
            }
        }
    }
    delete url;
}

void LIBNETXT_API(DnsResolve)(const char* url) {
    base::MessageLoop* message_loop = StatHubGetIoMessageLoop();
    if (message_loop && url) {
        message_loop->PostTask(FROM_HERE, base::Bind(&DoDnsResolve, new std::string(url)));
    }
}

base::TimeTicks LIBNETXT_API(GetTimeTicksNow)(){
    return base::TimeTicks::Now();
}

void  LIBNETXT_API(GetUrlOriginSpec)(const GURL& url, std::string& str){
    str = url.GetOrigin().spec();
}

scoped_refptr<net::HttpResponseHeaders>* LibNetXtscoped_refptr_netHttpResponseHeadersconstructor(){
    return new scoped_refptr<net::HttpResponseHeaders>;
}

void LibNetXtscoped_refptr_netHttpResponseHeadersdestructor(scoped_refptr<net::HttpResponseHeaders>* p ){
    delete p;
}

// assign to the scoped_refptr object
void LibNetXtAssignHttpResponseHeaders(scoped_refptr<net::HttpResponseHeaders>* p, net::HttpResponseHeaders const* src){
    *p = const_cast<net::HttpResponseHeaders*>(src);
}

void LIBNETXT_API(PostTask)(const base::Closure& task){
    base::MessageLoop::current()->PostTask(FROM_HERE, task);
}

std::string LibNetXtConvertHeadersBackToHTTPResponse(const std::string& a){
    return net::HttpUtil::ConvertHeadersBackToHTTPResponse(a);
}

void LIBNETXT_API(NetPreconnect)(net::HttpNetworkSession* session, GURL const& url, int numOfConnections) {
    net::Preconnect::DoPreconnect(session, url, numOfConnections);
}

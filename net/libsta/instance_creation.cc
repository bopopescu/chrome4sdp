/*
Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.

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

#include "base/command_line.h"

#include "instance_creation.h"
#include "net/libnetxt/dyn_lib_loader.h"
#include "net/libsta/common/interfaces/transport_service.h"
#include "net/libsta/common/interfaces/metrics_service.h"
#include "net/http/http_transaction.h"
#include "net/libnetxt/plugin_api_ptr.h"
#include "net/http/http_network_transaction.h"
#include "net/libsta/common/interfaces/metrics_provider.h"

#include "net/stat_hub/stat_hub.h"
#include "net/stat_hub/sta_proc.h"

static bool gTriedToLoadLib = false;
static bool gStaLibraryLoaded = false;
bool gMagicTurnOff = false; //  set to true while running unittests

namespace sta {
class AccelerationService;
}

typedef int (*ConnectAcceleratorToTransport_FunPtr )(sta::AccelerationService* a, sta::TransportService* t);

static ConnectAcceleratorToTransport_FunPtr ConnectAcceleratorToTransport = NULL;

typedef int (*ConnectProviderToMetrics_FunPtr )(sta::AccelerationService* a, sta::MetricsService* m);
static ConnectProviderToMetrics_FunPtr ConnectProviderToMetrics = NULL;

typedef bool (*DoObserveResourceFetchDone_FunPtr )(std::string, std::string, bool&);
static DoObserveResourceFetchDone_FunPtr DoObserveResourceFetchDone = NULL;

typedef void (*STAStatus_FunPtr )(bool&, bool&);
static STAStatus_FunPtr STAStatus = NULL;

//The methods in this file, and in the loading library/instantiation path is not thread-safe,
//and was written under the assumption that only one thread can create HttpNetworkSession objects.

int LoadStaLibrary() {

    gTriedToLoadLib = true;
    sta::AccelerationService * pAccelerator = GetInterfacePtr<sta::AccelerationService>(LibraryManager::
            GetFunctionPtr<sta::AccelerationService>("libsta","GetAccelerationServiceObject"));
    if (pAccelerator == NULL) {
        LIBNETXT_LOGI("STA library not loaded");
        return -1;
    }

    sta::TransportService* pService = sta::TransportService::getInstance();
    DCHECK(pService);

    // Get MetricsService instance
    sta::MetricsService* mService = sta::MetricsService::getInstance();
    DCHECK(mService);

    void* fh = LibraryManager::GetLibraryHandle("libsta");
    if (fh) {
      *(void **)(&ConnectAcceleratorToTransport) = LibraryManager::GetLibrarySymbol(fh, "ConnectAcceleratorToTransport", false);
      if (ConnectAcceleratorToTransport == NULL) {
         LIBNETXT_LOGE("symbol ConnectAcceleratorToTransport not found in library");
         return -2;
      }

      *(void **)(&ConnectProviderToMetrics) = LibraryManager::GetLibrarySymbol(fh, "ConnectProviderToMetrics", false);
      if (ConnectProviderToMetrics == NULL) {
         LIBNETXT_LOGE("symbol ConnectProviderToMetrics not found in library");
         return -2;
      }

      *(void **)(&DoObserveResourceFetchDone) = LibraryManager::GetLibrarySymbol(fh, "DoObserveResourceFetchDone", false);
      if (DoObserveResourceFetchDone == NULL) {
          LIBNETXT_LOGE("symbol DoObserveResourceFetchDone not found in library");
          return -2;
      }

      *(void **)(&STAStatus) = LibraryManager::GetLibrarySymbol(fh, "STAStatus", false);
      if (STAStatus == NULL) {
          LIBNETXT_LOGE("symbol STAStatus not found in library");
          return -2;
      }

    } else {
        LIBNETXT_LOGE("symbol libsta not found in library");
        return -2;
    }

    // Register the STA instance to MetricsService
    ConnectProviderToMetrics(pAccelerator, mService);

    int res = ConnectAcceleratorToTransport(pAccelerator,pService);
    if (res) return res;

    stat_hub::StatHub::GetInstance()->LoadProc(sta_proc::WhitelistManager::GetInstance());
    return res;
}

void GetSTAStatus(bool& is_ready_for_fetch, bool& is_sta_ready) {
    if (STAStatus) {
        STAStatus(is_ready_for_fetch, is_sta_ready);
    }
}

bool ObserveResourceFetchDone(std::string path, std::string module, bool& is_whitelist_ready ) {
    if (DoObserveResourceFetchDone) {
        return DoObserveResourceFetchDone(path, module, is_whitelist_ready);
    }
    return false;
}

void OnSessionCreation(const net::HttpNetworkSession::Params& params){
    if(gTriedToLoadLib || gMagicTurnOff || params.is_cloned){
        if(LIBNETXT_IS_VERBOSE) {
            LIBNETXT_LOGI("OnSessionCreation - skip library load");
        }
        return;
    }

    base::CommandLine* command_line = (base::CommandLine::InitializedForCurrentProcess() ? base::CommandLine::ForCurrentProcess() : 0);

        gStaLibraryLoaded = (LoadStaLibrary() == 0);

        if(gStaLibraryLoaded){
            // note: to control the level, use the built in commandline --log-level
            if (command_line && command_line->HasSwitch("terse-logging")){
                 logging::SetLogItems(false, false, false, false);
            }
        }
}


int CreateInstance_TATransaction(net::HttpTransaction ** ppInteface, net::RequestPriority pr, net::HttpNetworkSession* pSession, std::string* err) {

    bool is_sta_ready = sta_proc::WhitelistManager::GetInstance()->IsSTAReady();
    if(!gStaLibraryLoaded || !is_sta_ready){
        static bool reported = false;
        if(!reported){
            if (!gStaLibraryLoaded)
                LIBNETXT_LOGI("STA library not loaded (message will not repeat)");
            else if (!is_sta_ready)
                LIBNETXT_LOGI("STA is not ready yet (message will not repeat)");
            reported = true;
        }
        *ppInteface = new net::HttpNetworkTransaction(pr, pSession);
        return -1;
    }

    static StaTransactionFactory* pFactory = GET_DYNAMIC_OBJECT_INTERFACE_PTR("libsta", StaTransactionFactory);
    if(!pFactory){
            LIBNETXT_LOGE("STA object not found in library. Check library version mismatch");
            *ppInteface = new net::HttpNetworkTransaction(pr, pSession);
            return -2;
        }

    *ppInteface = pFactory->CreateTransaction(pr, pSession);

    if (!*ppInteface) {
        LIBNETXT_LOGE("STA failed to create TransactionProcessor");
        *ppInteface = new net::HttpNetworkTransaction(pr, pSession);
        return -3;
    }

    return 0;
}

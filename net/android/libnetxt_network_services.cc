/*
* Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.
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
#include <set>
#include "net/http/http_network_session.h"
#include "net/socket/client_socket_pool_manager.h"
#include "net/android/libnetxt_network_services.h"
#include "jni/NetworkServices_jni.h"
#include "base/logging.h"
#include "net/libnetxt/dyn_lib_loader.h"
#include "net/libnetxt/libnetxt_base.h"
#include "net/stat_hub/stat_hub_cmd_api.h"
#include "net/stat_hub/stat_hub.h"

namespace net{
void DoCloseIdleConnections() {
  // This function must be called in the IO thread

  // create a COPY of the set we are given since there is no guarantie that this call is from the same thread
  std::set<HttpNetworkSession*> sessions  =  std::set<HttpNetworkSession*>(net::HttpNetworkSession::getNetworkSessions());

  std::set<HttpNetworkSession*>::const_iterator it = sessions.begin();
  // iterate on all sessions
  for(;it != sessions.end(); it++){
    net::HttpNetworkSession* pSession = *it;
    ClientSocketPoolManager* poolManager = pSession->GetSocketPoolManager(net::HttpNetworkSession::NORMAL_SOCKET_POOL);
    if(poolManager != NULL)
      poolManager->CloseIdleSockets();

    poolManager = pSession->GetSocketPoolManager(net::HttpNetworkSession::NORMAL_SOCKET_STA_POOL);
    if(poolManager != NULL)
      poolManager->CloseIdleSockets();

    poolManager = pSession->GetSocketPoolManager(net::HttpNetworkSession::WEBSOCKET_SOCKET_POOL);
    if(poolManager != NULL)
      poolManager->CloseIdleSockets();
  } // all sessions
}

void HintUpcomingUserActivity(JNIEnv* env, jclass clazz) {
  static void (*DoPresumeUserLoadEvent)() = NULL;
  static bool initialized = false;

  if (!initialized) {
    //check if the application context is ready. If not, we should return and let the client retry later
    if ( base::android::GetApplicationContext() == NULL ) {
      if (LIBNETXT_IS_VERBOSE) {
        LIBNETXT_LOGD("base::android::GetApplicationContext() is NULL - skipping qmodem call");
      }
      return;
    }
    initialized = true;
    void* fh = LibraryManager::GetLibraryHandle("libqmodem_plugin");
    if (fh) {
      *(void **)(&DoPresumeUserLoadEvent) = LibraryManager::GetLibrarySymbol(fh, "presumeUserLoadEvent", false);
    }
    if (!DoPresumeUserLoadEvent) {
      LIBNETXT_LOGE("Unable to load qmodem_plugin");
      return;
    }
  }

  if (DoPresumeUserLoadEvent) {
    DoPresumeUserLoadEvent();
  }

}

void NotifyResourceFetcherDone(JNIEnv* env, jclass clazz, jstring path, jstring module) {
  StatHubCmd* cmd = STAT_HUB_API(CmdCreate)(SH_CMD_JAVA_GP_EVENT, SH_ACTION_RESOURCE_FETCH_DONE, 0);
  if (NULL!=cmd) {
    const char* temp;
    temp = env->GetStringUTFChars(path,0);
    cmd->AddParamAsString(std::string(temp).c_str());
    env->ReleaseStringUTFChars(path,temp);
    temp = env->GetStringUTFChars(module,0);
    cmd->AddParamAsString(std::string(temp).c_str());
    env->ReleaseStringUTFChars(module,temp);
    cmd->AddParamAsBool(true);
    STAT_HUB_API(CmdCommit)(cmd);
  }
}

void OnCloseIdleConnections(JNIEnv* env, jclass clazz) {
  // post a message to the network thread event loop, to call DoCloseIdleConnections()
  base::Closure task = base::Bind(&DoCloseIdleConnections);

  // We cannot use content::BrowserThread since in some targets it causes link failure
  base::MessageLoop* message_loop = stat_hub::StatHub::GetInstance()->GetIoMessageLoop();
  if (message_loop) {
    message_loop->PostTask(FROM_HERE, task);
  } else {
    LOG(INFO) << "OnCloseIdleConnections: message loop is null, ignoring.";
  }

}

bool RegisterLibnetxtNetworkServices(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

} //net

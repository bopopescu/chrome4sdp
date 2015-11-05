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
#include "base/compiler_specific.h"
#include "build/build_config.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <unistd.h>
#include <sys/time.h>

#include "base/time/time.h"

#include "sta_proc.h"

#include "net/stat_hub/stat_hub_cmd.h"
#include "net/stat_hub/stat_hub_api.h"
#include "net/stat_hub/stat_hub_cmd_api.h"

#include "net/libsta/instance_creation.h"

// Version numbers.
#define CURRENT_VERSION  "1.0.0"
#define STA_MODULE_NAME "STA"

//=========================================================================
namespace sta_proc {

static const char* kProcName = "sta";
WhitelistManager* WhitelistManager::whitelist_manager_ = NULL;

//=========================================================================
//                  Processor Implementation
//=========================================================================

WhitelistManager::WhitelistManager():is_ready_for_fetch_(true), is_sta_ready_(false) {
}

WhitelistManager* WhitelistManager::GetInstance() {
    if (!whitelist_manager_) {
        whitelist_manager_ = new WhitelistManager();
    }
    return whitelist_manager_;
}
// ========================================================================
bool WhitelistManager::OnGetProcInfo(std::string& name, std::string& version) {
    name = kProcName;
    version = CURRENT_VERSION;
    return true;
}

// ========================================================================
bool WhitelistManager::OnInit(sql::Connection* db) {
    GetSTAStatus(is_ready_for_fetch_, is_sta_ready_);
    if (LIBNETXT_IS_VERBOSE) {
        LIBNETXT_LOGI("STA WhitelistManager - is_ready_for_fetch_ = %d, is_sta_ready_ = %d", is_ready_for_fetch_, is_sta_ready_);
    }
    return true;
}

bool WhitelistManager::IsSTAReady() {
    return is_sta_ready_;
}

// ========================================================================
STAT_HUB_CMD_HANDLER_CONTAINER_IMPL(WhitelistManager, WhitelistManager_CmdHandlerResFetchDone) {
    switch(cmd->GetAction()) {
        case SH_ACTION_RESOURCE_FETCH_DONE:
        {
            const char* path = cmd->GetParamAsString(0);
            const char* module = cmd->GetParamAsString(1);

            return ObserveResourceFetchDone(std::string(path), std::string(module), is_sta_ready_);
        }
        default:
            return false;
    }
}

// ========================================================================
bool WhitelistManager::OnGetCmdMask(uint32& cmd_mask) {
    cmd_mask = 0;
    if (is_ready_for_fetch_)
        STAT_HUB_CMD_HANDLER_CONTAINER_INIT(cmd_handlers_, cmd_mask, SH_CMD_JAVA_GP_EVENT, WhitelistManager_CmdHandlerResFetchDone,
                SH_ACTION_RESOURCE_FETCH_DONE);
    return true;
}

} //namespace sta_proc

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
#ifndef NET_STA_PROC_H_
#define NET_STA_PROC_H_
#pragma once

#include "base/compiler_specific.h"
#include "build/build_config.h"
#include "net/stat_hub/stat_hub.h"

namespace sta_proc {

class WhitelistManager : public stat_hub::StatProcessor {
public:
    WhitelistManager();
    virtual ~WhitelistManager() {};
    static WhitelistManager* GetInstance();

    virtual bool OnInit(sql::Connection* db);
    virtual bool OnGetProcInfo(std::string& name, std::string& version);
    virtual bool OnGetCmdMask(unsigned int& cmd_mask);
    bool IsSTAReady();

    bool WhitelistManager_CmdHandlerResFetchDone(StatHubCmd* cmd);
private:
    static WhitelistManager* whitelist_manager_;
    bool is_ready_for_fetch_;
    bool is_sta_ready_;
};

} //namespace sta_proc

#endif  // NET_STA_PROC_H_

 /*
  * Copyright (c) 2016, The Linux Foundation. All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions are
  * met:
  *  * Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  *  * Redistributions in binary form must reproduce the above
  *    copyright notice, this list of conditions and the following
  *    disclaimer in the documentation and/or other materials provided
  *    with the distribution.
  *  * Neither the name of The Linux Foundation nor the names of its
  *    contributors may be used to endorse or promote products derived
  *    from this software without specific prior written permission.
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

#ifndef BT_AUDIO_MANAGER_APP_H
#define BT_AUDIO_MANAGER_APP_H

#include <map>
#include <string>
#include <pthread.h>

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "ipc.h"

#if (defined(BT_AUDIO_HAL_INTEGRATION))
#include "qahw_api.h"
#include "qahw_defs.h"
#endif

#define MAX_PROFILE_ENTRIES 2

typedef struct {
    ProfileIdType profile_id;
    ControlRequestType control_status;
} ControlStackEntry;

class BT_Audio_Manager {

  private:
    config_t *config;
    ControlStackEntry audio_control_stack[MAX_PROFILE_ENTRIES];

  public:
    BT_Audio_Manager(const bt_interface_t *bt_interface, config_t *config);
    ~BT_Audio_Manager();
    void ProcessEvent(BtEvent* pEvent);
    void HandleEnableBTAM();
    void HandleDisableBTAM();
    int GetTopIndex();
    ThreadIdType GetThreadId(ProfileIdType profile_id);
    void AddNewNode(ProfileIdType profile, ControlRequestType ctrlStatus);
    void RemoveNode(int index);
    int GetIndex(ProfileIdType profile_id);
    void SendControlStatusMessage(ControlStatusType ctrlStatus, ProfileIdType profile_id);
    char* dump_message(BluetoothEventId event_id);
    void LoadAudioHal();
    void UnloadAudioHal();
#if (defined BT_AUDIO_HAL_INTEGRATION)
    qahw_module_handle_t* GetAudioDevice();
    qahw_module_handle_t *qahw_mod_handle;
#endif
};

#endif

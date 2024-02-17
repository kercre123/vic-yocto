 /*
  * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
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

#include <list>
#include <map>
#include <string.h>

#include "Audio_Manager.hpp"
#include "A2dp_Sink_Streaming.hpp"

#define LOGTAG "BT_AM"

using namespace std;
using std::list;
using std::string;

BT_Audio_Manager *pBTAM = NULL;
extern A2dp_Sink_Streaming *pA2dpSinkStream;

#ifdef __cplusplus
extern "C" {
#endif

void BtAudioManagerHandler(void *msg) {
    BtEvent* pEvent = NULL;
    if(!msg) {
        printf("Msg is NULL, return.\n");
        return;
    }

    pEvent = ( BtEvent *) msg;
    switch(pEvent->event_id) {
        case PROFILE_API_START:
            ALOGD(LOGTAG " enable BT Audio Manager");
            if (pBTAM) {
                pBTAM->HandleEnableBTAM();
            }
            break;
        case PROFILE_API_STOP:
            ALOGD(LOGTAG " disable BT Audio Manager");
            if (pBTAM) {
                pBTAM->HandleDisableBTAM();
            }
            break;
        default:
            if(pBTAM) {
                pBTAM->ProcessEvent(( BtEvent *) msg);
            }
            break;
    }
    delete pEvent;
}

#ifdef __cplusplus
}
#endif

char* BT_Audio_Manager::dump_message(BluetoothEventId event_id) {
    switch(event_id) {
    case BT_AM_CONTROL_STATUS:
        return "AM_CONTROL_STATUS";
    case BT_AM_REQUEST_CONTROL:
        return "AM_REQUEST_CONTROL";
    case BT_AM_RELEASE_CONTROL:
        return "AM_RELEASE_CONTROL";
    }
    return "UNKNOWN";
}

void BT_Audio_Manager::HandleEnableBTAM(void) {
    BtEvent *pEvent = new BtEvent;
    pEvent->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
    pEvent->profile_start_event.profile_id = PROFILE_ID_BT_AM;
    pEvent->profile_start_event.status = true;
    PostMessage(THREAD_ID_GAP, pEvent);
    LoadAudioHal();
}

void BT_Audio_Manager::HandleDisableBTAM(void) {
    UnloadAudioHal();
    for(int i= 0 ; i < MAX_PROFILE_ENTRIES; i++) {
        audio_control_stack[i].profile_id =  PROFILE_ID_MAX;
        audio_control_stack[i].control_status = REQUEST_TYPE_DEFAULT;
    }
    BtEvent *pEvent = new BtEvent;
    pEvent->profile_stop_event.event_id = PROFILE_EVENT_STOP_DONE;
    pEvent->profile_stop_event.profile_id = PROFILE_ID_BT_AM;
    pEvent->profile_stop_event.status = true;
    PostMessage(THREAD_ID_GAP, pEvent);
}

int BT_Audio_Manager::GetTopIndex(void) {
    int top;
    for (top = MAX_PROFILE_ENTRIES -1 ; top >= 0; top--) {
        if (audio_control_stack[top].profile_id != PROFILE_ID_MAX) {
            break;
        }
    }
    if (top < 0) {
        ALOGD(LOGTAG " Empty, topmost index = %d", top);
        return top;
    }
    ALOGD(LOGTAG " topmost index =  %d, profile = %d  status_type = %d ", top,
           audio_control_stack[top].profile_id, audio_control_stack[top].control_status);
    return top;
}

void BT_Audio_Manager::LoadAudioHal()
{
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    ALOGD(LOGTAG " Load Audio HAL +");
    if (qahw_mod_handle != NULL) {
        ALOGD(" Audio HAL already loaded");
    } else {
        qahw_mod_handle = qahw_load_module(QAHW_MODULE_ID_PRIMARY);
    }
    if (qahw_mod_handle == NULL) {
        ALOGD("  qahw_load_module failed");
        return;
    }
    ALOGD(LOGTAG "Load Audio HAL -");
#endif
}
void BT_Audio_Manager::UnloadAudioHal()
{
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    int ret = 0;
    ALOGD(LOGTAG "UnLoad Audio HAL +");
    if(qahw_mod_handle != NULL)
        ret = qahw_unload_module(qahw_mod_handle);

    if (ret)
        ALOGE(LOGTAG "Unloading audio hal failed");

    qahw_mod_handle = NULL;
    ALOGD(LOGTAG "UnLoad Audio HAL -");
#endif
}
#if (defined(BT_AUDIO_HAL_INTEGRATION))
qahw_module_handle_t* BT_Audio_Manager::GetAudioDevice()
{
    if (qahw_mod_handle != NULL) {
        return qahw_mod_handle;
    }
    else {
        ALOGD(" audio hw module handle is NULL ");
        return NULL;
    }
}
#endif
ThreadIdType BT_Audio_Manager::GetThreadId(ProfileIdType profile_id) {
    ThreadIdType thread_id = THREAD_ID_MAX;
    switch(profile_id) {
    case PROFILE_ID_HFP_CLIENT:
        thread_id = THREAD_ID_HFP_CLIENT;
        break;
    case PROFILE_ID_A2DP_SINK:
        thread_id = THREAD_ID_A2DP_SINK;
        break;
    }
    return thread_id;
}
// adds a new controlEntry node on the top
void BT_Audio_Manager::AddNewNode(ProfileIdType profile, ControlRequestType ctrlStatus) {
    int index = GetTopIndex();
    if (index == (MAX_PROFILE_ENTRIES - 1))
        return;
    if (index < 0)
        index = 0;
    else
        index++;
    ALOGD(LOGTAG " AddNewNode @ index = %d, profile_id = %d ", index, profile);
    audio_control_stack[index].profile_id = profile;
    audio_control_stack[index].control_status = ctrlStatus;
}

//removes node from the mentioned index
void BT_Audio_Manager::RemoveNode(int index) {
    ALOGD(LOGTAG " RemoveNode index = %d", index);
    int i = index;
    for (i = index; i < (MAX_PROFILE_ENTRIES -1); i++) {
        audio_control_stack[i].profile_id = audio_control_stack[i+1].profile_id;
        audio_control_stack[i].control_status = audio_control_stack[i+1].control_status;
    }
    if (i == (MAX_PROFILE_ENTRIES -1)) {
        //max entry reached.
        audio_control_stack[MAX_PROFILE_ENTRIES - 1].profile_id = PROFILE_ID_MAX;
        audio_control_stack[MAX_PROFILE_ENTRIES - 1].control_status = REQUEST_TYPE_DEFAULT;
    }
}
/* get index for a particular profile entry
 * returns index if found, -1 otherwise
 */
int BT_Audio_Manager::GetIndex(ProfileIdType profile_id) {
    ALOGD(LOGTAG " GetIndex profile_id = %d", profile_id);
    int index = MAX_PROFILE_ENTRIES -1 ;
    for (index = MAX_PROFILE_ENTRIES - 1; index >= 0; index--) {
        if(audio_control_stack[index].profile_id != profile_id) {
            continue;
        }
        return index;
    }
    return index;
}
void BT_Audio_Manager::SendControlStatusMessage(ControlStatusType ctrlStatus,
        ProfileIdType profile_id) {
    BtEvent *pControlResponse = new BtEvent;
    pControlResponse->btamControlStatus.event_id = BT_AM_CONTROL_STATUS;
    pControlResponse->btamControlStatus.status_type = ctrlStatus;
    if (profile_id != PROFILE_ID_A2DP_SINK) {
        PostMessage(GetThreadId(profile_id), pControlResponse);
    }
    else {
        if (pA2dpSinkStream != NULL) {
            thread_post(pA2dpSinkStream->threadInfo.thread_id,
            pA2dpSinkStream->threadInfo.thread_handler, (void*)pControlResponse);
        }
    }
}
void BT_Audio_Manager::ProcessEvent(BtEvent* pEvent) {
    ALOGD(LOGTAG " Processing event %s", dump_message(pEvent->event_id));
    int top, profile_index;
    switch(pEvent->event_id) {
        case BT_AM_REQUEST_CONTROL:
            ALOGD(LOGTAG " CtrlReq profile_id  =  %d req_type = %d",
                    pEvent->btamControlReq.profile_id, pEvent->btamControlReq.request_type);
            // first get topmost empty index
            top = GetTopIndex();
            if (top < 0) {
                // fresh request, load Audio HAL TODO:BTAM Load Audio HAL
                AddNewNode(pEvent->btamControlReq.profile_id, pEvent->btamControlReq.request_type);
                if (pEvent->btamControlReq.request_type == REQUEST_TYPE_PERMANENT)
                    SendControlStatusMessage(STATUS_GAIN, pEvent->btamControlReq.profile_id);
                else if(pEvent->btamControlReq.request_type == REQUEST_TYPE_TRANSIENT)
                    SendControlStatusMessage(STATUS_GAIN_TRANSIENT, pEvent->btamControlReq.profile_id);
                break;
            }
            // there is already an entry, check for control_status of top element
            if (audio_control_stack[top].profile_id == pEvent->btamControlReq.profile_id) {
                // second request from same profile, this should not ideally happen
                ALOGD(LOGTAG " Request came from same profile");
                if (audio_control_stack[top].control_status == REQUEST_TYPE_TRANSIENT)
                    SendControlStatusMessage(STATUS_GAIN_TRANSIENT,
                                            pEvent->btamControlReq.profile_id);
                if (audio_control_stack[top].control_status == REQUEST_TYPE_PERMANENT)
                    SendControlStatusMessage(STATUS_GAIN, pEvent->btamControlReq.profile_id);
                break;
            }
            // request is from different profile
            switch(audio_control_stack[top].control_status) {
                case REQUEST_TYPE_TRANSIENT:
                    // request is from another profile, it should be denied
                    SendControlStatusMessage(STATUS_LOSS_TRANSIENT, pEvent->btamControlReq.profile_id);
                    break;
                case REQUEST_TYPE_PERMANENT:
                    if (pEvent->btamControlReq.request_type == REQUEST_TYPE_TRANSIENT) {
                        if(top == (MAX_PROFILE_ENTRIES - 1)) {
                           SendControlStatusMessage(STATUS_LOSS, pEvent->btamControlReq.profile_id);
                           break;
                        }
                        SendControlStatusMessage(STATUS_LOSS_TRANSIENT,
                                                      audio_control_stack[top].profile_id);
                        AddNewNode(pEvent->btamControlReq.profile_id,
                                                      pEvent->btamControlReq.request_type);
                        SendControlStatusMessage(STATUS_GAIN_TRANSIENT,
                                                      audio_control_stack[top+1].profile_id);
                    }
                    else if(pEvent->btamControlReq.request_type == REQUEST_TYPE_PERMANENT) {
                        SendControlStatusMessage(STATUS_LOSS,
                                                      audio_control_stack[top].profile_id);
                        RemoveNode(top);
                        AddNewNode(pEvent->btamControlReq.profile_id,
                                                      pEvent->btamControlReq.request_type);
                        SendControlStatusMessage(STATUS_GAIN,
                                                      audio_control_stack[top].profile_id);
                    }
                    break;
            }
            break;
        case BT_AM_RELEASE_CONTROL:
            // remove entry from audio control stack
            profile_index = GetIndex(pEvent->btamControlRelease.profile_id);
            top = GetTopIndex();
            if (profile_index < 0)
                break;
            // if its the topmost element and request type is Transient
            if ((profile_index == top) && (audio_control_stack[top].control_status ==
                        REQUEST_TYPE_TRANSIENT) && (top != 0)) {
                SendControlStatusMessage(STATUS_REGAINED,
                                       audio_control_stack[top-1].profile_id);
            }
            RemoveNode(profile_index);
            break;
    }
}

BT_Audio_Manager :: BT_Audio_Manager(const bt_interface_t *bt_interface, config_t *config) {
    for(int i= 0 ; i < MAX_PROFILE_ENTRIES; i++) {
        audio_control_stack[i].profile_id =  PROFILE_ID_MAX;
        audio_control_stack[i].control_status = REQUEST_TYPE_DEFAULT;
    }
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    qahw_mod_handle = NULL;
#endif
}

BT_Audio_Manager :: ~BT_Audio_Manager() {
    UnloadAudioHal();
}

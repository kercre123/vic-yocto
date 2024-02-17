/******************************************************************************
 *
 *  Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
 *  Not a Contribution.
 *  Copyright (C) 2014 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#include "ipc.h"
#include "osi/include/thread.h"
#include "osi/include/log.h"

ThreadInfo threadInfo[THREAD_ID_MAX] = {
    //thread_id thread type            Thread Message Handler    Thread Name
    { NULL ,    THREAD_ID_MAIN,        &BtMainMsgHandler,        "Main_Thread" } ,
    { NULL ,    THREAD_ID_GAP,         &BtGapMsgHandler,         "Gap_Thread" } ,
    { NULL ,    THREAD_ID_A2DP_SINK,   &BtA2dpSinkMsgHandler,    "A2dp_Sink_Thread" } ,
    { NULL ,    THREAD_ID_HFP_CLIENT,  &BtHfpClientMsgHandler,   "Hfp_Client_Thread" } ,
    { NULL ,    THREAD_ID_PAN,         &BtPanMsgHandler,         "Pan_Thread" } ,
    { NULL ,    THREAD_ID_GATT,        &BtGattMsgHandler,        "Gatt_Thread" } ,
    { NULL ,    THREAD_ID_BT_AM,       &BtAudioManagerHandler,   "BT_AUDIO_MANAGER_Thread" } ,
    { NULL ,    THREAD_ID_SDP_CLIENT,  &BtSdpClientMsgHandler,   "Sdp_Client_Thread" } ,
#ifdef USE_BT_OBEX
    { NULL ,    THREAD_ID_PBAP_CLIENT, &BtPbapClientMsgHandler,  "Pbap_Client_Thread" } ,
    { NULL ,    THREAD_ID_OPP,         &BtOppMsgHandler,         "Opp_Thread" } ,
#endif
    { NULL ,    THREAD_ID_HFP_AG,      &BtHfpAgMsgHandler,       "Hfp_AG_Thread" } ,
    { NULL ,    THREAD_ID_A2DP_SOURCE, &BtA2dpSourceMsgHandler,  "A2dp_Source_Thread" } ,
    { NULL ,    THREAD_ID_AVRCP,       &BtAvrcpMsgHandler,       "Avrcp_Thread" } ,
};

void PostMessage(ThreadIdType thread_type, void *msg) {
    if (thread_type >= THREAD_ID_MAX) {
        ALOGE(TAG " Invalid thread type %d", thread_type);
    } else if (!threadInfo[thread_type].thread_id) {
        ALOGE(TAG " Invalid thread id %d", threadInfo[thread_type].thread_id);
    }  else if (!threadInfo[thread_type].thread_handler) {
        ALOGE(TAG " Missing thread message handler");
    } else {
        thread_post(threadInfo[thread_type].thread_id, threadInfo[thread_type].
                thread_handler, msg);
    }
}


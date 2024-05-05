/* Copyright (c) 2016, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 * Copyright (C) 2012-2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <list>
#include <map>
#include <iostream>
#include <string.h>
#include <hardware/bluetooth.h>
#include <hardware/hardware.h>
#include <hardware/bt_hf.h>
#include "hardware/bt_hf_vendor.h"

#include "Audio_Manager.hpp"
#include "HfpAG.hpp"

#define LOGTAG "HFP_AG "

using namespace std;
using std::list;
using std::string;

Hfp_Ag *pHfpAG = NULL;
extern BT_Audio_Manager *pBTAM;

#ifdef __cplusplus
extern "C" {
#endif

#if defined(BT_MODEM_INTEGRATION)
const char *MCM_LIBRARY_NAME = "/usr/lib/libmcm.so.0";
#endif

void BtHfpAgMsgHandler(void *msg) {
    BtEvent* pEvent = NULL;
    if(!msg) {
        printf("Msg is NULL, return.\n");
        return;
    }

    pEvent = ( BtEvent *) msg;

    ALOGD(LOGTAG " BtHfpAgMsgHandler event = %d", pEvent->event_id);
    fprintf(stdout, " BtHfpAgMsgHandler event = %d\n", pEvent->event_id);
    switch(pEvent->event_id) {
        case PROFILE_API_START:
            if (pHfpAG) {
                pHfpAG->HandleEnableAg();
            }
            break;
        case PROFILE_API_STOP:
            if (pHfpAG) {
                pHfpAG->HandleDisableAg();
            }
            break;
        default:
            if(pHfpAG) {
               pHfpAG->ProcessEvent(( BtEvent *) msg);
            }
            break;
    }
    delete pEvent;
}

#ifdef __cplusplus
}
#endif

static void connection_state_callback(bthf_connection_state_t state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG " Connection State CB");
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    switch( state ) {
        case BTHF_CONNECTION_STATE_DISCONNECTED:
            pEvent->hfp_ag_event.event_id = HFP_AG_DISCONNECTED_CB;
        break;
        case BTHF_CONNECTION_STATE_CONNECTING:
            pEvent->hfp_ag_event.event_id = HFP_AG_CONNECTING_CB;
        break;
        case BTHF_CONNECTION_STATE_CONNECTED:
            pEvent->hfp_ag_event.event_id = HFP_AG_CONNECTED_CB;
        break;
        case BTHF_CONNECTION_STATE_SLC_CONNECTED:
            pEvent->hfp_ag_event.event_id = HFP_AG_SLC_CONNECTED_CB;
        break;
        case BTHF_CONNECTION_STATE_DISCONNECTING:
            pEvent->hfp_ag_event.event_id = HFP_AG_DISCONNECTING_CB;
        break;
        default:
        break;
    }
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

static void audio_state_callback(bthf_audio_state_t state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG " Audio State CB");
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    switch( state ) {
        case BTHF_AUDIO_STATE_DISCONNECTED:
            pEvent->hfp_ag_event.event_id = HFP_AG_AUDIO_STATE_DISCONNECTED_CB;
        break;
        case BTHF_AUDIO_STATE_CONNECTING:
            pEvent->hfp_ag_event.event_id = HFP_AG_AUDIO_STATE_CONNECTING_CB;
        break;
        case BTHF_AUDIO_STATE_CONNECTED:
            pEvent->hfp_ag_event.event_id = HFP_AG_AUDIO_STATE_CONNECTED_CB;
        break;
        case BTHF_AUDIO_STATE_DISCONNECTING:
            pEvent->hfp_ag_event.event_id = HFP_AG_AUDIO_STATE_DISCONNECTING_CB;
        break;
    }
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void voice_recognition_callback(bthf_vr_state_t state, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG "VR state is %s",(state == BTHF_VR_STATE_STOPPED) ? "stopped": "started");
    fprintf(stdout, "VR state is %s\n",(state == BTHF_VR_STATE_STOPPED) ? "stopped": "started");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.arg1 = state;
    pEvent->hfp_ag_event.event_id = HFP_AG_VR_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void answer_call_callback(bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " answer_call_callback");
    fprintf(stdout, " answer_call_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.event_id = HFP_AG_ANSWER_CALL_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void hangup_call_callback(bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " hangup_call_callback");
    fprintf(stdout, " hangup_call_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.event_id = HFP_AG_HANGUP_CALL_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void volume_control_callback(bthf_volume_type_t type, int volume, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG "%s : %s volume is %d", __func__,
          (type == BTHF_VOLUME_TYPE_SPK) ? "speaker": "mic", volume);

    fprintf(stdout, "%s : %s volume is %d\n", __func__,
          (type == BTHF_VOLUME_TYPE_SPK) ? "speaker": "mic", volume);

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.event_id = HFP_AG_VOL_CONTROL_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void dial_call_callback(char *number, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " dial_call_callback");
    fprintf(stdout, " dial_call_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));

    // for AT+BLDN, number will be NULL
    if (number == NULL)
       pEvent->hfp_ag_event.str[0] = '\0';
    else
       strncpy(pEvent->hfp_ag_event.str, number, strlen(number));

    pEvent->hfp_ag_event.event_id = HFP_AG_DIAL_CALL_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void dtmf_cmd_callback(char dtmf, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " dtmf_cmd_callback");
    fprintf(stdout, " dtmf_cmd_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.arg1 = dtmf;
    pEvent->hfp_ag_event.event_id = HFP_AG_DTMF_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void noice_reduction_callback(bthf_nrec_t nrec, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " noice_reduction_callback");
    fprintf(stdout, " noice_reduction_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.arg1 = nrec;
    pEvent->hfp_ag_event.event_id = HFP_AG_NREC_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void wbs_callback(bthf_wbs_config_t wbs_config, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " wbs_callback");
    fprintf(stdout, " wbs_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.arg1 = wbs_config;
    pEvent->hfp_ag_event.event_id = HFP_AG_WBS_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void at_chld_callback(bthf_chld_type_t chld, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " at_chld_callback");
    fprintf(stdout, " at_chld_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.arg1 = (int)chld;
    pEvent->hfp_ag_event.event_id = HFP_AG_CHLD_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void at_cnum_callback(bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " at_cnum_callback");
    fprintf(stdout, " at_cnum_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.event_id = HFP_AG_SUBSCRIBER_INFO_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void at_cind_callback(bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " at_cind_callback");
    fprintf(stdout, "at_cind_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.event_id = HFP_AG_CIND_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void at_cops_callback(bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " at_cops_callback");
    fprintf(stdout, "at_cops_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.event_id = HFP_AG_COPS_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void at_clcc_callback(bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " at_clcc_callback");
    fprintf(stdout, "at_clcc_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.event_id = HFP_AG_CLCC_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void unknown_at_callback(char *at_string, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " unknown_at_callback");
    fprintf(stdout, "unknown_at_callback\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->hfp_ag_event.event_id = HFP_AG_UNKNOWN_AT_CMD_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void key_pressed_callback(bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG " key_pressed_callback");
}

void bind_cmd_vendor_cb(char* hf_ind, bthf_vendor_bind_type_t type, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " bind_cmd_vendor_cb");
    fprintf(stdout, " bind_cmd_vendor_cb\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    strncpy(pEvent->hfp_ag_event.str, hf_ind, strlen(hf_ind));
    pEvent->hfp_ag_event.arg1 = type;
    pEvent->hfp_ag_event.event_id = HFP_AG_BIND_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void biev_cmd_vendor_cb(char* hf_ind_val, bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG " biev_cmd_vendor_cb");
    fprintf(stdout, " biev_cmd_vendor_cb\n");

    memcpy(&pEvent->hfp_ag_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    strncpy(pEvent->hfp_ag_event.str, hf_ind_val, strlen(hf_ind_val));
    pEvent->hfp_ag_event.event_id = HFP_AG_BIEV_CB;
    PostMessage(THREAD_ID_HFP_AG, pEvent);
}

static bthf_callbacks_t sBluetoothHfpAgCallbacks = {
    sizeof(sBluetoothHfpAgCallbacks),
    connection_state_callback,
    audio_state_callback,
    voice_recognition_callback,
    answer_call_callback,
    hangup_call_callback,
    volume_control_callback,
    dial_call_callback,
    dtmf_cmd_callback,
    noice_reduction_callback,
    wbs_callback,
    at_chld_callback,
    at_cnum_callback,
    at_cind_callback,
    at_cops_callback,
    at_clcc_callback,
    unknown_at_callback,
    key_pressed_callback
};

static bthf_vendor_callbacks_t sBluetoothHfpAgVendorCallbacks = {
    sizeof(sBluetoothHfpAgVendorCallbacks),
    bind_cmd_vendor_cb,
    biev_cmd_vendor_cb,
};

#if defined(BT_MODEM_INTEGRATION)
void ril_ind_cb(mcm_client_handle_type hndl, uint32 msg_id,
                     void *ind_c_struct, uint32 ind_len) {
   BtEvent *pEvent = new BtEvent;
   fprintf(stdout, "%s: indications is %u\n",__func__, msg_id);
   ALOGD(LOGTAG "%s: indications is %u\n", __func__, msg_id);

   if (ind_c_struct == NULL) {
       ALOGE(LOGTAG "%s: indication data is NULL", __func__);
       fprintf(stdout, "indication data is NULL\n");
       return;
   }

   pEvent->hfp_ag_event.hdl = hndl;
   pEvent->hfp_ag_event.msg_id = msg_id;
   pEvent->hfp_ag_event.data_length = ind_len;
   memcpy(&pEvent->hfp_ag_event.data, ind_c_struct, ind_len);
   pEvent->hfp_ag_event.event_id = HFP_AG_RIL_IND_CB;
   PostMessage(THREAD_ID_HFP_AG, pEvent);
}

void ril_resp_cb(mcm_client_handle_type hndl, uint32 msg_id,
                      void *resp_c_struct, uint32 resp_len, void *token_id){
   BtEvent *pEvent = new BtEvent;
   fprintf(stdout, "%s: response msg %u\n", __func__, msg_id);
   ALOGD(LOGTAG "%s: response msg  is %u\n", __func__, msg_id);

   if (resp_c_struct == NULL) {
       ALOGE(LOGTAG "%s: response data is NULL", __func__);
       fprintf(stdout, "response data is NULL\n");
       return;
   }

   pEvent->hfp_ag_event.hdl = hndl;
   pEvent->hfp_ag_event.msg_id = msg_id;
   pEvent->hfp_ag_event.data_length = resp_len;
   memcpy(&pEvent->hfp_ag_event.data, resp_c_struct, resp_len);
   pEvent->hfp_ag_event.event_id = HFP_AG_RIL_RESP_CB;
   PostMessage(THREAD_ID_HFP_AG, pEvent);
}
#endif

void Hfp_Ag::HandleEnableAg(void) {
    if (bluetooth_interface != NULL)
    {
        sBtHfpAgInterface = (bthf_interface_t *)bluetooth_interface->
                get_profile_interface(BT_PROFILE_HANDSFREE_ID);
        if (sBtHfpAgInterface == NULL)
        {
            // TODO: sent message to indicate failure for profile init
            ALOGE(LOGTAG "get profile interface failed, returning");
            return;
        }
        sBtHfpAgVendorInterface = (bthf_vendor_interface_t *)bluetooth_interface->
            get_profile_interface(BT_PROFILE_HANDSFREE_VENDOR_ID);
        if (sBtHfpAgVendorInterface == NULL)
        {
            ALOGE(LOGTAG "get profile vendor interface failed, returning");
            return;
        }
        change_state(HFP_AG_STATE_DISCONNECTED);
        sBtHfpAgInterface->init(&sBluetoothHfpAgCallbacks, 1);
        sBtHfpAgVendorInterface->init_vendor(&sBluetoothHfpAgVendorCallbacks);

#if defined(BT_MODEM_INTEGRATION)
        init_modem();
#endif

#if defined(BT_ALSA_AUDIO_INTEGRATION)
        init_audio();
#endif
        BtEvent *pEvent = new BtEvent;
        pEvent->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
        pEvent->profile_start_event.profile_id = PROFILE_ID_HFP_AG;
        pEvent->profile_start_event.status = true;
        PostMessage(THREAD_ID_GAP, pEvent);
    }
}

void Hfp_Ag::HandleDisableAg(void) {
   change_state(HFP_AG_STATE_NOT_STARTED);
   if(sBtHfpAgInterface != NULL) {
       sBtHfpAgInterface->cleanup();
       sBtHfpAgInterface = NULL;
   }
   if(sBtHfpAgVendorInterface != NULL) {
       sBtHfpAgVendorInterface->cleanup_vendor();
       sBtHfpAgVendorInterface = NULL;
   }
#if defined(BT_MODEM_INTEGRATION)
   release_modem();
#endif

#if defined(BT_ALSA_AUDIO_INTEGRATION)
   release_audio();
#endif

   BtEvent *pEvent = new BtEvent;
   pEvent->profile_stop_event.event_id = PROFILE_EVENT_STOP_DONE;
   pEvent->profile_stop_event.profile_id = PROFILE_ID_HFP_AG;
   pEvent->profile_stop_event.status = true;
   PostMessage(THREAD_ID_GAP, pEvent);
}

void Hfp_Ag::ProcessEvent(BtEvent* pEvent) {
    ALOGD(LOGTAG " Processing event %d", pEvent->event_id);
    fprintf(stdout, " AG: Processing event = %d\n", pEvent->event_id);
    switch(mAgState) {
        case HFP_AG_STATE_DISCONNECTED:
            state_disconnected_handler(pEvent);
            break;
        case HFP_AG_STATE_PENDING:
            state_pending_handler(pEvent);
            break;
        case HFP_AG_STATE_CONNECTED:
            state_connected_handler(pEvent);
            break;
        case HFP_AG_STATE_AUDIO_ON:
            state_audio_on_handler(pEvent);
            break;
        case HFP_AG_STATE_NOT_STARTED:
            ALOGE(LOGTAG " STATE UNINITIALIZED, return");
            break;
    }
}

void Hfp_Ag::state_disconnected_handler(BtEvent* pEvent) {
    char str[18];
    ALOGD(LOGTAG "state_disconnected_handler Processing event %d", pEvent->event_id);
    fprintf(stdout, "state_disconnected_handler Processing event %d\n", pEvent->event_id);
    switch(pEvent->event_id) {
        case HFP_AG_API_CONNECT_REQ:
            memcpy(&mConnectingDevice, &pEvent->hfp_ag_event.bd_addr, sizeof(bt_bdaddr_t));
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->connect(&pEvent->hfp_ag_event.bd_addr);
            }
            bdaddr_to_string(&mConnectingDevice, str, 18);
            fprintf(stdout, "connecting with device %s", str);
            ALOGD(LOGTAG " connecting with device %s", str);
            change_state(HFP_AG_STATE_PENDING);
            break;
        case HFP_AG_CONNECTING_CB:
            memcpy(&mConnectingDevice, &pEvent->hfp_ag_event.bd_addr, sizeof(bt_bdaddr_t));
            change_state(HFP_AG_STATE_PENDING);
            break;
        case HFP_AG_CONNECTED_CB:
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            memcpy(&mConnectedDevice, &pEvent->hfp_ag_event.bd_addr, sizeof(bt_bdaddr_t));

            bdaddr_to_string(&mConnectedDevice, str, 18);
            fprintf(stdout, " connected with device %s", str);
            ALOGD(LOGTAG " connected with device %s", str);

            change_state(HFP_AG_STATE_CONNECTED);
            break;
        default:
            ALOGD(LOGTAG " event not handled %d ", pEvent->event_id);
            break;
    }
}
void Hfp_Ag::state_pending_handler(BtEvent* pEvent) {
    char str[18];
    ALOGD(LOGTAG "state_pending_handler Processing event %d", pEvent->event_id);
    fprintf(stdout, "state_pending_handler Processing event %d\n", pEvent->event_id);
    switch(pEvent->event_id) {
        case HFP_AG_CONNECTING_CB:
            break;
        case HFP_AG_CONNECTED_CB:
            memcpy(&mConnectedDevice, &pEvent->hfp_ag_event.bd_addr, sizeof(bt_bdaddr_t));
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));

            bdaddr_to_string(&mConnectedDevice, str, 18);
            fprintf(stdout, "connected with device %s", str);
            ALOGD(LOGTAG "connected with device %s", str);
            change_state(HFP_AG_STATE_CONNECTED);
            break;
        case HFP_AG_DISCONNECTED_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Disconnected from or Unable to connect with device %s", str);
            ALOGD(LOGTAG "Disconnected from or Unable to connect with device %s", str);

            memset(&mConnectedDevice, 0, sizeof(bt_bdaddr_t));
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            change_state(HFP_AG_STATE_DISCONNECTED);
            break;
        default:
            ALOGD(LOGTAG " event not handled %d ", pEvent->event_id);
            break;
    }
}

void Hfp_Ag::state_connected_handler(BtEvent* pEvent) {
    ALOGD(LOGTAG "state_connected_handler Processing event %d", pEvent->event_id);
    fprintf(stdout, "state_connected_handler Processing event = %d", pEvent->event_id);
    char str[18];
    BtEvent *pControlRequest, *pReleaseControlReq;
    switch(pEvent->event_id) {
        case HFP_AG_API_CONNECT_REQ: // TODO: handle connections to another device
            break;
        case HFP_AG_API_DISCONNECT_REQ:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            if (sBtHfpAgInterface != NULL) {
                bt_status_t ret_val;
                ret_val = sBtHfpAgInterface->disconnect(&pEvent->hfp_ag_event.bd_addr);
                if (ret_val != BT_STATUS_SUCCESS) {
                    fprintf(stdout, "Failure disconnecting with device %s", str);
                    ALOGD(LOGTAG "Failure disconnecting with device %s", str);
                    break;
                }
            }

            fprintf(stdout, "Disconnecting with device %s", str);
            ALOGD(LOGTAG "Disconnecting with device %s", str);
            memset(&mConnectedDevice, 0, sizeof(bt_bdaddr_t));
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            change_state(HFP_AG_STATE_PENDING);
            break;
        case HFP_AG_SLC_CONNECTED_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "SLC connected with device %s", str);
            ALOGD(LOGTAG " SLC connected with device %s", str);
#if defined(BT_MODEM_INTEGRATION)
            processSlcConnected();
#endif
            break;
        case HFP_AG_DISCONNECTED_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Disconnected with device %s", str);
            ALOGD(LOGTAG "Disconnected with device %s", str);

            memset(&mConnectedDevice, 0, sizeof(bt_bdaddr_t));
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            change_state(HFP_AG_STATE_DISCONNECTED);
            break;
        case HFP_AG_DISCONNECTING_CB:
            break;
        case HFP_AG_VR_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "VR start/stop req from device %s", str);
            ALOGD(LOGTAG "VR start/stop req from device %s", str);

            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
            break;
        case HFP_AG_WBS_CB:
            mWbsState = (bthf_wbs_config_t)pEvent->hfp_ag_event.arg1;
            break;
        case HFP_AG_NREC_CB:
            mNrec = (bthf_nrec_t)pEvent->hfp_ag_event.arg1;
            break;
        case HFP_AG_ANSWER_CALL_CB:
            // answer call using RIL APIs.
            // OK will be sent from stack itself.
#if defined(BT_MODEM_INTEGRATION)
            send_voice_cmd(MCM_VOICE_CALL_ANSWER_V01);
#endif
            break;
        case HFP_AG_HANGUP_CALL_CB:
            // OK will be sent from stack itself.
#if defined(BT_MODEM_INTEGRATION)
            end_call(BTHF_CALL_STATE_ACTIVE);
#endif
            break;
        case HFP_AG_VOL_CONTROL_CB:
            // TODO: change the speaker volume using mm audio shell script
            // OK will be sent from stack itself.
            break;
        case HFP_AG_DIAL_CALL_CB:
#if defined(BT_MODEM_INTEGRATION)
            dial_call(pEvent->hfp_ag_event.str, &pEvent->hfp_ag_event.bd_addr);
#else
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
#endif
            break;
        case HFP_AG_CIND_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Sending CIND resp to device %s", str);
            ALOGD(LOGTAG "Sending CIND resp to device %s", str);

            if (sBtHfpAgInterface != NULL) {
#if defined(BT_MODEM_INTEGRATION)
                // we already have active/held/ringing call, call setup info. send it to stack
                sBtHfpAgInterface->cind_response(1, mNumActiveCalls, mNumHeldCalls,
                                                mCallSetupState, 5, 0, 5, &pEvent->hfp_ag_event.bd_addr);
#else
                sBtHfpAgInterface->cind_response(1, 0, 0, BTHF_CALL_STATE_IDLE, 5, 0, 5, &pEvent->hfp_ag_event.bd_addr);
#endif

            }
            break;
        case HFP_AG_CHLD_CB:
#if defined(BT_MODEM_INTEGRATION)
            {
               uint32 ret_val = 0;
               ret_val = process_chld(pEvent->hfp_ag_event.arg1);
               if (ret_val != MCM_SUCCESS_V01) {
                   ALOGE(LOGTAG, "error processing chld %d", pEvent->hfp_ag_event.arg1);
                   fprintf(stdout, "error processing chld %d", pEvent->hfp_ag_event.arg1);
               }

               if (sBtHfpAgInterface != NULL) {
                   sBtHfpAgInterface->at_response( (ret_val == MCM_SUCCESS_V01)
                         ? BTHF_AT_RESPONSE_OK : BTHF_AT_RESPONSE_ERROR,
                         0, &pEvent->hfp_ag_event.bd_addr);
               }
            }
#else
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
#endif
            break;
        case HFP_AG_COPS_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Sending COPS resp to device %s", str);
            ALOGD(LOGTAG "Sending COPS resp to device %s", str);

#if defined(BT_MODEM_INTEGRATION)
            //  info needs to be fetched from RIL for MDM
            get_and_send_operator_name(&pEvent->hfp_ag_event.bd_addr);
#else
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->cops_response("", &pEvent->hfp_ag_event.bd_addr);
            }
#endif
            break;
        case HFP_AG_SUBSCRIBER_INFO_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Sending CNUM resp to device %s", str);
            ALOGD(LOGTAG "Sending CNUM resp to device %s", str);

#if defined(BT_MODEM_INTEGRATION)
            //  info needs to be fetched from RIL for MDM
            get_and_send_subscriber_number(&pEvent->hfp_ag_event.bd_addr);
#else
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
#endif
            break;
        case HFP_AG_CLCC_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Sending CLCC resp to device %s", str);
            ALOGD(LOGTAG "Sending CLCC resp to device %s", str);

            // TODO: cross check if we need to call get_current_calls here.

            if (sBtHfpAgInterface != NULL) {
#if defined(BT_MODEM_INTEGRATION)
                for (int i = 0; i < MCM_MAX_VOICE_CALLS_V01; i++) {
                    if (mCalls[i].call_id != 0xFFFFFFFF)
                        sBtHfpAgInterface->clcc_response(mCalls[i].idx,
                                                         mCalls[i].dir,
                                                         mCalls[i].stat,
                                                         mCalls[i].mode,
                                                         mCalls[i].mpty,
                                                         mCalls[i].number,
                                                         mCalls[i].numType,
                                                         &pEvent->hfp_ag_event.bd_addr);
                }
#endif
                // just send OK for now
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_OK, 0, &pEvent->hfp_ag_event.bd_addr);
            }
            break;
         case HFP_AG_UNKNOWN_AT_CMD_CB:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
         break;
#if defined(BT_MODEM_INTEGRATION)
         case HFP_AG_RIL_IND_CB:
             process_ril_ind(pEvent);
             break;
         case HFP_AG_RIL_RESP_CB:
             process_ril_resp(pEvent);
             break;
#endif
        case HFP_AG_API_CONNECT_AUDIO_REQ:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Connecting SCO/eSCO with device %s", str);
            ALOGD(LOGTAG "Connecting SCO/eSCO with device %s", str);

            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->connect_audio(&pEvent->hfp_ag_event.bd_addr);
            }
            break;
        case HFP_AG_AUDIO_STATE_CONNECTED_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "SCO/eSCO connected with device %s, codec %s", str,
                ((mWbsState == BTHF_WBS_YES)? "WBS": "NBS"));
            ALOGD(LOGTAG "SCO/eSCO connected with device %s, codec %s", str,
                ((mWbsState == BTHF_WBS_YES)? "WBS": "NBS"));

#if defined(BT_ALSA_AUDIO_INTEGRATION)
            setup_sco_path();
#endif
            change_state(HFP_AG_STATE_AUDIO_ON);
            break;
        case HFP_AG_BIND_CB:
            process_at_bind(pEvent);
            break;
        case HFP_AG_BIEV_CB:
            process_at_biev(pEvent);
            break;
        case HFP_AG_API_ACCEPT_CALL_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_RELEASE_HELD_CALL_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_REJECT_CALL_REQ:
            break;
        case HFP_AG_API_END_CALL_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_HOLD_CALL_REQ:
            break;
        case HFP_AG_API_SWAP_CALLS_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_DIAL_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_REDIAL_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_START_VR_REQ:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->start_voice_recognition(&mConnectedDevice);
            }
            break;
        case HFP_AG_API_STOP_VR_REQ:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->stop_voice_recognition(&mConnectedDevice);
            }
            break;
        case HFP_AG_API_QUERY_CURRENT_CALLS_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_QUERY_OPERATOR_NAME_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_QUERY_SUBSCRIBER_INFO_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_SPK_VOL_CTRL_REQ:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->volume_control(BTHF_VOLUME_TYPE_SPK,
                                          pEvent->hfp_ag_event.arg1, &mConnectedDevice);
            }
            break;
        case HFP_AG_API_MIC_VOL_CTRL_REQ:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->volume_control(BTHF_VOLUME_TYPE_MIC,
                                          pEvent->hfp_ag_event.arg1, &mConnectedDevice);
            }
            break;
        default:
            ALOGD(LOGTAG," event not handled %d ", pEvent->event_id);
            break;
    }
}

void Hfp_Ag::state_audio_on_handler(BtEvent* pEvent) {
    char str[18];
    BtEvent *pControlRequest, *pReleaseControlReq;
    ALOGD(LOGTAG "state_audio_on_handler Processing event %d", pEvent->event_id);
    switch(pEvent->event_id) {
        case HFP_AG_API_DISCONNECT_REQ:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);

            // disconnect SCO, clean up SCO
#if defined(BT_ALSA_AUDIO_INTEGRATION)
            teardown_sco_path();
#endif
            if (sBtHfpAgInterface != NULL) {
                bt_status_t ret_val;
                // no need to check if disconnection of SCO is success here.
                sBtHfpAgInterface->disconnect_audio(&pEvent->hfp_ag_event.bd_addr);

                ret_val = sBtHfpAgInterface->disconnect(&pEvent->hfp_ag_event.bd_addr);
                if (ret_val != BT_STATUS_SUCCESS) {
                    fprintf(stdout, "Failure disconnecting with device %s", str);
                    ALOGD(LOGTAG "Failure disconnecting with device %s", str);
                    break;
                }
            }

            fprintf(stdout, "Disconnecting with device %s", str);
            ALOGD(LOGTAG "Disconnecting with device %s", str);
            memset(&mConnectedDevice, 0, sizeof(bt_bdaddr_t));
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            change_state(HFP_AG_STATE_PENDING);
            break;
        case HFP_AG_API_DISCONNECT_AUDIO_REQ:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->disconnect_audio(&pEvent->hfp_ag_event.bd_addr);
            }

            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Disconnecting SCO/eSCO with device %s", str);
            ALOGD(LOGTAG "Disconnecting SCO/eSCO with device %s", str);
            break;
        case HFP_AG_AUDIO_STATE_DISCONNECTED_CB:

            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Disconnected SCO connection with device %s", str);
            ALOGD(LOGTAG "Disconnected SCO connection with device %s", str);

#if defined(BT_ALSA_AUDIO_INTEGRATION)
            teardown_sco_path();
#endif
            change_state(HFP_AG_STATE_CONNECTED);
            break;
        case HFP_AG_VR_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "VR start/stop req from device %s", str);
            ALOGD(LOGTAG "VR start/stop req from device %s", str);

            // send error for VR start/stop request

            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
            break;
        case HFP_AG_WBS_CB:
            mWbsState = (bthf_wbs_config_t)pEvent->hfp_ag_event.arg1;
            break;
        case HFP_AG_NREC_CB:
            mNrec = (bthf_nrec_t)pEvent->hfp_ag_event.arg1;
            break;
        case HFP_AG_ANSWER_CALL_CB:
            // answer call using RIL APIs.
            // OK will be sent from stack itself.
#if defined(BT_MODEM_INTEGRATION)
            send_voice_cmd(MCM_VOICE_CALL_ANSWER_V01);
#endif
            break;
        case HFP_AG_HANGUP_CALL_CB:
            // OK will be sent from stack itself.
#if defined(BT_MODEM_INTEGRATION)
            end_call(BTHF_CALL_STATE_ACTIVE);
#endif
            break;
        case HFP_AG_VOL_CONTROL_CB:
            // TODO: change the speaker volume using mm audio shell script
            // OK will be sent from stack itself.
            break;
        case HFP_AG_DIAL_CALL_CB:
#if defined(BT_MODEM_INTEGRATION)
            dial_call(pEvent->hfp_ag_event.str, &pEvent->hfp_ag_event.bd_addr);
#else
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
#endif
            break;
        case HFP_AG_CIND_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Sending CIND resp to device %s", str);
            ALOGD(LOGTAG "Sending CIND resp to device %s", str);

            if (sBtHfpAgInterface != NULL) {
#if defined(BT_MODEM_INTEGRATION)
                // we already have active/held/ringing call, call setup info. send it to stack
                sBtHfpAgInterface->cind_response(1, mNumActiveCalls, mNumHeldCalls,
                                                mCallSetupState, 5, 0, 5, &pEvent->hfp_ag_event.bd_addr);
#else
                sBtHfpAgInterface->cind_response(1, 0, 0, BTHF_CALL_STATE_IDLE, 5, 0, 5, &pEvent->hfp_ag_event.bd_addr);
#endif

            }
            break;
        case HFP_AG_CHLD_CB:
#if defined(BT_MODEM_INTEGRATION)
            {
               uint32 ret_val = 0;
               ret_val = process_chld(pEvent->hfp_ag_event.arg1);
               if (ret_val != MCM_SUCCESS_V01) {
                   ALOGE(LOGTAG, "error processing chld %d", pEvent->hfp_ag_event.arg1);
                   fprintf(stdout, "error processing chld %d", pEvent->hfp_ag_event.arg1);
               }

               if (sBtHfpAgInterface != NULL) {
                   sBtHfpAgInterface->at_response( (ret_val == MCM_SUCCESS_V01)
                         ? BTHF_AT_RESPONSE_OK : BTHF_AT_RESPONSE_ERROR,
                         0, &pEvent->hfp_ag_event.bd_addr);
               }
            }
#else
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
#endif
            break;
        case HFP_AG_COPS_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Sending COPS resp to device %s", str);
            ALOGD(LOGTAG "Sending COPS resp to device %s", str);

#if defined(BT_MODEM_INTEGRATION)
            //  info needs to be fetched from RIL for MDM
            get_and_send_operator_name(&pEvent->hfp_ag_event.bd_addr);
#else
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->cops_response("", &pEvent->hfp_ag_event.bd_addr);
            }
#endif
            break;
        case HFP_AG_SUBSCRIBER_INFO_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Sending CNUM resp to device %s", str);
            ALOGD(LOGTAG "Sending CNUM resp to device %s", str);

#if defined(BT_MODEM_INTEGRATION)
            //  info needs to be fetched from RIL for MDM
            get_and_send_subscriber_number(&pEvent->hfp_ag_event.bd_addr);
#else
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
#endif
            break;
        case HFP_AG_CLCC_CB:
            bdaddr_to_string(&pEvent->hfp_ag_event.bd_addr, str, 18);
            fprintf(stdout, "Sending CLCC resp to device %s", str);
            ALOGD(LOGTAG "Sending CLCC resp to device %s", str);

            // TODO: cross check if we need to call get_current_calls here.

            if (sBtHfpAgInterface != NULL) {
#if defined(BT_MODEM_INTEGRATION)
                for (int i = 0; i < MCM_MAX_VOICE_CALLS_V01; i++) {
                    if (mCalls[i].call_id != 0xFFFFFFFF)
                        sBtHfpAgInterface->clcc_response(mCalls[i].idx,
                                                         mCalls[i].dir,
                                                         mCalls[i].stat,
                                                         mCalls[i].mode,
                                                         mCalls[i].mpty,
                                                         mCalls[i].number,
                                                         mCalls[i].numType,
                                                         &pEvent->hfp_ag_event.bd_addr);
                }
#endif
                // just send OK for now
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_OK, 0, &pEvent->hfp_ag_event.bd_addr);
            }
            break;
         case HFP_AG_UNKNOWN_AT_CMD_CB:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
            }
         break;
#if defined(BT_MODEM_INTEGRATION)
         case HFP_AG_RIL_IND_CB:
             process_ril_ind(pEvent);
             break;
         case HFP_AG_RIL_RESP_CB:
             process_ril_resp(pEvent);
             break;
#endif
        case HFP_AG_BIND_CB:
            process_at_bind(pEvent);
            break;
        case HFP_AG_BIEV_CB:
            process_at_biev(pEvent);
            break;
        case HFP_AG_API_ACCEPT_CALL_REQ:
            break;
        case HFP_AG_API_RELEASE_HELD_CALL_REQ:
            break;
        case HFP_AG_API_REJECT_CALL_REQ:
            break;
        case HFP_AG_API_END_CALL_REQ:
            break;
        case HFP_AG_API_HOLD_CALL_REQ:
            break;
        case HFP_AG_API_SWAP_CALLS_REQ:
            break;
        case HFP_AG_API_DIAL_REQ:
            break;
        case HFP_AG_API_REDIAL_REQ:
            break;
        case HFP_AG_API_START_VR_REQ:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->start_voice_recognition(&mConnectedDevice);
            }
            break;
        case HFP_AG_API_STOP_VR_REQ:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->stop_voice_recognition(&mConnectedDevice);
            }
            break;
        case HFP_AG_API_QUERY_CURRENT_CALLS_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_QUERY_OPERATOR_NAME_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_QUERY_SUBSCRIBER_INFO_REQ:
            if (sBtHfpAgInterface != NULL) {
            }
            break;
        case HFP_AG_API_SPK_VOL_CTRL_REQ:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->volume_control(BTHF_VOLUME_TYPE_SPK,
                                          pEvent->hfp_ag_event.arg1, &mConnectedDevice);
            }
            break;
        case HFP_AG_API_MIC_VOL_CTRL_REQ:
            if (sBtHfpAgInterface != NULL) {
                sBtHfpAgInterface->volume_control(BTHF_VOLUME_TYPE_MIC,
                                          pEvent->hfp_ag_event.arg1, &mConnectedDevice);
            }
            break;
        default:
            ALOGD(LOGTAG," event not handled %d ", pEvent->event_id);
            break;
    }

}

void Hfp_Ag::ConfigureAudio(bool enable) {

}

#if defined(BT_MODEM_INTEGRATION)

void Hfp_Ag::processSlcConnected() {
  //  update the calls info to stack once done with SLC
  // TODO: should we add any delay here?
   sBtHfpAgInterface->phone_state_change(mNumActiveCalls,
                               mNumHeldCalls,
                               mCallSetupState,
                               mRingingAddress == NULL ? "" : mRingingAddress,
                               BTHF_CALL_ADDRTYPE_INTERNATIONAL);
}

void Hfp_Ag::get_and_send_operator_name(bt_bdaddr_t *bd_addr) {
   uint32 ret_val = 0;
   mcm_nw_get_operator_name_req_msg_v01 cops_req;

   memset(&cops_req, 0, sizeof(cops_req));
   memset(&cops_resp, 0, sizeof(cops_resp));

   ret_val = mcm_client_execute_command_sync_ptr(mcm_client_hdl,
                                 MCM_NW_GET_OPERATOR_NAME_REQ_V01,
                                 &cops_req,
                                 sizeof(cops_req),
                                 &cops_resp,
                                 sizeof(mcm_nw_get_operator_name_resp_msg_v01));
   if (ret_val == MCM_SUCCESS_V01 && cops_resp.operator_name_valid) {
       ALOGD(LOGTAG, "getting operator name successful");
       if (sBtHfpAgInterface != NULL) {
           // TODO: cross check of short_eons has the operator name
           sBtHfpAgInterface->cops_response(cops_resp.operator_name.short_eons, bd_addr);
       }
   }
   else {
       ALOGE(LOGTAG, "getting operator name list failed");
       if (sBtHfpAgInterface != NULL) {
           sBtHfpAgInterface->cops_response("", bd_addr);
       }
   }
}

void Hfp_Ag::get_and_send_subscriber_number(bt_bdaddr_t *bd_addr) {
   uint32 ret_val = 0;
   mcm_sim_get_device_phone_number_req_msg_v01 cnum_req;

   memset(&cnum_req, 0, sizeof(cnum_req));

   ret_val = mcm_client_execute_command_sync_ptr(mcm_client_hdl,
                                 MCM_SIM_GET_DEVICE_PHONE_NUMBER_REQ_V01,
                                 &cnum_req,
                                 sizeof(cnum_req),
                                 &get_phone_num_resp,
                                 sizeof(mcm_sim_get_device_phone_number_resp_msg_v01));
   if (ret_val == MCM_SUCCESS_V01 &&
         get_phone_num_resp.resp.result == MCM_RESULT_SUCCESS_V01 &&
         get_phone_num_resp.phone_number_valid) {
       ALOGD(LOGTAG, "getting subscriber info successful");
       if (sBtHfpAgInterface != NULL) {
           char phone_num_str[256];
           strcpy(phone_num_str, "+CNUM: ,\"");

           // dest buffer is 256 bytes length handle buffer overflow if phone number length is > 239
           if (get_phone_num_resp.phone_number_len > 239)
               strncat(phone_num_str, get_phone_num_resp.phone_number, 239);
           else
               strcat(phone_num_str, get_phone_num_resp.phone_number);

           strcat(phone_num_str, "\",145,,4");

           sBtHfpAgInterface->formatted_at_response(phone_num_str, bd_addr);
           sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_OK, 0, bd_addr);
       }
   }
   else {
       ALOGE(LOGTAG, "getting subscriber info failed");
       if (sBtHfpAgInterface != NULL) {
           sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, bd_addr);
       }
   }
}

bthf_call_state_t Hfp_Ag::get_call_state(mcm_voice_call_state_t_v01 state){
    switch (state) {
       case MCM_VOICE_CALL_STATE_INCOMING_V01:
          mNumRingingCalls++;
          mCallSetupState = BTHF_CALL_STATE_INCOMING;
          return BTHF_CALL_STATE_INCOMING;
       case MCM_VOICE_CALL_STATE_DIALING_V01:
          mCallSetupState = BTHF_CALL_STATE_DIALING;
          return BTHF_CALL_STATE_DIALING;
       case MCM_VOICE_CALL_STATE_ALERTING_V01:
          mCallSetupState = BTHF_CALL_STATE_ALERTING;
          return BTHF_CALL_STATE_ALERTING;
       case MCM_VOICE_CALL_STATE_ACTIVE_V01:
          mNumActiveCalls++;
          return BTHF_CALL_STATE_ACTIVE;
       case MCM_VOICE_CALL_STATE_HOLDING_V01:
          mNumHeldCalls++;
          return BTHF_CALL_STATE_HELD;
       case MCM_VOICE_CALL_STATE_WAITING_V01:
          mNumRingingCalls++;
          mCallSetupState = BTHF_CALL_STATE_INCOMING;
          return BTHF_CALL_STATE_INCOMING;
       case MCM_VOICE_CALL_STATE_END_V01:
          // intenntional fall through
       default:
          return BTHF_CALL_STATE_IDLE;
    }
}

void Hfp_Ag::process_call_list(mcm_voice_call_record_t_v01 *calls, uint32_t num_calls) {
    mcm_voice_call_record_t_v01 *call_record;
    uint32_t i, j;

    mNumActiveCalls = 0;
    mNumHeldCalls = 0;
    mNumRingingCalls = 0;
    mCallSetupState = BTHF_CALL_STATE_IDLE;
    mRingingAddress = NULL;
    memset(mCalls, 0, sizeof(mCalls));

    // call_id should be invalidated
    for (i = 0; i < MCM_MAX_VOICE_CALLS_V01; i++) {
        mCalls[i].call_id = 0xFFFFFFFF;
        mCalls[i].idx = 0;
    }

    for (i = 0, j = 0; j < num_calls; j++) {
        call_record = &calls[j];

        // if list contains terminated call info, don't update its info in our call list
        if (call_record->state == MCM_VOICE_CALL_STATE_END_V01)
           continue;

        mCalls[i].call_id = call_record->call_id;  // call_id is not index
        mCalls[i].idx = i+1; // in CLCC resp, index starts from 1
        mCalls[i].dir = (call_record->direction == MCM_VOICE_CALL_MOBILE_ORIGINATED_V01)
                          ? BTHF_CALL_DIRECTION_OUTGOING : BTHF_CALL_DIRECTION_INCOMING;
        mCalls[i].stat = get_call_state(call_record->state);
        mCalls[i].mode = BTHF_CALL_TYPE_VOICE;
        // TODO: this needs to be revisited since RIL does not provide this info
        mCalls[i].mpty = BTHF_CALL_MPTY_TYPE_SINGLE;
        mCalls[i].numType = BTHF_CALL_ADDRTYPE_INTERNATIONAL; // TODO: cross check
        strncpy(mCalls[i].number, call_record->number, sizeof(call_record->number));

        if (mCalls[i].stat == BTHF_CALL_STATE_INCOMING ||
             mCalls[i].stat == BTHF_CALL_STATE_WAITING)
           mRingingAddress = mCalls[i].number;
        i++;
    }
}

int Hfp_Ag::get_current_calls() {
   int ret_val = 0;

   mcm_voice_get_calls_req_msg_v01         get_calls_req_msg;
   mcm_voice_get_calls_resp_msg_v01        get_calls_resp_msg;

   memset(&get_calls_req_msg, 0, sizeof(get_calls_req_msg));
   memset(&get_calls_resp_msg, 0, sizeof(get_calls_resp_msg));

   ret_val = mcm_client_execute_command_sync_ptr(mcm_client_hdl,
                                              MCM_VOICE_GET_CALLS_REQ_V01,
                                              &get_calls_req_msg,
                                              sizeof(get_calls_req_msg),
                                              &get_calls_resp_msg,
                                              sizeof(get_calls_resp_msg));
   if (ret_val == MCM_SUCCESS_V01) {
       ALOGD(LOGTAG, "getting current call list successful");
       // store the call information
       process_call_list(get_calls_resp_msg.calls, get_calls_resp_msg.calls_len);
   }
   else
       ALOGE(LOGTAG, "getting current call list failed");

   return ret_val;
}

void Hfp_Ag::dial_call(char *number, bt_bdaddr_t *bd_addr) {
    if (mDiallingOut == true ||
        (strlen(number) == 0) && (strlen(mLastDialledNumber) == 0) ||
         number[0] == '>' ||
         strlen(number) > MCM_MAX_PHONE_NUMBER_V01) {
        ALOGE(LOGTAG, "MO call in progress, or number not available of redial or memory dialling not supported");
        fprintf(stdout, "MO call in progress, or number not available of redial or memory dialling not supported\n");
        // if MO call is already being initiated, send error
        // if it is redial request and we don't have last dialled number, send error
        // if memory dialling is requested, send error
        if (sBtHfpAgInterface != NULL)
            sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, bd_addr);
        return;
    }

    uint32 ret_val = 0;
    mcm_voice_dial_req_msg_v01   dial_req;
    memset(&dial_req, 0, sizeof(mcm_voice_dial_req_msg_v01));
    memset(&dial_resp, 0, sizeof(mcm_voice_dial_resp_msg_v01));

    dial_req.address_valid = true;

    // redial request
    if (strlen(number) == 0)
        strncpy(dial_req.address, mLastDialledNumber, strlen(mLastDialledNumber));
    else {
        // remove trailing ';' if present
        if (number[strlen(number) - 1] == ';')
            strncpy(dial_req.address, number, strlen(number) - 1);
        else
            strncpy(dial_req.address, number, strlen(number));
    }

    ret_val = mcm_client_execute_command_async_ptr(mcm_client_hdl,
                                                   MCM_VOICE_DIAL_REQ_V01,
                                                   &dial_req,
                                                   sizeof(dial_req),
                                                   &dial_resp,
                                                   sizeof(dial_resp),
                                                   ril_resp_cb,
                                                   &token_id);

    if (ret_val != MCM_SUCCESS_V01) {
        ALOGE(LOGTAG, "sending dial command failed");
        fprintf(stdout, LOGTAG "sending dial command failed\n");
        // send error if dial fails
        if (sBtHfpAgInterface != NULL)
            sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, bd_addr);
        return;
    }

    mDiallingOut = true;
    // store the last dialled number
    if (number[strlen(number) - 1] == ';')
        strncpy(dial_req.address, number, strlen(number) - 1);
    else
        strncpy(dial_req.address, number, strlen(number));
}

uint32 Hfp_Ag::send_voice_cmd(mcm_voice_call_operation_t_v01 op) {

    uint32 ret_val = 0;
    mcm_voice_command_req_msg_v01    voice_cmd_req;

    memset(&voice_cmd_req, 0, sizeof(mcm_voice_command_req_msg_v01));
    memset(&voice_cmd_resp, 0, sizeof(mcm_voice_command_resp_msg_v01));

    voice_cmd_req.call_operation = op;
    ret_val = mcm_client_execute_command_async_ptr(mcm_client_hdl,
                                                   MCM_VOICE_COMMAND_REQ_V01,
                                                   &voice_cmd_req,
                                                   sizeof(voice_cmd_req),
                                                   &voice_cmd_resp,
                                                   sizeof(voice_cmd_resp),
                                                   ril_resp_cb,
                                                   &token_id);

   if (ret_val == MCM_SUCCESS_V01) {
       // save operation to process the response call back
       call_op = op;
   }
   else {
        ALOGE(LOGTAG, "sending %d command failed", op);
        fprintf(stdout, "sending %d command failed\n", op);
   }

   return ret_val;
}

uint32 Hfp_Ag::get_call_id(bthf_call_state_t state){
    for (uint8 i = 0; i < MCM_MAX_VOICE_CALLS_V01; i++) {
        if (mCalls[i].call_id != 0xFFFFFFFF && mCalls[i].stat == state)
            return mCalls[i].call_id;
    }
    return 0xFFFFFFFF;
}

uint32 Hfp_Ag::end_call(bthf_call_state_t state) {

    uint32 ret_val = 0, call_id = 0;
    mcm_voice_hangup_req_msg_v01 hangup_req;

    call_id = get_call_id(state);
    if (call_id == 0xFFFFFFFF) {
        ALOGE(LOGTAG, "%s: No calls in state %u to hangup, returning", __func__, state);
        fprintf(stdout, "%s: No calls in state %u to hangup, returning\n", __func__, state);
        return MCM_ERROR_GENERIC_V01;
    }

    memset(&hangup_req, 0, sizeof(mcm_voice_hangup_req_msg_v01));
    memset(&hangup_resp, 0, sizeof(mcm_voice_hangup_resp_msg_v01 ));

    hangup_req.call_id = call_id;
    ret_val = mcm_client_execute_command_async_ptr(mcm_client_hdl,
                                                   MCM_VOICE_HANGUP_REQ_V01,
                                                   &hangup_req,
                                                   sizeof(hangup_req),
                                                   &hangup_resp,
                                                   sizeof(hangup_resp),
                                                   ril_resp_cb,
                                                   &token_id);

   if (ret_val != MCM_SUCCESS_V01) {
        ALOGE(LOGTAG, "sending hangup command failed");
        fprintf(stdout, LOGTAG "sending hangup command failed\n");
   }

   return ret_val;
}

uint32 Hfp_Ag::process_chld(int chld) {
   uint32 ret_val = MCM_ERROR_GENERIC_V01;

   switch (chld) {
       case BTHF_CHLD_TYPE_RELEASEHELD:
          if (mNumRingingCalls > 0) {
              // state BTHF_CALL_STATE_INCOMING for incoming/waiting calls
              ret_val = end_call(BTHF_CALL_STATE_INCOMING);
          }
          else if(mNumHeldCalls > 0)
              ret_val = end_call(BTHF_CALL_STATE_HELD);
       break;
       case BTHF_CHLD_TYPE_RELEASEACTIVE_ACCEPTHELD:
          if (mNumActiveCalls == 0 && mNumHeldCalls == 0 && mNumRingingCalls == 0)
              break;

          if (mNumActiveCalls > 0)
              ret_val = end_call(BTHF_CALL_STATE_ACTIVE);

          if (mNumRingingCalls > 0)
              ret_val = send_voice_cmd(MCM_VOICE_CALL_ANSWER_V01);
          else if (mNumHeldCalls > 0)
              ret_val = send_voice_cmd(MCM_VOICE_CALL_UNHOLD_V01);
       break;
       case BTHF_CHLD_TYPE_HOLDACTIVE_ACCEPTHELD:
          if (mNumActiveCalls > 0)
              ret_val = send_voice_cmd(MCM_VOICE_CALL_HOLD_V01);

          if (mNumRingingCalls > 0)
              ret_val = send_voice_cmd(MCM_VOICE_CALL_ANSWER_V01);
          else if (mNumHeldCalls > 0)
              ret_val = send_voice_cmd(MCM_VOICE_CALL_UNHOLD_V01);
       break;
       case BTHF_CHLD_TYPE_ADDHELDTOCONF:
          // TODO: cross check on this. There is no way to know from RIL if a call is in conference
          if (mNumActiveCalls > 0 && mNumHeldCalls > 0)
             ret_val = send_voice_cmd(MCM_VOICE_CALL_CONFERENCE_V01);
       break;
       default:
          ALOGE(LOGTAG, "unhandled chld command %d", chld);
          fprintf(stdout, "unhandled chld command %d\n", chld);
       break;
   }
   return ret_val;
}

void Hfp_Ag::process_ril_ind(BtEvent* pEvent){
   if (pEvent->hfp_ag_event.hdl != mcm_client_hdl) {
       fprintf(stdout, "invalid mcm client handle, returning\n");
       ALOGE(LOGTAG, "%s: invalid mcm client handle, returning\n");
       return;
   }

   switch(pEvent->hfp_ag_event.msg_id){
      case MCM_VOICE_CALL_IND_V01:
      {
          mcm_voice_call_ind_msg_v01 *call_ind =
                   (mcm_voice_call_ind_msg_v01 *)(pEvent->hfp_ag_event.data);
          // TODO: save the existing active, held and ringing call info before updating them.
          // Send the information to stack only when there is a change in the call info
          if (call_ind != NULL) {
              process_call_list(call_ind->calls, call_ind->calls_len);
              // for MO call initiated from BT headset, send OK to headset
              if (mDiallingOut == true && mCallSetupState == BTHF_CALL_STATE_DIALING) {
                  mDiallingOut = false;
                  sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_OK, 0,
                                                 &pEvent->hfp_ag_event.bd_addr);
              }
              //  update the calls info to stack
              sBtHfpAgInterface->phone_state_change(mNumActiveCalls,
                     mNumHeldCalls,
                     mCallSetupState,
                     mRingingAddress == NULL ? "" : mRingingAddress,
                     BTHF_CALL_ADDRTYPE_INTERNATIONAL);
          }
      }
          break;
      case MCM_NW_SIGNAL_STRENGTH_EVENT_IND_V01:
      {
          mcm_nw_signal_strength_event_ind_msg_v01 *network_ss_ind_msg =
                      (mcm_nw_signal_strength_event_ind_msg_v01 *)(pEvent->hfp_ag_event.data);
      }
          break;
      case MCM_VOICE_MUTE_IND_V01:
      // intentional fall through
      case MCM_VOICE_DTMF_IND_V01:
      // intentional fall through
      default:
          fprintf(stdout, "unhandled indication\n", pEvent->hfp_ag_event.msg_id);
          ALOGD(LOGTAG, "unhandled indication %u\n", pEvent->hfp_ag_event.msg_id);
          break;
   }
}

void Hfp_Ag::process_ril_resp(BtEvent* pEvent){
   if (pEvent->hfp_ag_event.hdl != mcm_client_hdl) {
       fprintf(stdout, "invalid mcm client handle, returning\n");
       ALOGE(LOGTAG, "%s: invalid mcm client handle, returning\n");
       return;
   }

   switch(pEvent->hfp_ag_event.msg_id){
      case MCM_VOICE_DIAL_RESP_V01:
      {
          mcm_voice_dial_resp_msg_v01 *dial_resp = (mcm_voice_dial_resp_msg_v01*)pEvent->hfp_ag_event.data;
          mcm_response_t_v01 *resp = &dial_resp->response;

          // if dial request failed, send error to remote
          if (resp->result != MCM_RESULT_SUCCESS_V01) {
              ALOGE(LOGTAG, "dialling call failed with error %d", resp->error);
              fprintf(stdout, "dialling call failed with error %d\n", resp->error);
              if (sBtHfpAgInterface != NULL)
                  sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_ERROR, 0, &pEvent->hfp_ag_event.bd_addr);
          }
      }
          break;
      case MCM_VOICE_COMMAND_RESP_V01:
      {
          mcm_voice_command_resp_msg_v01 *cmd_resp = (mcm_voice_command_resp_msg_v01 *)pEvent->hfp_ag_event.data;
          mcm_response_t_v01 *resp = &cmd_resp->response;

          // if voice command request failed, send error to remote
          if (resp->result != MCM_RESULT_SUCCESS_V01) {
              ALOGE(LOGTAG, "voice command %d failed with error %d", call_op, resp->error);
              fprintf(stdout, "voice command %d failed with error %d\n", call_op, resp->error);
          }
      }
          break;
      case MCM_VOICE_HANGUP_RESP_V01:
      {
          mcm_voice_hangup_resp_msg_v01 *hangup_resp = (mcm_voice_hangup_resp_msg_v01 *)pEvent->hfp_ag_event.data;
          mcm_response_t_v01 *resp = &hangup_resp->response;

          // if hangup request failed, send error to remote
          if (resp->result != MCM_RESULT_SUCCESS_V01) {
              ALOGE(LOGTAG, "ending call failed with error %d", resp->error);
              fprintf(stdout, "ending call failed with error %d\n", resp->error);
          }
      }
          break;
      default:
          ALOGE(LOGTAG, "unhandled response %d", pEvent->hfp_ag_event.msg_id);
          fprintf(stdout, "unhandled response %d\n", pEvent->hfp_ag_event.msg_id);
          break;
   }
}

void Hfp_Ag::init_modem() {
   lib_handle = NULL;
   mcm_client_hdl = 0;
   mcm_client_init_ptr = NULL;
   mcm_client_release_ptr = NULL;
   mcm_client_execute_command_async_ptr = NULL;
   mcm_client_execute_command_sync_ptr= NULL;

   // reset call related information
   mDiallingOut = false;
   mNumActiveCalls = 0;
   mNumHeldCalls = 0;
   mNumRingingCalls = 0;
   mSignalStrength = 0;
   mCallSetupState = BTHF_CALL_STATE_IDLE;
   memset(mLastDialledNumber, 0, sizeof(mLastDialledNumber));
   memset(mCalls, 0, sizeof(mCalls));

   // call_id should be invalidated
   for (int i = 0; i < MCM_MAX_VOICE_CALLS_V01; i++) {
       mCalls[i].call_id = 0xFFFFFFFF;
       mCalls[i].idx = 0;
   }

   lib_handle = dlopen(MCM_LIBRARY_NAME, RTLD_NOW);

   if (!lib_handle) {
      ALOGE(LOGTAG, "%s unable to open %s: %s", __func__, MCM_LIBRARY_NAME, dlerror());
      return;
   }

   mcm_client_init_ptr = (mcm_client_init_t)dlsym(lib_handle, "mcm_client_init");
   // TODO: handle error
   if (mcm_client_init_ptr == NULL) {
       ALOGE(LOGTAG, "unable to find mcm_client_init symbol");
   }

   mcm_client_release_ptr = (mcm_client_release_t)dlsym(lib_handle, "mcm_client_release");
   // TODO: handle error
   if (mcm_client_release_ptr == NULL) {
       ALOGE(LOGTAG, "unable to find mcm_client_release symbol");
   }
   mcm_client_execute_command_async_ptr =
         (mcm_client_execute_command_async_t)dlsym(lib_handle,
                                           "mcm_client_execute_command_async");
   // TODO: handle error
   if (mcm_client_execute_command_async_ptr == NULL) {
       ALOGE(LOGTAG, "unable to find mcm_client_execute_command_async symbol");
   }
   mcm_client_execute_command_sync_ptr =
         (mcm_client_execute_command_sync_t)dlsym(lib_handle,
                                          "mcm_client_execute_command_sync");
   // TODO: handle error
   if (mcm_client_execute_command_sync_ptr == NULL) {
       ALOGE(LOGTAG, "unable to find mcm_client_execute_command_sync symbol");
   }

   mcm_client_init_ptr(&mcm_client_hdl, ril_ind_cb, ril_resp_cb);

   mcm_voice_event_register_req_msg_v01   req_msg;
   mcm_voice_event_register_resp_msg_v01  resp_msg;

   memset(&req_msg, 0, sizeof(req_msg));
   memset(&resp_msg, 0, sizeof(resp_msg));

   req_msg.register_voice_call_event_valid = TRUE;
   req_msg.register_voice_call_event       = TRUE;

   int ret_val = MCM_ERROR_GENERIC_V01;

   ret_val = mcm_client_execute_command_sync_ptr(mcm_client_hdl,
                                             MCM_VOICE_EVENT_REGISTER_REQ_V01,
                                             &req_msg,
                                             sizeof(req_msg),
                                             &resp_msg,
                                             sizeof(resp_msg));

   if (ret_val == MCM_SUCCESS_V01)
       ALOGD(LOGTAG, "registration of voice indications successful");
   else
       ALOGE(LOGTAG, "registration of voice indications failed");

   // register for signal strength indications
   mcm_nw_event_register_req_msg_v01 nw_evt_req_msg;
   mcm_nw_event_register_resp_msg_v01 nw_evt_resp_msg;
   memset(&nw_evt_req_msg, 0, sizeof(nw_evt_req_msg));
   memset(&nw_evt_resp_msg, 0, sizeof(nw_evt_resp_msg));

   nw_evt_req_msg.register_voice_registration_event_valid = TRUE;
   nw_evt_req_msg.register_voice_registration_event = TRUE;

   nw_evt_req_msg.register_signal_strength_event_valid = TRUE;
   nw_evt_req_msg.register_signal_strength_event = TRUE;

   ret_val = mcm_client_execute_command_sync_ptr(mcm_client_hdl,
                                              MCM_NW_EVENT_REGISTER_REQ_V01,
                                              &nw_evt_req_msg,
                                              sizeof(nw_evt_req_msg),
                                              &nw_evt_resp_msg,
                                              sizeof(nw_evt_resp_msg));
   if (ret_val == MCM_SUCCESS_V01)
       ALOGD(LOGTAG, "registration of network indications successful");
   else
       ALOGE(LOGTAG, "registration of network indications failed");

   // get current call list to update the headset
   ret_val = get_current_calls();

}

void Hfp_Ag::release_modem() {
   if (mcm_client_hdl)
       mcm_client_release_ptr(mcm_client_hdl);

   if (lib_handle)
       dlclose(lib_handle);
}
#endif

#if defined(BT_ALSA_AUDIO_INTEGRATION)
void Hfp_Ag::init_audio() {
   char cmd[50];
   mWbsState = BTHF_WBS_NO;
   mNrec = BTHF_NREC_STOP;

   // set up voice path using amix commands
   strcpy(cmd, "amix \'SEC_AUX_PCM_RX_Voice Mixer CSVoice\' 1");
   system(cmd);

   strcpy(cmd, "amix \'Voice_Tx Mixer SEC_AUX_PCM_TX_Voice\' 1");
   system(cmd);
}

void Hfp_Ag::set_audio_params() {
   char cmd[50];

   ALOGD(LOGTAG, "%s: setting sample rate %s\n", __func__,
             (mWbsState == BTHF_WBS_YES ? "16000" : "8000"));
   fprintf(stdout, "%s: setting sample rate %s\n", __func__,
             (mWbsState == BTHF_WBS_YES ? "16000" : "8000"));

   // set sample rate using amix commands
   if (mWbsState == BTHF_WBS_YES)
       strcpy(cmd, "amix \'AUX PCM SampleRate\' \'rate_16000\'");
   else
       strcpy(cmd, "amix \'AUX PCM SampleRate\' \'rate_8000\'");
   system(cmd);
}

void Hfp_Ag::setup_sco_path() {
   char cmd[50];

   ALOGD(LOGTAG, "%s: starting arec and aplay\n", __func__);
   fprintf(stdout, "%s: starting arec and aplay\n", __func__);

   // set sample rate before starting sco
   set_audio_params();

   // start sco using aplay and arec commands
   sprintf(cmd, "aplay -D hw:0,2 -P -R%s -C 1 &",
              (mWbsState == BTHF_WBS_YES ? "16000" : "8000"));
   system(cmd);

   sprintf(cmd, "arec -D hw:0,2 -P -R%s -C 1 &",
              (mWbsState == BTHF_WBS_YES ? "16000" : "8000"));
   system(cmd);
}

void Hfp_Ag::teardown_sco_path() {
    char cmd[50];

    ALOGD(LOGTAG, "%s: killing arec and aplay\n", __func__);
    fprintf(stdout, "%s: killing arec and aplay\n", __func__);

    strcpy(cmd, "killall -9 arec");
    system(cmd);

    strcpy(cmd, "killall -9 aplay");
    system(cmd);

}

void Hfp_Ag::release_audio() {
   char cmd[50];

   // set up voice path using amix commands
   strcpy(cmd, "amix \'SEC_AUX_PCM_RX_Voice Mixer CSVoice\' 0");
   system(cmd);

   strcpy(cmd, "amix \'Voice_Tx Mixer SEC_AUX_PCM_TX_Voice\' 0");
   system(cmd);
}

#endif

void Hfp_Ag::process_at_bind(BtEvent* pEvent) {
   char *hf_ind, *str1, *str2;
   int i = 0, type = pEvent->hfp_ag_event.arg1;

   hf_ind = pEvent->hfp_ag_event.str;
   ALOGD(LOGTAG " %s: str is %s, type is %d", __func__, hf_ind, type);

   if (type == 0) {
       str1 = hf_ind;
       for (i = 0; i < MAX_HF_INDICATORS; i++) {
          mHfIndHfList[i] = (int)strtol(str1, &str2, 0);

          // move ahead in the string if char is not ',' or a digit
          while(*str2 != ',' && *str2 != '\0' && !(*str2 >= '0' && *str2 <= '9'))
              str2++;

          // if headset does not support all indicators, break
          if (*str2 == '\0')
              break;

          if (*str2 == ',')
             str2++;

          str1 = str2;
      }
   }
   else if(type == 1) {
      if (sBtHfpAgVendorInterface != NULL) {
          for (i = 0; i < MAX_HF_INDICATORS;i++) {
              // TODO: send all the indicators as disabled for now
              sBtHfpAgVendorInterface->
                  bind_response_vendor(i+1, BTHF_VENDOR_HF_INDICATOR_STATE_DISABLED,
                  &pEvent->hfp_ag_event.bd_addr);
          }
      }
      if (sBtHfpAgInterface != NULL) {
          sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_OK, 0,
                      &pEvent->hfp_ag_event.bd_addr);
      }
   }
   else if(type == 2) {
       char str[256] = "(", temp_str[5];

       for(int i = 0; i < MAX_HF_INDICATORS; i++) {
           sprintf(temp_str, "%d,", i+1);
           strcat(str, temp_str);
       }
       str[strlen(str) - 1] = ')';

       if (sBtHfpAgVendorInterface != NULL) {
          sBtHfpAgVendorInterface->bind_string_response_vendor(str, &pEvent->hfp_ag_event.bd_addr);
       }
       if (sBtHfpAgInterface != NULL) {
          sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_OK, 0,
                      &pEvent->hfp_ag_event.bd_addr);
       }
   }
}

void Hfp_Ag::process_at_biev(BtEvent* pEvent) {
    // TODO: just send OK for now
    if (sBtHfpAgInterface != NULL) {
        sBtHfpAgInterface->at_response(BTHF_AT_RESPONSE_OK, 0,
                    &pEvent->hfp_ag_event.bd_addr);
    }
}

void Hfp_Ag::change_state(HfpAgState mState) {
   ALOGD(LOGTAG " current State = %d, new state = %d", mAgState, mState);
   pthread_mutex_lock(&lock);
   mAgState = mState;
   ALOGD(LOGTAG " state changes to %d ", mAgState);
   pthread_mutex_unlock(&lock);

   // reset variables when we enter into disconnected state
   if (mState == HFP_AG_STATE_DISCONNECTED) {
       mWbsState = BTHF_WBS_NO;
       mNrec = BTHF_NREC_STOP;
   }
}

Hfp_Ag :: Hfp_Ag(const bt_interface_t *bt_interface, config_t *config) {
    this->bluetooth_interface = bt_interface;
    sBtHfpAgInterface = NULL;
    mAgState = HFP_AG_STATE_NOT_STARTED;
    mcontrolStatus = STATUS_LOSS_TRANSIENT;
    pthread_mutex_init(&this->lock, NULL);

    memset(mHfIndHfList, 0, sizeof(mHfIndHfList));
    memset(mHfIndAgList, 0, sizeof(mHfIndAgList));
}

Hfp_Ag :: ~Hfp_Ag() {
    mcontrolStatus = STATUS_LOSS_TRANSIENT;
    pthread_mutex_destroy(&lock);
}

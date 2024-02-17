/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
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

#ifndef HFP_AG_APP_H
#define HFP_AG_APP_H

#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_hf.h>
#include <pthread.h>

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "ipc.h"
#include "utils.h"
#include "hardware/bt_hf_vendor.h"

// for MDM, define, this, TODO: move it to bitbake
//#define BT_ALSA_AUDIO_INTEGRATION 0

#if defined(BT_MODEM_INTEGRATION)
#include <dlfcn.h>
#include "mcm_client.h"
#include "mcm_voice_v01.h"
#include "mcm_sim_v01.h"
#include "mcm_nw_v01.h"

typedef  uint32 (*mcm_client_init_t)
(
    mcm_client_handle_type      *client_handle,
    mcm_client_ind_cb            client_ind_cb,
    mcm_client_async_cb          client_resp_cb
);

typedef  uint32 (*mcm_client_release_t)(mcm_client_handle_type handle);

typedef  uint32 (*mcm_client_execute_command_async_t)
(
    mcm_client_handle_type        client_handle,
    int                           msg_id,
    void                         *req_c_struct,
    int                           req_c_struct_len,
    void                         *resp_c_struct,
    int                           resp_c_struct_len,
    mcm_client_async_cb           async_resp_cb,
    void                          *token_id
);

typedef  uint32 (*mcm_client_execute_command_sync_t)
(
    mcm_client_handle_type      client_handle,
    int                         msg_id,
    void                       *req_c_struct,
    int                         req_c_struct_len,
    void                       *resp_c_struct,
    int                         resp_c_struct_len
);
#endif

// currently only 2 HF indicators are defined by SIG
#define MAX_HF_INDICATORS 2


typedef enum {
    HFP_AG_STATE_NOT_STARTED = 0,
    HFP_AG_STATE_DISCONNECTED,
    HFP_AG_STATE_PENDING,
    HFP_AG_STATE_CONNECTED,
    HFP_AG_STATE_AUDIO_ON
}HfpAgState;

#if defined(BT_MODEM_INTEGRATION)

typedef struct {
    uint32_t    call_id; // call id to identify this call
    uint8_t     idx;
    bthf_call_direction_t     dir; // 0=outgoing, 1=incoming
    bthf_call_state_t     stat; // 0-6
    bthf_call_mode_t     mode; // 0=voice, 1=data, 2=fax
    bthf_call_mpty_type_t     mpty; // 0=no, 1=yes
    bthf_call_addrtype_t     numType;
    char        number[MCM_MAX_PHONE_NUMBER_V01 + 1]; // remote's number
}call_info;


    void ril_ind_cb(mcm_client_handle_type hndl, uint32 msg_id,
                     void *ind_c_struct, uint32 ind_len);
    void ril_resp_cb(mcm_client_handle_type hndl, uint32 msg_id,
                     void *resp_c_struct, uint32 resp_len, void *token_id);
#endif

class Hfp_Ag {

  private:
#if defined(BT_MODEM_INTEGRATION)
    bool mDiallingOut;
    uint8_t mNumActiveCalls;
    uint8_t mNumHeldCalls;
    uint8_t mNumRingingCalls;
    bthf_call_state_t mCallSetupState;
    int mSignalStrength;
    char *mRingingAddress;

    char mLastDialledNumber[MCM_MAX_PHONE_NUMBER_V01 + 1];
    call_info mCalls[MCM_MAX_VOICE_CALLS_V01];

    mcm_client_handle_type          mcm_client_hdl;
    int                             token_id;
    mcm_voice_call_operation_t_v01  call_op;
    uint32_t                        call_id;

    mcm_voice_command_resp_msg_v01                  voice_cmd_resp;
    mcm_voice_hangup_resp_msg_v01                   hangup_resp;
    mcm_voice_dial_resp_msg_v01                     dial_resp;
    mcm_voice_get_calls_resp_msg_v01                get_calls_resp;
    mcm_voice_event_register_resp_msg_v01           voice_event_register_resp;
    mcm_voice_set_call_waiting_resp_msg_v01         set_cw_resp;
    mcm_voice_dtmf_resp_msg_v01                     dtmf_resp;
    mcm_voice_start_dtmf_resp_msg_v01               start_dtmf_resp;
    mcm_voice_stop_dtmf_resp_msg_v01                stop_dtmf_resp;
    mcm_sim_get_device_phone_number_resp_msg_v01    get_phone_num_resp;
    mcm_nw_get_operator_name_resp_msg_v01           cops_resp;

    // handle to mcm library
    void *lib_handle;
    // function pointer to mcm_client_init
    mcm_client_init_t mcm_client_init_ptr;

    // function pointer to mcm_client_release
    mcm_client_release_t mcm_client_release_ptr;

    // function pointer to mcm_client_execute_command_async
    mcm_client_execute_command_async_t mcm_client_execute_command_async_ptr;

    // function pointer to mcm_client_execute_command_sync
    mcm_client_execute_command_sync_t mcm_client_execute_command_sync_ptr;

#endif

    int mHfIndHfList[MAX_HF_INDICATORS];
    // index 0 is for 1st assigned number, index 1 for 2nd assigned number etc
    int mHfIndAgList[MAX_HF_INDICATORS];
    const bt_interface_t * bluetooth_interface;
    const bthf_interface_t *sBtHfpAgInterface;
    HfpAgState mAgState;
    ControlStatusType mcontrolStatus;
    bthf_wbs_config_t mWbsState;
    bthf_nrec_t mNrec;
    const bthf_vendor_interface_t *sBtHfpAgVendorInterface;
  public:
    Hfp_Ag(const bt_interface_t *bt_interface, config_t *config);
    ~Hfp_Ag();
    void ProcessEvent(BtEvent* pEvent);
    void state_disconnected_handler(BtEvent* pEvent);
    void state_pending_handler(BtEvent* pEvent);
    void state_connected_handler(BtEvent* pEvent);
    void state_audio_on_handler(BtEvent* pEvent);
    void change_state(HfpAgState mState);
    pthread_mutex_t lock;
    bt_bdaddr_t mConnectingDevice;
    bt_bdaddr_t mConnectedDevice;
    void HandleEnableAg();
    void HandleDisableAg();
    void ConfigureAudio(bool enable);
    void process_at_bind(BtEvent* pEvent);
    void process_at_biev(BtEvent* pEvent);
#if defined(BT_MODEM_INTEGRATION)
    void init_modem();
    void release_modem();
    bthf_call_state_t get_call_state(mcm_voice_call_state_t_v01 state);
    void process_call_list(mcm_voice_call_record_t_v01 *calls, uint32_t num_calls);
    int get_current_calls();
    void get_and_send_operator_name(bt_bdaddr_t *bd_addr);
    void get_and_send_subscriber_number(bt_bdaddr_t *bd_addr);
    void dial_call(char *number, bt_bdaddr_t *bd_addr);
    uint32 send_voice_cmd(mcm_voice_call_operation_t_v01 op);
    uint32 end_call(bthf_call_state_t state);
    uint32 get_call_id(bthf_call_state_t state);
    uint32 process_chld(int chld);
    void process_ril_ind(BtEvent* pEvent);
    void process_ril_resp(BtEvent* pEvent);
    void processSlcConnected();
#endif

#if defined(BT_ALSA_AUDIO_INTEGRATION)
    void init_audio();
    void set_audio_params();
    void setup_sco_path();
    void teardown_sco_path();
    void release_audio();
#endif
};

#endif

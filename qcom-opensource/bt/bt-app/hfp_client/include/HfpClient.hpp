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

#ifndef HFP_CLIENT_APP_H
#define HFP_CLIENT_APP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_hf_client.h>
#include <pthread.h>

#if defined(USE_GST)
#include <gst/gst.h>
#endif

#if defined(BT_AUDIO_HAL_INTEGRATION)

#include <hardware/audio.h>
#include <hardware/hardware.h>
#endif

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "osi/include/allocator.h"
#include "ipc.h"
#include "utils.h"
#include "hardware/bt_hf_client_vendor.h"

#include "Audio_Manager.hpp"

typedef enum {
    HFP_CLIENT_STATE_NOT_STARTED = 0,
    HFP_CLIENT_STATE_DISCONNECTED,
    HFP_CLIENT_STATE_CONNECTING,
    HFP_CLIENT_STATE_CONNECTED,
    HFP_CLIENT_STATE_AUDIO_ON
}HfpClientState;

typedef enum {
    HFP_CLIENT_MODE_NORMAL,
    HFP_CLIENT_MODE_RINGTONE,
    HFP_CLIENT_MODE_IN_CALL
}HfpClientMode;

class Hfp_Client {

  private:
    bool mAudioWbs;
    unsigned int peer_feat;
    unsigned int chld_feat;
#if defined(BT_AUDIO_HAL_INTEGRATION)
    config_t *config;
    qahw_stream_handle_t* out_stream;
    qahw_stream_handle_t* out_stream_ring_tone;
#endif
    const bt_interface_t * bluetooth_interface;
    const bthf_client_interface_t *sBtHfpClientInterface;
    HfpClientState mClientState;
    HfpClientMode mAudioMode;
    ControlStatusType mcontrolStatus;
    const bthf_client_vendor_interface_t *sBtHfpClientVendorInterface;

  public:
    Hfp_Client(const bt_interface_t *bt_interface, config_t *config);
    ~Hfp_Client();
    void ProcessEvent(BtEvent* pEvent);
    void state_disconnected_handler(BtEvent* pEvent);
    void state_connecting_handler(BtEvent* pEvent);
    void state_connected_handler(BtEvent* pEvent);
    void state_audio_on_handler(BtEvent* pEvent);
    void change_state(HfpClientState mState);
    void change_mode(HfpClientMode mode);
    pthread_mutex_t lock;
    bt_bdaddr_t mConnectingDevice;
    bt_bdaddr_t mConnectedDevice;
    void HandleEnableClient();
    void HandleDisableClient();
    void ConfigureAudio(bool enable);
    void ConfigureRingTonePlayback();
    void ConfigureVolume(bthf_client_volume_type_t vol_type, int vol, bool mute_mic);
    void PlayRingTone();
    void StopRingTone();
};

#endif

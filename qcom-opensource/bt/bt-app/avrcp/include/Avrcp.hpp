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

#ifndef AVRCP_APP_H
#define AVRCP_APP_H

#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_rc.h>
#include <pthread.h>

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "osi/include/allocator.h"
#include "ipc.h"
#include "utils.h"
#include "hardware/bt_rc_vendor.h"
#include <list>

class Avrcp {

  private:
    config_t *config;
    const bt_interface_t * bluetooth_interface;
    const btrc_ctrl_interface_t *sBtAvrcpCtrlInterface;
    const btrc_ctrl_vendor_interface_t *sBtAvrcpCtrlVendorInterface;
    int mPreviousPercentageVol;
    bool mFirstAbsVolCmdRecvd;
  public:
    Avrcp(const bt_interface_t *bt_interface, config_t *config);
    ~Avrcp();
    char* dump_message(BluetoothEventId event_id);
    pthread_mutex_t lock;
    std::list <std::string> rc_only_devices;
    bt_bdaddr_t mConnectedAvrcpDevice;
    uint32_t max_avrcp_conn;
    void HandleAvrcpCTEvents(BtEvent* pEvent);
    void HandleAvrcpCTPassThruEvents(BtEvent* pEvent);
    void HandleEnableAvrcp();
    void HandleDisableAvrcp();
    void SendPassThruCommandNative(uint8_t key_id, bt_bdaddr_t* addr, uint8_t direct);
    void OnDisconnected();
    void setAbsVolume(bt_bdaddr_t* dev, int absVol, int label);
    bool is_abs_vol_supported(bt_bdaddr_t bd_addr);
    int get_current_audio_index();
};

#endif

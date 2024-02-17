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

#ifndef SDP_CLIENT_H
#define SDP_CLIENT_H

#pragma once
#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_sdp.h>
#include <stdio.h>
#include <string.h>

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "osi/include/alarm.h"
#include "ipc.h"

#define SDP_SEARCH_TIMEOUT_DELAY     (6000)

class SdpClient {
    private:
        const bt_interface_t * bluetooth_interface;
        const btsdp_interface_t *sdp_client_interface;
        bt_bdaddr_t mDevice;
        config_t *config;
        bool Search(bt_bdaddr_t *addr, uint8_t *uuid, SdpSearchCb cb);
        void AddRecord(bluetooth_sdp_record *record, SdpAddRecordCb cb);
        void RemoveRecord(int record_handle, SdpRemoveRecordCb cb);

    public:
        SdpClient(const bt_interface_t *bt_interface, config_t *config);
        ~SdpClient();
        alarm_t *sdp_search_timer;
        bool HandleEnableSdpClient();
        bool HandleDisableSdpClient();
        void ProcessEvent(BtEvent* pEvent);
};

#endif


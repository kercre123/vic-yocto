/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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

#ifndef OPP_APP_H
#define OPP_APP_H

#pragma once
#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_sock.h>
#include <hardware/bt_sdp.h>
#include <stdio.h>
#include <string.h>

#include "osi/include/alarm.h"
#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "ipc.h"

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif

extern const char *BT_OPP_ENABLED;
#define OPP_CONNECT_TIMEOUT_DELAY     (30000)

class Opp {
    private:
        const bt_interface_t * bluetooth_interface;
        config_t *config;

        void AddSdpRecord();
        bool PerformSdp(bt_bdaddr_t *addr);
        bool SendFile(bt_bdaddr_t *addr, char * fileName);
        bool Connect();
        bool SendData();
        bool HandleConnectTimeout(bt_bdaddr_t *addr);
        bool Disconnect();
        bool Abort();
        bool IncomingFileRsp(bool accept);

    public:
        Opp(const bt_interface_t *bt_interface, config_t *config);
        ~Opp();
        alarm_t *opp_connect_timer;
        void ProcessEvent(BtEvent* pEvent);
        void RemoveSdpRecord();
};

#endif


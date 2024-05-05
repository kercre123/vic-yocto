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

#ifndef PAN_APP_H
#define PAN_APP_H

#pragma once
#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_pan.h>
#include <stdio.h>
#include <string.h>

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "ipc.h"

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif

#define MAX_LENGTH_INTERFACE_NAME 10
#define MAX_PAN_DEVICES 3

#define PAN_ROLE_NONE 0
/**
 * The local device is acting as a Network Access Point.
 */
#define LOCAL_NAP_ROLE 1
#define REMOTE_NAP_ROLE 1

/**
 * The local device is acting as a PAN User.
 */
#define LOCAL_PANU_ROLE 2
#define REMOTE_PANU_ROLE 2

/**
 * The Configuration options
 */
extern const char *BT_PAN_NAP_ROLE_SUPPORTED;
extern const char *BT_PAN_PANU_ROLE_SUPPORTED;
extern const char *BT_PAN_ENABLED;

typedef struct
{
    int state;
    int prev_state;
    int local_role;
    int remote_role;
    bt_bdaddr_t bd_addr;
} pan_device_t;

typedef enum
{
    TETHERED,
    REVERSE_TETHERED,
    UNTETHERED,
    PENDING
} pan_profile_state_t;

class Pan {
    private:
        config_t *config;
        const bt_interface_t * bluetooth_interface;
        const btpan_interface_t *pan_interface;
        bool is_nap_role_supported;
        bool is_panu_role_supported;
        char pan_interface_name[10];
        bool is_tethering_on;
        pan_profile_state_t pan_state;
        int num_of_pan_device_connected;
        pan_device_t pan_device[MAX_PAN_DEVICES];

        void HandlePanControlStateEvent(PanControlStateEvent *event);
        void HandlePanConnectionStateEvent(PanConnectionStateEvent *event);
        void HandlePanSetTetheringEvent(PanSetTetheringEvent *event);
        void HandlePanGetModeEvent(PanGetModeEvent *event);
        void HandlePanDeviceConnectEvent(PanDeviceConnectEvent *event);
        void HandlePanDeviceDisconnectEvent(PanDeviceDisconnectEvent *event);
        void HandlePanDeviceConnectedListEvent(PanDeviceConnectedListEvent *event);
        void HandlePanIpcMsg(BtIpcMsg *ipcMsg);

        bool Connect(bt_bdaddr_t *addr, int src_role, int dest_role);
        bool Disconnect(bt_bdaddr_t *addr);
        int GetLocalRole();

        pan_device_t *AddDevice(int state, const bt_bdaddr_t *addr,
                int local_role, int remote_role);
        pan_device_t *FindDeviceByAddress(bt_bdaddr_t *addr);
        void RemoveDevice(pan_device_t *dev);

    public:
        Pan(const bt_interface_t *bt_interface, config_t *config);
        ~Pan();
        bool HandleEnablePan();
        bool HandleDisablePan();
        void ProcessEvent(BtEvent* pEvent);
};

#endif


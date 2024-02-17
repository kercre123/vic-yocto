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

#include <list>
#include <map>
#include "osi/include/log.h"
#include "Pan.hpp"
#include "utils.h"

#define LOGTAG "PAN "

#define PAN_APP_UI_PRINT printf

using namespace std;

Pan *g_pan = NULL;;

/**
 * The Configuration options
 */
const char *BT_PAN_NAP_ROLE_SUPPORTED         = "BtPanNapRoleSupported";
const char *BT_PAN_PANU_ROLE_SUPPORTED        = "BtPanPanuRoleSupported";

static const bt_bdaddr_t bd_addr_null= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const bt_bdaddr_t bd_addr_to_string= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#ifdef __cplusplus
extern "C"
{
#endif

static void control_state_callback(btpan_control_state_t state, int local_role,
        bt_status_t error, const char* ifname)
{
     BtEvent *event = new BtEvent;

     ALOGV(LOGTAG "%s: state %d", __FUNCTION__, state);

     event->event_id = PAN_EVENT_CONTROL_STATE_CHANGED;
     event->pan_control_state_event.state = state;
     event->pan_control_state_event.error = error;
     event->pan_control_state_event.local_role = local_role;
     event->pan_control_state_event.ifname= ifname;
     PostMessage(THREAD_ID_PAN, event);
}

static void connection_state_callback(btpan_connection_state_t state, bt_status_t error,
        const bt_bdaddr_t *bd_addr, int local_role, int remote_role)
{
    BtEvent *event = new BtEvent;

    ALOGV(LOGTAG "%s: state %d", __FUNCTION__, state);

    event->event_id = PAN_EVENT_CONNECTION_STATE_CHANGED;
    event->pan_connection_state_event.state = state;
    event->pan_connection_state_event.error = error;
    memcpy(&event->pan_connection_state_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    event->pan_connection_state_event.local_role = local_role;
    event->pan_connection_state_event.remote_role = remote_role;
    PostMessage(THREAD_ID_PAN, event);
}

static btpan_callbacks_t sBluetoothPanCallbacks = {
    sizeof(sBluetoothPanCallbacks),
    control_state_callback,
    connection_state_callback
};

void BtPanMsgHandler(void *msg)
{
    BtEvent* event = NULL;
    bool status = false;
    if(!msg) {
        ALOGE(LOGTAG "%s: Msg is null, return", __FUNCTION__);
        return;
    }

    event = ( BtEvent *) msg;

    if (event == NULL)
    {
        ALOGE(LOGTAG "%s: event is null", __FUNCTION__);
        return;
    }

    ALOGD(LOGTAG "BtPanMsgHandler event = %d", event->event_id);
    switch(event->event_id) {
        case PROFILE_API_START:
            if (g_pan) {
                status = g_pan->HandleEnablePan();

                BtEvent *start_event = new BtEvent;
                start_event->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
                start_event->profile_start_event.profile_id = PROFILE_ID_PAN;
                start_event->profile_start_event.status = status;
                PostMessage(THREAD_ID_GAP, start_event);
            }
            break;

        case PROFILE_API_STOP:
            if (g_pan) {
                status = g_pan->HandleDisablePan();

                BtEvent *stop_event = new BtEvent;
                stop_event->profile_start_event.event_id = PROFILE_EVENT_STOP_DONE;
                stop_event->profile_start_event.profile_id = PROFILE_ID_PAN;
                stop_event->profile_start_event.status = status;
                PostMessage(THREAD_ID_GAP, stop_event);
            }
            break;

        default:
            if(g_pan) {
                g_pan->ProcessEvent((BtEvent *) msg);
            }
            break;
    }
    delete event;
}

#ifdef __cplusplus
}
#endif

Pan :: Pan(const bt_interface_t *bt_interface, config_t *config)
{
    this->bluetooth_interface = bt_interface;
    this->config = config;

    //checking for nap role
    this->is_nap_role_supported = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_PAN_NAP_ROLE_SUPPORTED, false);

    this->is_panu_role_supported = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_PAN_PANU_ROLE_SUPPORTED, false);

}

Pan :: ~Pan()
{
}

pan_device_t* Pan ::AddDevice(int state, const bt_bdaddr_t *addr,
        int local_role, int remote_role)
{
    for (int i = 0; i < MAX_PAN_DEVICES; i++)
    {
        if (!memcmp(&(pan_device[i].bd_addr), &bd_addr_null, sizeof(bt_bdaddr_t)))
        {
            ALOGV(LOGTAG "%s:  state:%d, local_role:%d, remote_role:%d",
                    __FUNCTION__, state, local_role, remote_role);

            memcpy(&(pan_device[i].bd_addr), addr, sizeof(bt_bdaddr_t));
            pan_device[i].prev_state = BTPAN_STATE_DISCONNECTED;
            pan_device[i].state =  state;
            pan_device[i].local_role = local_role;
            pan_device[i].remote_role = remote_role;
            return &pan_device[i];
        }
    }
    ALOGE(LOGTAG "%s:  MAX_PAN_CONNS:%d exceeded, return NULL as failed",
            __FUNCTION__, MAX_PAN_DEVICES);
    return NULL;
}

void Pan ::RemoveDevice(pan_device_t *dev)
{
    ALOGV(LOGTAG "%s: ", __FUNCTION__);
    if (dev)
    {
        dev->prev_state = BTPAN_STATE_DISCONNECTED;
        dev->state = BTPAN_STATE_DISCONNECTED;
        memcpy(&(dev->bd_addr), &bd_addr_null, sizeof(bt_bdaddr_t));
        dev->local_role = dev->remote_role = 0;
    } else {
        ALOGE(LOGTAG "%s: device is NULL", __FUNCTION__);
    }

}

pan_device_t* Pan :: FindDeviceByAddress(bt_bdaddr_t *addr)
{
    ALOGV(LOGTAG "%s", __FUNCTION__);

    for (int i = 0; i < MAX_PAN_DEVICES; i++)
    {
        if (memcmp(&(pan_device[i].bd_addr), addr, sizeof(bt_bdaddr_t)) == 0) {
            ALOGV(LOGTAG "%s: device found at postion %d", __FUNCTION__, i);
            return &pan_device[i];
        }
    }
    return NULL;
}

int Pan :: GetLocalRole()
{
    int local_role = 0;

    ALOGV(LOGTAG "%s", __FUNCTION__);

    if (pan_interface)
        local_role  = pan_interface->get_local_role();
    return local_role;
}

bool Pan :: Connect(bt_bdaddr_t *addr, int src_role, int dest_role)
{
    bool ret = true;
    char bd_str[MAX_BD_STR_LEN];

    if (src_role == LOCAL_PANU_ROLE && (is_panu_role_supported == false
        || pan_state != UNTETHERED)) {
        PAN_APP_UI_PRINT("\nPANU role is not supported OR PAN connection is already active\n");
        ALOGE(LOGTAG "%s: PANU role is not supported OR PAN connection is already active:"
                "is_panu_role_supported: %d, pan_State: %d",
                __FUNCTION__, is_panu_role_supported, pan_state);
        return false;
    }

    bdaddr_to_string(addr, bd_str, MAX_BD_STR_LEN);
    ALOGV(LOGTAG "%s: %s, src_role: %d, dest_role:%d",
            __FUNCTION__, bd_str, src_role, dest_role);

    if (!pan_interface) return false;

    if (!addr) {
        ALOGE(LOGTAG "%s: Bluetooth device address null", __FUNCTION__);
        return false;
    }

    bt_status_t status;
    if ((status = pan_interface->connect((bt_bdaddr_t *) addr, src_role, dest_role)) !=
         BT_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed PAN channel connection, status: %d",
                __FUNCTION__, status);
        ret = false;
    }
    return ret;
}

bool Pan :: Disconnect(bt_bdaddr_t *addr)
{
    bool ret = true;
    char bd_str[MAX_BD_STR_LEN];

    bdaddr_to_string(addr, bd_str, MAX_BD_STR_LEN);
    ALOGV(LOGTAG "%s: %s", __FUNCTION__, bd_str);

    if (!pan_interface) return false;

    if (!addr) {
        ALOGE(LOGTAG "%s: Bluetooth device address null", __FUNCTION__);
        return false;
    }

    bt_status_t status;
    if ((status =pan_interface->disconnect((bt_bdaddr_t *) addr)) !=
         BT_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed disconnect pan channel, status: %d", __FUNCTION__, status);
        ret = false;
    }
    return ret;
}


bool Pan :: HandleEnablePan() {
    ALOGV(LOGTAG "%s", __FUNCTION__);

    pan_state = UNTETHERED;
    num_of_pan_device_connected = 0;
    is_tethering_on = false;

    for (int i = 0; i < MAX_PAN_DEVICES; i++) {
        memcpy(&(pan_device[i].bd_addr), &bd_addr_null, sizeof(bt_bdaddr_t));
        pan_device[i].prev_state = BTPAN_STATE_DISCONNECTED;
        pan_device[i].state = BTPAN_STATE_DISCONNECTED;
        pan_device[i].local_role = pan_device[i].remote_role = PAN_ROLE_NONE;
    }

    if ((pan_interface = (btpan_interface_t *)
            bluetooth_interface->get_profile_interface(BT_PROFILE_PAN_ID)) == NULL) {
        ALOGE(LOGTAG "%s: Failed to get Bluetooth PAN Interface", __FUNCTION__);
        return false;
    }

    bt_status_t status;
    if ((status = pan_interface->init(&sBluetoothPanCallbacks)) != BT_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed to initialize Bluetooth PAN, status: %d", __FUNCTION__, status);
        pan_interface = NULL;
        return false;
    }
    return true;
}

bool Pan :: HandleDisablePan()
{
    ALOGV(LOGTAG "%s", __FUNCTION__);

    if (pan_interface !=NULL) {
        ALOGE(LOGTAG "%s: Cleaning up Bluetooth PAN Interface...", __FUNCTION__);
        pan_interface->cleanup();
        pan_interface = NULL;
    }
    return true;
}

void Pan::HandlePanDeviceConnectedListEvent(PanDeviceConnectedListEvent *event)
{
    pan_device_t *pan_dev;

    ALOGV(LOGTAG "%s", __FUNCTION__);

    PAN_APP_UI_PRINT("\n*****Connected Device List*****\n");
    for (int i = 0; i < MAX_PAN_DEVICES; i++) {
        if (pan_device[i].state == BTPAN_STATE_CONNECTED) {
            char bd_str[MAX_BD_STR_LEN];
            bdaddr_to_string(&(pan_device[i].bd_addr), bd_str, MAX_BD_STR_LEN);
            PAN_APP_UI_PRINT("%s \n", bd_str);
        }
    }
    PAN_APP_UI_PRINT("*****End Connected Device List*****\n");
}


void Pan::HandlePanDeviceConnectEvent(PanDeviceConnectEvent *event)
{
    pan_device_t *pan_dev;

    ALOGV(LOGTAG "%s", __FUNCTION__);

    pan_dev = FindDeviceByAddress(&(event->bd_addr));

    if (pan_dev == NULL) {
        Connect(&(event->bd_addr), LOCAL_PANU_ROLE, REMOTE_NAP_ROLE);
    } else {
        ALOGV(LOGTAG "%s: Device is already connected", __FUNCTION__);
    }
}

void Pan::HandlePanDeviceDisconnectEvent(PanDeviceDisconnectEvent *event)
{
    pan_device_t *pan_dev;

    ALOGV(LOGTAG "%s", __FUNCTION__);

    pan_dev = FindDeviceByAddress(&(event->bd_addr));

    if (pan_dev != NULL) {
        Disconnect(&(event->bd_addr));
    } else {
        ALOGW(LOGTAG "%s: Device is already disconnected", __FUNCTION__);
    }
}

void Pan::HandlePanControlStateEvent(PanControlStateEvent *event)
{
    ALOGV(LOGTAG "%s", __FUNCTION__);

    strlcpy(pan_interface_name, event->ifname, sizeof(pan_interface_name));
}

void Pan::HandlePanConnectionStateEvent(PanConnectionStateEvent *event)
{
    pan_device_t *pan_dev;
    char bd_str[MAX_BD_STR_LEN];

    ALOGV(LOGTAG "%s", __FUNCTION__);

    pan_dev = FindDeviceByAddress(&(event->bd_addr));

    if (pan_dev == NULL) {
        pan_dev = AddDevice(event->state, &(event->bd_addr), event->local_role,
                event->remote_role);
        if (pan_dev == NULL) {
            // already reached to max connection, disconnect this connection
            ALOGE(LOGTAG "%s:pan device is null", __FUNCTION__);
            Disconnect(&(event->bd_addr));
            return;
        }
    } else {
        pan_dev->prev_state = pan_dev->state;
        pan_dev->state = event->state;
        pan_dev->local_role = event->local_role;
        pan_dev->remote_role = event->remote_role;
    }

    ALOGV(LOGTAG "%s: state change from %d to %d, pan_dev->remote_role %d",
            __FUNCTION__, pan_dev->prev_state, pan_dev->state, pan_dev->remote_role);

    if ((pan_dev->prev_state == BTPAN_STATE_DISCONNECTED
        || pan_dev->prev_state == BTPAN_STATE_CONNECTED)
        && pan_dev->state == BTPAN_STATE_DISCONNECTING) {
        ALOGW(LOGTAG "%s: Ignoring state change from %d to %d ",
                __FUNCTION__,pan_dev->prev_state, pan_dev->state);
        return;
    }

    if (pan_dev->remote_role == REMOTE_PANU_ROLE) {
        ALOGV(LOGTAG "%s: LOCAL_NAP_ROLE:REMOTE_PANU_ROLE : pan_state: %d",
                __FUNCTION__, pan_state);

        if (pan_dev->state == BTPAN_STATE_DISCONNECTED) {
            bdaddr_to_string((&pan_dev->bd_addr), bd_str, MAX_BD_STR_LEN);
            PAN_APP_UI_PRINT("%s IS DISCONNECTED\n", bd_str);
            RemoveDevice(pan_dev);
            num_of_pan_device_connected--;

            if ((pan_state == TETHERED || pan_state == PENDING)
                && num_of_pan_device_connected == 0) {
                BtEvent *event = new BtEvent;
                event->event_id = SKT_API_IPC_MSG_WRITE;
                event->bt_ipc_msg_event.ipc_msg.type = BT_IPC_DISABLE_TETHERING;
                event->bt_ipc_msg_event.ipc_msg.status = INITIATED;
                ALOGV (LOGTAG "%s: Posting msg main thread: disable tethering", __FUNCTION__);
                PostMessage (THREAD_ID_MAIN, event);
            }
        } else if (pan_dev->state == BTPAN_STATE_CONNECTED) {
            bdaddr_to_string((&pan_dev->bd_addr), bd_str, MAX_BD_STR_LEN);
            PAN_APP_UI_PRINT("%s IS CONNECTED\n", bd_str);
            num_of_pan_device_connected++;

            if ((!is_tethering_on) || (pan_dev->local_role == LOCAL_PANU_ROLE
                || pan_state == REVERSE_TETHERED)){
                ALOGW(LOGTAG "%s: BT tethering is off/Local role is PANU drop the connection",
                        __FUNCTION__);
                Disconnect(&pan_dev->bd_addr);
                return;
            }

            if (pan_state == UNTETHERED && num_of_pan_device_connected > 0) {
                BtEvent *event = new BtEvent;
                event->event_id = SKT_API_IPC_MSG_WRITE;
                event->bt_ipc_msg_event.ipc_msg.type = BT_IPC_ENABLE_TETHERING;
                event->bt_ipc_msg_event.ipc_msg.status = INITIATED;

                pan_state = PENDING;
                ALOGV (LOGTAG "%s: Posting msg main thread: enable tethering", __FUNCTION__);
                PostMessage (THREAD_ID_MAIN, event);
            }
        }
    } else if (pan_dev->remote_role == REMOTE_NAP_ROLE) {
        ALOGW(LOGTAG "%s: LOCAL_PANU_ROLE:REMOTE_NAP_ROLE", __FUNCTION__);

        if (pan_dev->state == BTPAN_STATE_DISCONNECTED) {
            bdaddr_to_string((&pan_dev->bd_addr), bd_str, MAX_BD_STR_LEN);
            PAN_APP_UI_PRINT("%s IS DISCONNECTED\n", bd_str);
            RemoveDevice(pan_dev);

            if (pan_state == REVERSE_TETHERED || pan_state == PENDING) {
                BtEvent *event = new BtEvent;
                event->event_id = SKT_API_IPC_MSG_WRITE;
                event->bt_ipc_msg_event.ipc_msg.type = BT_IPC_DISABLE_REVERSE_TETHERING;
                event->bt_ipc_msg_event.ipc_msg.status = INITIATED;
                ALOGV (LOGTAG "%s: Posting msg main thread: disable reverse tethering",
                        __FUNCTION__);
                PostMessage (THREAD_ID_MAIN, event);
            }
        } else if (pan_dev->state == BTPAN_STATE_CONNECTED) {
            bdaddr_to_string((&pan_dev->bd_addr), bd_str, MAX_BD_STR_LEN);
            PAN_APP_UI_PRINT("%s IS CONNECTED\n", bd_str);

            if (pan_state == UNTETHERED) {
                BtEvent *event = new BtEvent;
                event->event_id = SKT_API_IPC_MSG_WRITE;
                event->bt_ipc_msg_event.ipc_msg.type = BT_IPC_ENABLE_REVERSE_TETHERING;
                event->bt_ipc_msg_event.ipc_msg.status = INITIATED;

                pan_state = PENDING;
                ALOGV (LOGTAG "%s: Posting msg main thread: enable reverse tethering",
                        __FUNCTION__);
                PostMessage (THREAD_ID_MAIN, event);
            }
        }
    }
}

void Pan::HandlePanSetTetheringEvent(PanSetTetheringEvent *event)
{
    ALOGV(LOGTAG "%s", __FUNCTION__);

    if (is_nap_role_supported) {
         if (is_tethering_on == event->is_tethering_on) {
             ALOGW(LOGTAG "%s: prev_tether_val is equal to curr_tether_val", __FUNCTION__);
             return;
         }

        is_tethering_on = event->is_tethering_on;

        //Drop all Pan connection, when tethering is OFF
        if (is_tethering_on == false) {
            for (int i = 0; i < MAX_PAN_DEVICES; i++) {
                if (pan_device[i].state == BTPAN_STATE_CONNECTED) {
                    Disconnect(&(pan_device[i].bd_addr));
                }
            }
        }

        if (pan_interface !=NULL) {
            ALOGE(LOGTAG "%s: Notifying BT tethering UI status to BNEP layer", __FUNCTION__);
            pan_interface->set_tethering(is_tethering_on);
        }

        PAN_APP_UI_PRINT("\n*****TETHER MODE UI OPTION SUCCESSFULLY CHANGED*****\n");
    } else {
        ALOGW(LOGTAG "%s: LOCAL_NAP_ROLE not supported", __FUNCTION__);
        PAN_APP_UI_PRINT("\n*****LOCAL_NAP_ROLE not supported*****\n");
    }
}

void Pan::HandlePanGetModeEvent(PanGetModeEvent *event)
{
    ALOGV(LOGTAG "%s", __FUNCTION__);
    PAN_APP_UI_PRINT("\n*****TETHER MODE UI OPTION: %s*****\n",
        is_tethering_on ? "ENABLED": "DISABLED");

    switch (pan_state) {
        case UNTETHERED:
            PAN_APP_UI_PRINT("\n*****PAN IS IN UNTETHERED MODE*****\n");
            break;

        case TETHERED:
            PAN_APP_UI_PRINT("\n*****PAN IS IN TETHERED MODE*****\n");
            break;

        case REVERSE_TETHERED:
            PAN_APP_UI_PRINT("\n*****PAN IS IN REVERSE TETHERED MODE*****\n");
            break;

        default:
            //This should never happen
            PAN_APP_UI_PRINT("\n*****PAN IS IN UNKNOWN MODE*****\n");
            break;

    }
}


void Pan::HandlePanIpcMsg(BtIpcMsg *ipcMsg)
{
    ALOGV(LOGTAG "%s: ipcMsg->type: %d, ipcMsg->status = %d",
            __FUNCTION__, ipcMsg->type, ipcMsg->status);

    switch(ipcMsg->type) {
        case BT_IPC_ENABLE_TETHERING:
            switch(ipcMsg->status){
                case SUCCESS:
                    ALOGV(LOGTAG "%s: BT_IPC_ENABLE_TETHERING: SUCCESS", __FUNCTION__);
                    pan_state  = TETHERED;
                    PAN_APP_UI_PRINT("\n*****PAN IS IN TETHERED MODE*****\n");
                    break;

                case FAILED:
                    ALOGV(LOGTAG "%s: BT_IPC_ENABLE_TETHERING: FAILED : drop pan connection",
                            __FUNCTION__);
                    pan_state  = UNTETHERED;

                    for (int i = 0; i < MAX_PAN_DEVICES; i++) {
                        if (pan_device[i].state == BTPAN_STATE_CONNECTED) {
                            Disconnect(&(pan_device[i].bd_addr));
                        }
                    }
                    PAN_APP_UI_PRINT("\n*****PAN IS IN UNTETHERED MODE*****\n");
                    break;

                default:
                    ALOGW(LOGTAG "%s: unhandled status: %d", __FUNCTION__, ipcMsg->status);
                    break;
            }
            break;

        case BT_IPC_DISABLE_TETHERING:
            switch(ipcMsg->status){
                case SUCCESS:
                    ALOGV(LOGTAG "%s: BT_IPC_DISABLE_TETHERING: SUCCESS", __FUNCTION__);
                    pan_state  = UNTETHERED;
                    PAN_APP_UI_PRINT("\n*****PAN IS IN UNTETHERED MODE*****\n");
                    break;

                case FAILED:
                    ALOGV(LOGTAG "%s: BT_IPC_DISABLE_TETHERING: FAILED", __FUNCTION__);
                    pan_state  = UNTETHERED;
                    PAN_APP_UI_PRINT("\n*****PAN IS IN UNTETHERED MODE*****\n");
                    break;

                default:
                    ALOGW(LOGTAG "%s: unhandled status: %d", __FUNCTION__, ipcMsg->status);
                    break;
            }

            ALOGV (LOGTAG "%s: BT_IPC_DISABLE_TETHERING: drop all pan connection", __FUNCTION__);

            for (int i = 0; i < MAX_PAN_DEVICES; i++) {
                if (pan_device[i].state == BTPAN_STATE_CONNECTED) {
                    Disconnect(&(pan_device[i].bd_addr));
                }
            }
            break;

        case BT_IPC_ENABLE_REVERSE_TETHERING:
            switch(ipcMsg->status){
                case SUCCESS:
                    ALOGV(LOGTAG "%s: BT_IPC_ENABLE_REVERSE_TETHERING: SUCCESS", __FUNCTION__);
                    pan_state  = REVERSE_TETHERED;
                    PAN_APP_UI_PRINT("\n*****PAN IS IN REVERSE TETHERED MODE*****\n");
                    break;

                case FAILED:
                    ALOGV(LOGTAG "%s: BT_IPC_ENABLE_REVERSE_TETHERING: FAILED: drop pan connection",
                            __FUNCTION__);
                    pan_state  = UNTETHERED;

                    for (int i = 0; i < MAX_PAN_DEVICES; i++) {
                        if (pan_device[i].state == BTPAN_STATE_CONNECTED) {
                            Disconnect(&(pan_device[i].bd_addr));
                        }
                    }
                    PAN_APP_UI_PRINT("\n*****PAN IS IN UNTETHERED MODE*****\n");
                    break;

                default:
                    ALOGW(LOGTAG "%s: unhandled status: %d", __FUNCTION__, ipcMsg->status);
                    break;
            }
            break;

        case BT_IPC_DISABLE_REVERSE_TETHERING:
            switch(ipcMsg->status){
                case SUCCESS:
                    ALOGV(LOGTAG "%s: BT_IPC_DISABLE_REVERSE_TETHERING: SUCCESS", __FUNCTION__);
                    pan_state  = UNTETHERED;
                    PAN_APP_UI_PRINT("\n*****PAN IS IN UNTETHERED MODE*****\n");
                    break;

                case FAILED:
                    ALOGV(LOGTAG "%s: BT_IPC_DISABLE_REVERSE_TETHERING: FAILED", __FUNCTION__);
                    pan_state  = UNTETHERED;
                    PAN_APP_UI_PRINT("\n*****PAN IS IN UNTETHERED MODE*****\n");
                    break;

                default:
                    ALOGW(LOGTAG "%s: unhandled status: %d", __FUNCTION__, ipcMsg->status);
                    break;
            }

            ALOGV (LOGTAG "%s: BT_IPC_DISABLE_REVERSE_TETHERING: drop all pan connection",
                    __FUNCTION__);

            for (int i = 0; i < MAX_PAN_DEVICES; i++) {
                if (pan_device[i].state == BTPAN_STATE_CONNECTED) {
                    Disconnect(&(pan_device[i].bd_addr));
                }
            }
            break;


        default:
            ALOGW(LOGTAG "%s: unhandled ipc msg: %d", __FUNCTION__, ipcMsg->type);
            break;
    }
}

void Pan::ProcessEvent(BtEvent* event)
{
    ALOGD(LOGTAG "%s: Processing event %d", __FUNCTION__, event->event_id);

    switch(event->event_id) {
        case PAN_EVENT_CONNECTION_STATE_CHANGED:
            HandlePanConnectionStateEvent((PanConnectionStateEvent *)event);
            break;

        case PAN_EVENT_CONTROL_STATE_CHANGED:
            HandlePanControlStateEvent((PanControlStateEvent *)event);
            break;

        case PAN_EVENT_SET_TETHERING_REQ:
            HandlePanSetTetheringEvent((PanSetTetheringEvent *)event);
            break;

        case PAN_EVENT_GET_MODE_REQ:
            HandlePanGetModeEvent((PanGetModeEvent *)event);
            break;

        case PAN_EVENT_DEVICE_CONNECT_REQ:
            HandlePanDeviceConnectEvent((PanDeviceConnectEvent *)event);
            break;

        case PAN_EVENT_DEVICE_DISCONNECT_REQ:
            HandlePanDeviceDisconnectEvent((PanDeviceDisconnectEvent *)event);
            break;

        case PAN_EVENT_DEVICE_CONNECTED_LIST_REQ:
            HandlePanDeviceConnectedListEvent((PanDeviceConnectedListEvent *)event);
            break;

        case SKT_API_IPC_MSG_READ:
            HandlePanIpcMsg((BtIpcMsg *)(&event->bt_ipc_msg_event.ipc_msg));
            break;

        default:
            ALOGW(LOGTAG "%s: unhandled event: %d", __FUNCTION__, event->event_id);
            break;
    }
}



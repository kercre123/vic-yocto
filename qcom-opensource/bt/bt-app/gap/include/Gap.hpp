/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef GAP_APP_HPP
#define GAP_APP_HPP

#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/vendor.h>
#include <hardware/bt_sock.h>

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "osi/include/alarm.h"
#include "ipc.h"
#include "AdapterProperties.hpp"
#include "RemoteDevices.hpp"

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif

/**
 * @file Gap.hpp
 * @brief gap header file
*/

/**
 * Maximum Bonded Device
*/
#define MAX_BONDED_DEVICES (20)
#define PROFILE_STARTUP_TIMEOUT_DELAY     (5000)
#define PROFILE_STOP_TIMEOUT_DELAY        (5000)
#define ENABLE_TIMEOUT_DELAY              (12000)
#define DISABLE_TIMEOUT_DELAY             (8000)

const unsigned char g_audiosink_uuid[16] = {0x00, 0x00, 0x11, 0x0B, 0x00, 0x00,
                0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

const unsigned char g_audiosource_uuid[16] = {0x00, 0x00, 0x11, 0x0A, 0x00, 0x00,
                0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

const unsigned char g_advaudiodist_uuid[16] = {0x00, 0x00, 0x11, 0x0D,0x00, 0x00,
                0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

const unsigned char g_hsp_uuid[16] = {0x00, 0x00, 0x11, 0x08, 0x00, 0x00, 0x10,
                0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

const unsigned char g_hsp_aguuid[16] = {0x00, 0x00, 0x11, 0x12, 0x00, 0x00, 0x10,
                 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

const unsigned char g_handsfree_uuid[16] = {0x00, 0x00, 0x11, 0x1E, 0x00, 0x00, 0x10,
                 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

const unsigned char g_handsfree_aguuid[16] = {0x00, 0x00, 0x11, 0x1f, 0x00, 0x00,
                 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

const unsigned char g_avrcpcontroller_uuid[16] = {0x00, 0x00, 0x11, 0x0E, 0x00, 0x00,
                 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};

const unsigned char g_avrcptarget_uuid[16] = {0x00, 0x00, 0x11, 0x0C, 0x00, 0x00,
                 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};


typedef struct {
    ProfileIdType profile_id;
    ThreadIdType  thread_id;
    bool is_enabled;
    bool start_status;
    bool stop_status;
    char name[248];
} ProfileConfig;

enum ProfileType
{
  TYPE_AUDIO_SINK,
  TYPE_AUIDO_SOURCE,
  TYPE_ADVANCED_AUDIO,
  TYPE_HSP,
  TYPE_HSP_AG,
  TYPE_HANDSFREE,
  TYPE_HANDSFREE_AG,
  TYPE_AVRCP_CT,
  TYPE_AVRCP_TG
};

/**
 * @class Gap
 *
 * @brief Gap Class
 */
class Gap {

  private:
    config_t *config_;
    /**
     *  structure object for standard Bluetooth DM interface
     */
    const bt_interface_t *bluetooth_interface_;

    /**
     *  structure object for Vendor interface
     */
    const btvendor_interface_t *sBtVendorInterface;

    /**
     *  structure object for standard Bluetooth Socket interface
     */
    btsock_interface_t *sock_interface_;

    /**
     *  class object for @ref AdapterProperties class
     */
    AdapterProperties *adapter_properties_obj_;
    /**
     *  class object for @ref RemoteDevices class
     */
    RemoteDevices     *remote_devices_obj_;

    ProfileConfig  profile_config[PROFILE_ID_MAX];

    int supported_profiles_count;

    bool is_user_input_enabled_;

#ifdef USE_BT_OBEX
    bool is_obex_enabled_;

    int obex_logging_level_;
#endif

    /**
     * @brief HandlePinRequestEvent
     *
     * It is used to hadnle PinKey request to the main application and sends
     * a @ref MAIN_EVENT_PIN_REQUEST event to @ref THREAD_ID_MAIN main thread
     *
     * @param  PINRequestEvent  event
     * @return none
     */
    void HandlePinRequestEvent(PINRequestEvent *event);
    /**
     * @brief HandleSspRequestEvent
     *
     * It is used to handle SSP request to the main application and sends a
     * @ref MAIN_EVENT_SSP_REQUEST event to @ref THREAD_ID_MAIN main thread
     *
     * @param  SSPRequestEvent event
     * @return none
     */
    void HandleSspRequestEvent(SSPRequestEvent *event);

    /**
     * @brief HandleBondStateEvent
     *
     * It is used to notify bond state to the main application. and calls a
     * @ref OnbondStateChanged function of @ref AdapterProperties class to handle
     * bond_state event
     *
     * @param  DeviceBondStateEventInt  event
     * @return none
     */
    void HandleBondStateEvent(DeviceBondStateEventInt *event);

    /**
     * @brief HandleEnable
     *
     * This function will enable the bluetooth by internally calling
     * stack provided enable() callback
     *
     * @return none
     */
    void HandleEnable();

    /**
     * @brief HandleDisable
     *
     * This function will disable the bluetooth by internally calling
     * stack provided disable() callback
     *
     * @return none
     */
    void HandleDisable();
    /**
     * @brief HandleStartDiscovery
     *
     * This function will initiate the discovery by calling stack provided start_discovery()
     * callback
     *
     * @return none
     */
    void HandleStartDiscovery();
    /**
     * @brief HandleStopDiscovery
     *
     * This function will stops the discovery by calling stack provided cancel_discovery()
     * callback
     *
     * @return none
     */
    void HandleStopDiscovery();

    /**
     * @brief HandleSspReply
     *
     * This function send ssp reply by calling stack provided ssp_reply callback
     *
     * @param SSPReplyEvent
     * @return none
     */
    void HandleSspReply(SSPReplyEvent *event);
    /**
     * @brief HandlePinReply
     *
     * This function send pin reply by calling stack provided pin_reply callback
     *
     * @param PINReplyEvent
     * @return none
     */
    void HandlePinReply(PINReplyEvent *event);
  public:
    Gap(const bt_interface_t *bt_interface, config_t *config);
    ~Gap();
    alarm_t *profile_startup_timer;
    alarm_t *profile_stop_timer;
    alarm_t *enable_timer;
    alarm_t *disable_timer;

    /**

     * @brief ProcessEvent
     *
     * It will handle incomming events
     *
     * @param BtEvent
     * @return none
     */
    void ProcessEvent(BtEvent* event);

    /**
     * @brief GetState
     *
     * It will check the current BT state, and returns the state of BT
     *
     * @return int
     */
    int  GetState();

    /**
     * @brief IsEnabled
     *
     * It will check the current BT state, If state is BT_STATE_ON it will return true
     * else it will return false
     * @return bool
     */
    bool IsEnabled();

    /**
     * @brief IsDiscovering
     *
     * It will check discovering state, If discovery is in progress then returns true
     * else returns false
     * @return bool
     */
    bool IsDiscovering();

    /**
     * @brief GetBtAddress
     *
     * It will return's the local bluetooth address
     *
     * @return @ref bt_bdaddr_t
     */
    bt_bdaddr_t *GetBtAddress(void);

    /**
     * @brief GetBtName
     *
     * It will return's the local bluetooth name
     *
     * @return bluetooth name @ref bt_bdname_t
     */
    bt_bdname_t *GetBtName(void);

    /**
     * @brief SetBtName
     *
     * It will set the local bluetooth name
     *
     * @return status
     */
    int SetBtName(bt_property_t *prop);

    /**
     * @brief IsDeviceBonded
     *
     * It will return's true if device is already bonded else returns false
     *
     * @return bool
     */
    bool IsDeviceBonded(bt_bdaddr_t device);
};

#endif

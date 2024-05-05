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

#pragma once

#include "osi/include/thread.h"
#include <hardware/bluetooth.h>
#include <hardware/bt_gatt.h>
#include <hardware/bt_gatt_types.h>
#include <hardware/bt_sdp.h>
#include <hardware/bt_rc.h>


extern thread_t *g_gap_thread;
extern thread_t *g_main_thread;
extern thread_t *g_socket_thread;
extern thread_t *g_pan_thread;
extern thread_t *g_gatt_thread;
extern thread_t *g_pbapc_thread;

// TODO: move this to bitbake
//#define BT_MODEM_INTEGRATION 0

/**
 * @file ipc.h
 *
 * @brief It is common header file which contains all event related structures
 */

#define MAIN_MSG_BASE           (0)
#define GAP_MSG_BASE            (1000)
#define PAN_MSG_BASE            (2000)
#define GATT_MSG_BASE           (3000)
#define RSP_MSG_BASE            (4000)
#define SDP_CLIENT_MSG_BASE     (5000)
#define PBAP_CLIENT_MSG_BASE    (6000)
#define OPP_MSG_BASE            (7000)

#define AUDIO_MANAGER_MSG_BASE  (250)
#define A2DP_SINK_MSG_BASE      (300)
#define HFP_CLIENT_MSG_BASE     (400)
#define A2DP_SOURCE_MSG_BASE    (500)
#define HFP_AG_MSG_BASE         (600)
#define AVRCP_MSG_BASE          (700)
#define MAX_BD_STR_LEN          (18)
#define BT_IPC_MSG_LEN 2

#define CMD_ID_VOL_UP           0x41
#define CMD_ID_VOL_DOWN         0x42
#define CMD_ID_PLAY             0x44
#define CMD_ID_STOP             0x45
#define CMD_ID_PAUSE            0x46
#define CMD_ID_REWIND           0x48
#define CMD_ID_FF               0x49
#define CMD_ID_FORWARD          0x4B
#define CMD_ID_BACKWARD         0x4C

#define KEY_PRESSED             0
#define KEY_RELEASED            1

/**
 *   Threads info
 */
typedef enum {
    THREAD_ID_MAIN = 0,
    THREAD_ID_GAP,
    THREAD_ID_A2DP_SINK,
    THREAD_ID_HFP_CLIENT,
    THREAD_ID_PAN,
    THREAD_ID_GATT,
    THREAD_ID_BT_AM,
    THREAD_ID_SDP_CLIENT,
#ifdef USE_BT_OBEX
    THREAD_ID_PBAP_CLIENT,
    THREAD_ID_OPP,
#endif
    THREAD_ID_HFP_AG,
    THREAD_ID_A2DP_SOURCE,
    THREAD_ID_AVRCP,
    THREAD_ID_MAX,
} ThreadIdType;

/**
 *   Profiles info
 */
typedef enum {
    PROFILE_ID_A2DP_SINK = 0,
    PROFILE_ID_HFP_CLIENT,
    PROFILE_ID_BT_AM,
    PROFILE_ID_PAN,
    PROFILE_ID_GATT,
    PROFILE_ID_SDP_CLIENT,
#ifdef USE_BT_OBEX
    PROFILE_ID_PBAP_CLIENT,
    PROFILE_ID_OPP,
#endif
    PROFILE_ID_HFP_AG,
    PROFILE_ID_A2DP_SOURCE,
    PROFILE_ID_AVRCP,
    PROFILE_ID_MAX
} ProfileIdType;

/**
 *   enums for BTAudioManager
 */
typedef enum {
    REQUEST_TYPE_PERMANENT = 0,
    REQUEST_TYPE_TRANSIENT,       // Transient focus is always for a call
    REQUEST_TYPE_DEFAULT
} ControlRequestType;

typedef enum {
    STATUS_LOSS = 0,
    STATUS_LOSS_TRANSIENT,
    STATUS_GAIN_TRANSIENT,
    STATUS_GAIN,
    STATUS_REGAINED
} ControlStatusType;

typedef void (*ThreadHandler) (void *context);

typedef struct {
    thread_t *thread_id;
    ThreadIdType thread_type;
    ThreadHandler thread_handler;
    char thread_name[50];
} ThreadInfo;

/**
 *  list of EVENTS used by GAP and MAIN thread
 */
typedef enum {
    MAIN_API_INIT = (MAIN_MSG_BASE + 1),
    MAIN_API_DEINIT,
    MAIN_API_ENABLE,
    MAIN_API_DISABLE,
    MAIN_EVENT_ACL_CONNECTED,
    MAIN_EVENT_ACL_DISCONNECTED,
    MAIN_EVENT_DEVICE_FOUND,
    MAIN_EVENT_INQUIRY_STATUS,
    MAIN_EVENT_BOND_STATE,
    MAIN_EVENT_ENABLED,
    MAIN_EVENT_DISABLED,
    MAIN_EVENT_SSP_REQUEST,
    MAIN_EVENT_PIN_REQUEST,
#ifdef USE_BT_OBEX
    MAIN_EVENT_INCOMING_FILE_REQUEST,
#endif

    MAIN_MSG_DISCOVER_DEVICES,
    MAIN_MSG_BOND_DEVICE,
    MAIN_MSG_CONNECT_DEVICE,
    MAIN_MSG_DISCONNECT_DEVICE,

    BT_AM_REQUEST_CONTROL = AUDIO_MANAGER_MSG_BASE,
    BT_AM_RELEASE_CONTROL,
    BT_AM_CONTROL_STATUS,

    A2DP_SINK_API_CONNECT_REQ = A2DP_SINK_MSG_BASE,
    A2DP_SINK_API_DISCONNECT_REQ,
    A2DP_SINK_DISCONNECTED_CB,
    A2DP_SINK_CONNECTING_CB,
    A2DP_SINK_CONNECTED_CB,
    A2DP_SINK_DISCONNECTING_CB,
    A2DP_SINK_FOCUS_REQUEST_CB,
    A2DP_SINK_AUDIO_SUSPENDED,
    A2DP_SINK_AUDIO_STOPPED,
    A2DP_SINK_AUDIO_STARTED,
    A2DP_SINK_CODEC_CONFIG,
    A2DP_SINK_FETCH_PCM_DATA,
    A2DP_SINK_FILL_COMPRESS_BUFFER,
    A2DP_SINK_CLEANUP_REQ,
    A2DP_SINK_CLEANUP_DONE,
    A2DP_SINK_STREAMING_FLUSH_AUDIO,

    A2DP_SINK_STREAMING_CLEANUP_REQ,
    A2DP_SINK_STREAMING_API_START,
    A2DP_SINK_STREAMING_API_STOP,
    A2DP_SINK_STREAMING_OPEN_INPUT_STREAM,
    A2DP_SINK_STREAMING_CLOSE_AUDIO_STREAM,
    A2DP_SINK_STREAMING_AM_REQUEST_CONTROL,
    A2DP_SINK_STREAMING_FETCH_PCM_DATA,
    A2DP_SINK_STREAMING_CONTROL_STATUS,
    A2DP_SINK_STREAMING_AM_RELEASE_CONTROL,
    A2DP_SINK_STREAMING_DISCONNECTED,
    A2DP_SINK_STREAMING_DISABLE_DONE,
    A2DP_SINK_CODEC_LIST,

    AVRCP_CTRL_CONNECTED_CB = AVRCP_MSG_BASE,
    AVRCP_CTRL_DISCONNECTED_CB,
    AVRCP_CTRL_PASS_THRU_CMD_REQ,
    AVRCP_CLEANUP_REQ,
    AVRCP_CLEANUP_DONE,
    AVRCP_CTRL_REG_NOTI_ABS_VOL_CB,
    AVRCP_CTRL_VOL_CHANGED_NOTI_REQ,
    AVRCP_CTRL_SET_ABS_VOL_CMD_CB,
    AVRCP_CTRL_GET_CAP_REQ,
    AVRCP_CTRL_LIST_PALYER_SETTING_ATTR_REQ,
    AVRCP_CTRL_LIST_PALYER_SETTING_VALUE_REQ,
    AVRCP_CTRL_GET_PALYER_APP_SETTING_REQ,
    AVRCP_CTRL_SET_PALYER_APP_SETTING_VALUE_REQ,
    AVRCP_CTRL_GET_ELEMENT_ATTR_REQ,
    AVRCP_CTRL_GET_PLAY_STATUS_REQ,
    AVRCP_CTRL_REG_NOTIFICATION_REQ,
    AVRCP_CTRL_SET_ADDRESSED_PLAYER_REQ,
    AVRCP_CTRL_SET_BROWSED_PLAYER_REQ,
    AVRCP_CTRL_CHANGE_PATH_REQ,
    AVRCP_CTRL_GET_FOLDER_ITEMS_REQ,
    AVRCP_CTRL_GET_ITEM_ATTRIBUTES_REQ,
    AVRCP_CTRL_PLAY_ITEMS_REQ,
    AVRCP_CTRL_ADDTO_NOW_PLAYING_REQ,
    AVRCP_CTRL_SEARCH_REQ,

    HFP_CLIENT_API_ENABLE = HFP_CLIENT_MSG_BASE,
    HFP_CLIENT_API_DISABLE,
    HFP_CLIENT_API_ENABLE_DONE,
    HFP_CLIENT_API_ENABLE_FAILED,
    HFP_CLIENT_API_DISABLE_DONE,
    HFP_CLIENT_API_CONNECT_REQ,
    HFP_CLIENT_API_DISCONNECT_REQ,
    HFP_CLIENT_API_CONNECT_AUDIO_REQ,
    HFP_CLIENT_API_DISCONNECT_AUDIO_REQ,
    HFP_CLIENT_API_ACCEPT_CALL_REQ,
    HFP_CLIENT_API_REJECT_CALL_REQ,
    HFP_CLIENT_API_END_CALL_REQ,
    HFP_CLIENT_API_HOLD_CALL_REQ,
    HFP_CLIENT_API_RELEASE_HELD_CALL_REQ,
    HFP_CLIENT_API_RELEASE_ACTIVE_ACCEPT_WAITING_OR_HELD_CALL_REQ,
    HFP_CLIENT_API_SWAP_CALLS_REQ,
    HFP_CLIENT_API_ADD_HELD_CALL_TO_CONF_REQ,
    HFP_CLIENT_API_RELEASE_SPECIFIED_ACTIVE_CALL_REQ,
    HFP_CLIENT_API_PRIVATE_CONSULTATION_MODE_REQ,
    HFP_CLIENT_API_PUT_INCOMING_CALL_ON_HOLD_REQ,
    HFP_CLIENT_API_ACCEPT_HELD_INCOMING_CALL_REQ,
    HFP_CLIENT_API_REJECT_HELD_INCOMING_CALL_REQ,
    HFP_CLIENT_API_DIAL_REQ,
    HFP_CLIENT_API_REDIAL_REQ,
    HFP_CLIENT_API_DIAL_MEMORY_REQ,
    HFP_CLIENT_API_START_VR_REQ,
    HFP_CLIENT_API_STOP_VR_REQ,
    HFP_CLIENT_API_CALL_ACTION_REQ,
    HFP_CLIENT_API_QUERY_CURRENT_CALLS_REQ,
    HFP_CLIENT_API_QUERY_OPERATOR_NAME_REQ,
    HFP_CLIENT_API_QUERY_SUBSCRIBER_INFO_REQ,
    HFP_CLIENT_API_SCO_VOL_CTRL_REQ,
    HFP_CLIENT_API_MIC_VOL_CTRL_REQ,
    HFP_CLIENT_API_SPK_VOL_CTRL_REQ,
    HFP_CLIENT_API_SEND_DTMF_REQ,
    HFP_CLIENT_API_DISABLE_NREC_ON_AG_REQ,
    HFP_CLIENT_API_SEND_AT_CMD_REQ,
    HFP_CLIENT_DISCONNECTED_CB,
    HFP_CLIENT_PLAY_RINGTONE_REQ,
    HFP_CLIENT_STOP_RINGTONE_REQ,
    HFP_CLIENT_CONNECTING_CB,
    HFP_CLIENT_CONNECTED_CB,
    HFP_CLIENT_SLC_CONNECTED_CB,
    HFP_CLIENT_DISCONNECTING_CB,
    HFP_CLIENT_AUDIO_STATE_DISCONNECTED_CB,
    HFP_CLIENT_AUDIO_STATE_CONNECTING_CB,
    HFP_CLIENT_AUDIO_STATE_CONNECTED_CB,
    HFP_CLIENT_AUDIO_STATE_CONNECTED_MSBC_CB,

    HFP_AG_API_ENABLE = HFP_AG_MSG_BASE,
    HFP_AG_API_DISABLE,
    HFP_AG_API_ENABLE_DONE,
    HFP_AG_API_ENABLE_FAILED,
    HFP_AG_API_DISABLE_DONE,
    HFP_AG_API_INIT_MODEM,
    HFP_AG_API_DEINIT_MODEM,
    HFP_AG_API_CONNECT_REQ,
    HFP_AG_API_DISCONNECT_REQ,
    HFP_AG_API_CONNECT_AUDIO_REQ,
    HFP_AG_API_DISCONNECT_AUDIO_REQ,
    HFP_AG_API_DIAL_REQ,
    HFP_AG_API_REDIAL_REQ,
    HFP_AG_API_ACCEPT_CALL_REQ,
    HFP_AG_API_REJECT_CALL_REQ,
    HFP_AG_API_END_CALL_REQ,
    HFP_AG_API_HOLD_CALL_REQ,
    HFP_AG_API_SWAP_CALLS_REQ,
    HFP_AG_API_RELEASE_HELD_CALL_REQ,
    HFP_AG_API_ADD_HELD_CALL_TO_CONF_REQ,
    HFP_AG_API_DUMP_CALLS_REQ,
    HFP_AG_API_QUERY_CURRENT_CALLS_REQ,
    HFP_AG_API_QUERY_OPERATOR_NAME_REQ,
    HFP_AG_API_QUERY_SUBSCRIBER_INFO_REQ,
    HFP_AG_API_START_VR_REQ,
    HFP_AG_API_STOP_VR_REQ,
    HFP_AG_API_MIC_VOL_CTRL_REQ,
    HFP_AG_API_SPK_VOL_CTRL_REQ,
    HFP_AG_CONNECTING_CB,
    HFP_AG_CONNECTED_CB,
    HFP_AG_SLC_CONNECTED_CB,
    HFP_AG_DISCONNECTING_CB,
    HFP_AG_DISCONNECTED_CB,
    HFP_AG_AUDIO_STATE_DISCONNECTING_CB,
    HFP_AG_AUDIO_STATE_DISCONNECTED_CB,
    HFP_AG_AUDIO_STATE_CONNECTING_CB,
    HFP_AG_AUDIO_STATE_CONNECTED_CB,
    HFP_AG_VR_CB,
    HFP_AG_ANSWER_CALL_CB,
    HFP_AG_HANGUP_CALL_CB,
    HFP_AG_VOL_CONTROL_CB,
    HFP_AG_DIAL_CALL_CB,
    HFP_AG_DTMF_CB,
    HFP_AG_NREC_CB,
    HFP_AG_WBS_CB,
    HFP_AG_CHLD_CB,
    HFP_AG_SUBSCRIBER_INFO_CB,
    HFP_AG_CIND_CB,
    HFP_AG_COPS_CB,
    HFP_AG_CLCC_CB,
    HFP_AG_UNKNOWN_AT_CMD_CB,
    HFP_AG_BIND_CB,
    HFP_AG_BIEV_CB,
    HFP_AG_RIL_IND_CB,
    HFP_AG_RIL_RESP_CB,

    A2DP_SOURCE_API_CONNECT_REQ = A2DP_SOURCE_MSG_BASE,
    A2DP_SOURCE_API_DISCONNECT_REQ,
    A2DP_SOURCE_AUDIO_CMD_REQ,
    A2DP_SOURCE_CONNECTION_PRIORITY_REQ,
    A2DP_SOURCE_DISCONNECTED_CB,
    A2DP_SOURCE_CONNECTING_CB,
    A2DP_SOURCE_CONNECTED_CB,
    A2DP_SOURCE_DISCONNECTING_CB,
    A2DP_SOURCE_CODEC_CONFIG_CB,
    A2DP_SOURCE_AUDIO_SUSPENDED,
    A2DP_SOURCE_AUDIO_STOPPED,
    A2DP_SOURCE_AUDIO_STARTED,
    A2DP_SOURCE_CODEC_LIST,
    AVRCP_TARGET_CONNECTED_CB,
    AVRCP_TARGET_DISCONNECTED_CB,
    AVRCP_TARGET_GET_ELE_ATTR,
    AVRCP_TARGET_GET_PLAY_STATUS,
    AVRCP_TARGET_LIST_PLAYER_APP_ATTR,
    AVRCP_TARGET_LIST_PLAYER_APP_VALUES,
    AVRCP_TARGET_GET_PLAYER_APP_VALUE,
    AVRCP_TARGET_SET_PLAYER_APP_VALUE,
    AVRCP_TARGET_REG_NOTI,
    AVRCP_TARGET_TRACK_CHANGED,
    AVRCP_TARGET_VOLUME_CHANGED,
    AVRCP_TARGET_SET_ABS_VOL,
    AVRCP_TARGET_ABS_VOL_TIMEOUT,
    AVRCP_TARGET_SEND_VOL_UP_DOWN,
    AVRCP_TARGET_PLAY_POSITION_TIMEOUT,
    AVRCP_TARGET_GET_FOLDER_ITEMS_CB,
    AVRCP_TARGET_SET_ADDR_PLAYER_CB,
    AVRCP_TARGET_ADDR_PLAYER_CHANGED,
    AVRCP_TARGET_AVAIL_PLAYER_CHANGED,
    AVRCP_TARGET_USE_BIGGER_METADATA,
    AVRCP_SET_EQUALIZER_VAL,
    AVRCP_SET_REPEAT_VAL,
    AVRCP_SET_SHUFFLE_VAL,
    AVRCP_SET_SCAN_VAL,

    GAP_API_ENABLE = GAP_MSG_BASE,
    GAP_API_DISABLE,
    GAP_API_START_INQUIRY,
    GAP_API_STOP_INQUIRY,
    GAP_API_CREATE_BOND,
    GAP_API_SSP_REPLY,
    GAP_API_PIN_REPLY,
    GAP_API_SET_BDNAME,

    GAP_EVENT_ADAPTER_STATE,
    GAP_EVENT_ACL_STATE_CHANGED,
    GAP_EVENT_DISCOVERY_STATE_CHANGED,
    GAP_EVENT_DEVICE_FOUND_INT,
    GAP_EVENT_DEVICE_FOUND,
    GAP_EVENT_PIN_REQUEST,
    GAP_EVENT_SSP_REQUEST,
    GAP_EVENT_REMOTE_DEVICE_PROPERTIES,
    GAP_EVENT_ADAPTER_PROPERTIES,
    GAP_EVENT_BOND_STATE_INT,
    GAP_EVENT_BOND_STATE,
    GAP_EVENT_PROFILE_START_TIMEOUT,
    GAP_EVENT_PROFILE_STOP_TIMEOUT,
    GAP_EVENT_ENABLE_TIMEOUT,
    GAP_EVENT_DISABLE_TIMEOUT,
    GAP_EVENT_SSR_CLEANUP,
    SKT_API_START_LISTENER,
    SKT_API_IPC_MSG_WRITE,
    SKT_API_IPC_MSG_READ,

    PROFILE_API_START,
    PROFILE_API_STOP,
    PROFILE_EVENT_START_DONE,
    PROFILE_EVENT_STOP_DONE,

    PAN_EVENT_CONTROL_STATE_CHANGED = PAN_MSG_BASE,
    PAN_EVENT_CONNECTION_STATE_CHANGED,
    PAN_EVENT_SET_TETHERING_REQ,
    PAN_EVENT_GET_MODE_REQ,
    PAN_EVENT_DEVICE_CONNECT_REQ,
    PAN_EVENT_DEVICE_DISCONNECT_REQ,
    PAN_EVENT_DEVICE_CONNECTED_LIST_REQ,

    //GATTS EVENTS
    BTGATTS_REGISTER_APP_EVENT = GATT_MSG_BASE,
    BTGATTS_CONNECTION_EVENT,
    BTGATTS_SERVICE_ADDED_EVENT,
    BTGATTS_INCLUDED_SERVICE_ADDED_EVENT,
    BTGATTS_CHARACTERISTIC_ADDED_EVENT,
    BTGATTS_DESCRIPTOR_ADDED_EVENT,
    BTGATTS_SERVICE_STARTED_EVENT,
    BTGATTS_SERVICE_STOPPED_EVENT,
    BTGATTS_SERVICE_DELETED_EVENT,
    BTGATTS_REQUEST_READ_EVENT,
    BTGATTS_REQUEST_WRITE_EVENT,
    BTGATTS_REQUEST_EXEC_WRITE_EVENT,
    BTGATTS_RESPONSE_CONFIRMATION_EVENT,
    BTGATTS_INDICATION_SENT_EVENT,
    BTGATTS_CONGESTION_EVENT,
    BTGATTS_MTU_CHANGED_EVENT,

    //GATTC EVENTS
    BTGATTC_REGISTER_APP_EVENT,
    BTGATTC_SCAN_RESULT_EVENT,
    BTGATTC_OPEN_EVENT,
    BTGATTC_CLOSE_EVENT,
    BTGATTC_SEARCH_COMPLETE_EVENT,
    BTGATTC_SEARCH_RESULT_EVENT,
    BTGATTC_GET_CHARACTERISTIC_EVENT,
    BTGATTC_GET_DESCRIPTOR_EVENT,
    BTGATTC_GET_INCLUDED_SERVICE_EVENT,
    BTGATTC_REGISTER_FOR_NOTIFICATION_EVENT,
    BTGATTC_NOTIFY_EVENT,
    BTGATTC_READ_CHARACTERISTIC_EVENT,
    BTGATTC_WRITE_CHARACTERISTIC_EVENT,
    BTGATTC_READ_DESCRIPTOR_EVENT,
    BTGATTC_WRITE_DESCRIPTOR_EVENT,
    BTGATTC_EXECUTE_WRITE_EVENT,
    BTGATTC_REMOTE_RSSI_EVENT,
    BTGATTC_ADVERTISE_EVENT,
    BTGATTC_CONFIGURE_MTU_EVENT,
    BTGATTC_SCAN_FILTER_CFG_EVENT,
    BTGATTC_SCAN_FILTER_PARAM_EVENT,
    BTGATTC_SCAN_FILTER_STATUS_EVENT,
    BTGATTC_MULTIADV_ENABLE_EVENT,
    BTGATTC_MULTIADV_UPDATE_EVENT,
    BTGATTC_MULTIADV_SETADV_DATA_EVENT,
    BTGATTC_MULTIADV_DISABLE_EVENT,
    BTGATTC_CONGESTION_EVENT,
    BTGATTC_BATCHSCAN_CFG_STORAGE_EVENT,
    BTGATTC_BATCHSCAN_STARTSTOP_EVENT,
    BTGATTC_BATCHSCAN_REPORTS_EVENT,
    BTGATTC_BATCHSCAN_THRESHOLD_EVENT,
    BTGATTC_TRACK_ADV_EVENT_EVENT,
    BTGATTC_SCAN_PARAMETER_SETUP_COMPLETED_EVENT,
    BTGATTC_GET_GATT_DB_EVENT,

    RSP_ENABLE_EVENT = RSP_MSG_BASE,
    RSP_DISABLE_EVENT,

    SDP_CLIENT_SEARCH = SDP_CLIENT_MSG_BASE,
    SDP_CLIENT_ADD_RECORD,
    SDP_CLIENT_REMOVE_RECORD,
    SDP_CLIENT_SEARCH_TIMEOUT,

    PBAP_CLIENT_REGISTER = PBAP_CLIENT_MSG_BASE,
    PBAP_CLIENT_CONNECT,
    PBAP_CLIENT_INTERNAL_CONNECT,
    PBAP_CLIENT_CONNECT_TIMEOUT,
    PBAP_CLIENT_DISCONNECT,
    PBAP_CLIENT_ABORT,
    PBAP_CLIENT_GET_PHONEBOOK_SIZE,
    PBAP_CLIENT_GET_PHONEBOOK,
    PBAP_CLIENT_GET_VCARD,
    PBAP_CLIENT_GET_VCARD_LISTING,
    PBAP_CLIENT_SET_PATH,
    PBAP_CLIENT_SET_FILTER,
    PBAP_CLIENT_SET_ORDER,
    PBAP_CLIENT_SET_SEARCH_ATTRIBUTE,
    PBAP_CLIENT_SET_SEARCH_VALUE,
    PBAP_CLIENT_SET_PHONE_BOOK,
    PBAP_CLIENT_SET_REPOSITORY,
    PBAP_CLIENT_SET_VCARD_FORMAT,
    PBAP_CLIENT_SET_LIST_COUNT,
    PBAP_CLIENT_SET_START_OFFSET,
    PBAP_CLIENT_GET_FILTER,
    PBAP_CLIENT_GET_ORDER,
    PBAP_CLIENT_GET_SEARCH_ATTRIBUTE,
    PBAP_CLIENT_GET_PHONE_BOOK,
    PBAP_CLIENT_GET_REPOSITORY,
    PBAP_CLIENT_GET_VCARD_FORMAT,
    PBAP_CLIENT_GET_LIST_COUNT,
    PBAP_CLIENT_GET_START_OFFSET,

    OPP_SRV_REGISTER = OPP_MSG_BASE,
    OPP_SEND_DATA,
    OPP_ABORT_TRANSFER,
    OPP_INTERNAL_CONNECT,
    OPP_INTERNAL_SEND,
    OPP_INTERNAL_DISCONNECTION,
    OPP_INCOMING_FILE_RESPONSE,
    OPP_CONNECT_TIMEOUT,
} BluetoothEventId;

typedef struct {
    bt_bdaddr_t address;
    char name[248];
    int bluetooth_class;
    short rssi;
    bt_uuid_t uuids[16];
    int device_type;
    int ret_value;
    char alias[64];
    int bond_state;
    bool broadcast;
} DeviceProperties;

typedef enum {
    BT_ADAPTER_STATE_OFF,
    BT_ADAPTER_STATE_ON,
    BT_ADAPTER_STATE_TURNING_ON,
    BT_ADAPTER_STATE_TURNING_OFF
} AdapterState;

/**
 * Generic Event for GAP
 */
typedef struct {
    BluetoothEventId event_id;
    bt_state_t          status;
} GapAppEvent;

/**
 * Event for notifying Remote Device properties
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t         bd_addr;
    int                 num_properties;
    bt_property_t       *properties;
} RemotePropertiesEvent;

/**
 * Event for notifying Adapter Properties
 */
typedef struct {
    BluetoothEventId event_id;
    int                 num_properties;
    bt_property_t       *properties;
} AdapterPropertiesEvent;

/**
 * Event for notifying Device found, It used only for internall threads
 */
typedef struct {
    BluetoothEventId event_id;
    int                 num_properties;
    bt_property_t       *properties;
} DeviceFoundEventInt;

/**
 * Event for notifying Device found
 */
typedef struct {
    BluetoothEventId event_id;
    DeviceProperties remoteDevice;
} DeviceFoundEvent;

/**
 * Event for notifying Device Bond state
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bond_state_t     state;
    bt_bdaddr_t         bd_addr;
} DeviceBondStateEventInt;

/**
 * Event for notifying Device Bond state
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bond_state_t     state;
    bt_bdaddr_t         bd_addr;
    bt_bdname_t         bd_name;
} DeviceBondStateEvent;

/**
 * Event for notifying ACL state
 */
typedef struct {
    BluetoothEventId event_id;
    bt_status_t status;
    bt_bdaddr_t bd_addr;
    bt_acl_state_t state;
} ACLStateEvent;

/**
 * Event for notifying Discovery state
 */
typedef struct {
    BluetoothEventId event_id;
    bt_discovery_state_t state;
} DiscoveryStateEvent;

/**
 * Event for notifying pin request
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t         bd_addr;
    bt_bdname_t         bd_name;
    uint32_t            cod;
    bool                secure;
} PINRequestEvent;

/**
 * Event to post pin reply
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t         bd_addr;
    bt_bdname_t         bd_name;
    uint8_t             pin_len;
    bool                secure;
    bt_pin_code_t       pincode;
} PINReplyEvent;

/**
 * Event for notifying pin request
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t         bd_addr;
    bt_bdname_t         bd_name;
    uint32_t            cod;
    bt_ssp_variant_t    pairing_variant;
    uint32_t            pass_key;
} SSPRequestEvent;

/**
 * Event to post ssp reply
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t         bd_addr;
    bt_bdname_t         bd_name;
    uint32_t            cod;
    bt_ssp_variant_t    pairing_variant;
    uint32_t            pass_key;
    uint8_t             accept;
} SSPReplyEvent;

/**
 * Event for notifying Device Discover
 */
typedef struct {
    BluetoothEventId event_id;
} DeviceDiscoverRequest;

/**
 * Event for notifying Device Bond
 */
typedef struct {
    BluetoothEventId    event_id;
    bt_bdaddr_t         bd_addr;
} DeviceBondRequest;

/**
 * Event for notifying Device connect
 */
typedef struct {
    BluetoothEventId    event_id;
    bt_bdaddr_t         bd_addr;
} DeviceConnectRequest;

/**
 * Event for notifying Device disconnect
 */
typedef struct {
    BluetoothEventId    event_id;
    bt_bdaddr_t         bd_addr;
} DeviceDisconnectRequest;

/**
 * API to start Profile
 */
typedef struct {
    BluetoothEventId event_id;
} ProfileStartRequest;

/**
 * Event for notifying Profile start status
 */
typedef struct {
    BluetoothEventId event_id;
    ProfileIdType    profile_id;
    bool             status;
} ProfileStartEvent;


/**
 * API to stop Profile
 */
typedef struct {
    BluetoothEventId event_id;
} ProfileStopRequest;

/**
 * API to set BT Name
 */
typedef struct {
    BluetoothEventId event_id;
    bt_property_t prop;
} SetDeviceName;
/**
 * Event for notifying Profile stop status
 */
typedef struct {
    BluetoothEventId event_id;
    ProfileIdType    profile_id;
    bool             status;
} ProfileStopEvent;

typedef struct {
    BluetoothEventId   event_id;
    bt_bdaddr_t         bd_addr;
    uint8_t*           buf_ptr;
    uint16_t           buf_size;
    uint16_t           arg1;
    uint16_t           arg2;
} A2dpSinkEvent;

typedef struct {
    BluetoothEventId   event_id;
    char codec_list[200];
} A2dpCodecListEvent;

typedef struct {
    BluetoothEventId   event_id;
    bt_bdaddr_t         bd_addr;
    ControlStatusType  status_type;
} A2dpSinkStreamingEvent;

typedef struct {
    BluetoothEventId   event_id;
    bt_bdaddr_t         bd_addr;
    uint8_t*           buf_ptr;
    uint16_t           buf_size;
    uint16_t           arg1;
    uint16_t           arg2;
} A2dpSourceEvent;

typedef struct {
    BluetoothEventId   event_id;
    uint8_t            key_id;
    bt_bdaddr_t        bd_addr;
    uint8_t*           buf_ptr;
    uint16_t           buf_size;
    btrc_player_attr_t attr_id;
    uint8_t attr_ids[BTRC_MAX_APP_SETTINGS];
    uint8_t attr_values[BTRC_MAX_APP_SETTINGS];
    uint16_t           arg1;
    uint32_t           arg2;
    uint8_t            arg3;
    uint8_t            arg4;
} AvrcpTargetEvent;

typedef struct {
    BluetoothEventId   event_id;
    bt_bdaddr_t        bd_addr;
    uint8_t*           buf_ptr;
    uint32_t*          buf_ptr32;
    uint8_t            num_attrb;
    uint8_t            arg1;
    uint8_t            arg2;
    uint16_t           arg3;
    uint32_t           arg4;
    uint32_t           arg5;
    uint64_t           arg6;
} AvrcpCtrlEvent;


typedef struct {
    BluetoothEventId    event_id;
    bt_bdaddr_t         bd_addr;
    uint8_t             key_id;
    uint8_t             arg1;
    uint8_t             arg2;
} AvrcpCtrlPassThruCmdReq;

/**
 * Event for notifying hfp client message
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t         bd_addr;
    unsigned int        peer_feat;
    unsigned int        chld_feat;
    char                str[20];
    int                 arg1;
    int                 arg2;
} HfpClientEvent;

/**
 * Event for notifying hfp ag message
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t         bd_addr;
    char                str[513];
    int                 arg1;
    int                 arg2;
    uint32_t            hdl;
    uint32_t            msg_id;
    uint32_t            data_length;
    void                *data;
} HfpAGEvent;

/**
 * Event for notifying Pan control state
 */
typedef struct {
    BluetoothEventId event_id;
    uint8_t local_role;
    uint8_t state;
    uint8_t error;
    const char *ifname;
} PanControlStateEvent;

/**
 * Event for notifying Pan connection state
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t bd_addr;
    uint8_t local_role;
    uint8_t remote_role;
    uint8_t state;
    uint8_t error;
} PanConnectionStateEvent;

/**
 * Event for notifying tethering on/off from UI
 */
typedef struct {
    BluetoothEventId event_id;
    bool is_tethering_on;
} PanSetTetheringEvent;

/**
 * Event for displaying tethering user choice & pan mode on UI
 */
typedef struct {
    BluetoothEventId event_id;
} PanGetModeEvent;

/**
 * Event for notifying Pan disconnect
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t bd_addr;
} PanDeviceDisconnectEvent;

/**
 * Event for notifying Pan connect
 */
typedef struct {
    BluetoothEventId event_id;
    bt_bdaddr_t bd_addr;
} PanDeviceConnectEvent;

/**
 * Event for notifying Pan connected device list
 */
typedef struct {
    BluetoothEventId event_id;
} PanDeviceConnectedListEvent;

/**
 * GATT client  Structures
 */
typedef struct{
    BluetoothEventId event_id;
    int status;
    int clientIf;
    bt_uuid_t *app_uuid;
} GattcRegisterAppEvent;

typedef struct{
    BluetoothEventId event_id;
    bt_bdaddr_t* bda;
    int rssi;
    uint8_t* adv_data;
} GattcScanResultEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
    int clientIf;
    bt_bdaddr_t* bda;
} GattcOpenEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
    int clientIf;
    bt_bdaddr_t* bda;
} GattcCloseEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
} GattcSearchCompleteEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    btgatt_srvc_id_t *srvc_id;
} GattcSearchResultEvent;

typedef struct{
   BluetoothEventId event_id;
   int conn_id;
   int status;
   btgatt_srvc_id_t *srvc_id;
   btgatt_gatt_id_t *char_id;
   int char_prop;
} GattcGetCharacteristicEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
    btgatt_srvc_id_t *srvc_id;
    btgatt_gatt_id_t *char_id;
    btgatt_gatt_id_t *descr_id;
} GattcGetDescriptorEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
    btgatt_srvc_id_t *srvc_id;
    btgatt_srvc_id_t *incl_srvc_id;
} GattcGetIncludedServiceEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int registered;
    int status;
    int handle;
} GattcRegisterForNotificationEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    btgatt_notify_params_t *p_data;
} GattcNotifyEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
    btgatt_read_params_t *p_data;
} GattcReadCharacteristicEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
    int handle;
} GattcWriteCharacteristicEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
} GattcExecuteWriteEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
    btgatt_read_params_t *p_data;
} GattcReadDescriptorEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
    uint16_t handle;
} GattcWriteDescriptorEvent;

typedef struct{
    BluetoothEventId event_id;
    int client_if;
    bt_bdaddr_t* bda;
    int rssi;
    int status;
} GattcRemoteRssiEvent;

typedef struct{
    BluetoothEventId event_id;
    int status;
    int client_if;
}GattcAdvertiseEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    int status;
    int mtu;
} GattcConfigureMtuEvent;

typedef struct{
    BluetoothEventId event_id;
    int action;
    int client_if;
    int status;
    int filt_type;
    int avbl_space;
} GattcScanFilterCfgEvent;

typedef struct{
    BluetoothEventId event_id;
    int action;
    int client_if;
    int status;
    int avbl_space;
} GattcScanFilterParamEvent;

typedef struct{
    BluetoothEventId event_id;
    int action;
    int client_if;
    int status;
} GattcScanFilterStatusEvent;

typedef struct{
    BluetoothEventId event_id;
    int client_if;
    int status;
} GattcMultiadvEnableEvent;

typedef struct{
    BluetoothEventId event_id;
    int client_if;
    int status;
}GattcMultiadvUpdateEvent;

typedef struct{
    BluetoothEventId event_id;
    int client_if;
    int status;
} GattcMultiadvSetadvDataEvent;

typedef struct{
    BluetoothEventId event_id;
    int client_if;
    int status;
} GattcMultiadvDisableEvent;

typedef struct{
    BluetoothEventId event_id;
    int conn_id;
    bool congested;
} GattcCongestionEvent;

typedef struct{
    BluetoothEventId event_id;
    int client_if;
    int status;
} GattcBatchscanCfgStorageEvent;

typedef struct
{
    BluetoothEventId event_id;
    int startstop_action;
    int client_if;
    int status;
} GattcBatchscanStartstopEvent;

typedef struct
{
    BluetoothEventId event_id;
    int client_if;
    int status;
    int report_format;
    int num_records;
    int data_len;
    uint8_t *p_rep_data;
} GattcBatchscanReportsEvent;

typedef struct
{
    BluetoothEventId event_id;
    int client_if;
} GattcBatchscanThresholdEvent;

typedef struct
{
    BluetoothEventId event_id;
    btgatt_track_adv_info_t *p_adv_track_info;
} GattcTrackAdvEventEvent;

typedef struct
{
    BluetoothEventId event_id;
    int client_if;
    btgattc_error_t status;
} GattcScanParameterSetupCompletedEvent;

typedef struct
{
    BluetoothEventId event_id;
    int conn_id;
    btgatt_db_element_t *db;
    int count;
} GattcGetGattDbEvent;

typedef struct{
    BluetoothEventId event_id;
} GattcEvent;

/* GATT Server Structures */
typedef struct {
    BluetoothEventId event_id;
    int status;
    int server_if;
    bt_uuid_t *uuid;
} GattsRegisterAppEvent;

typedef struct {
    BluetoothEventId event_id;
    int conn_id;
    int server_if;
    int connected;
    bt_bdaddr_t *bda;
} GattsConnectionEvent;

typedef struct {
    BluetoothEventId event_id;
    int status;
    int server_if;
    btgatt_srvc_id_t *srvc_id;
    int srvc_handle;
} GattsServiceAddedEvent;

typedef struct
{
    BluetoothEventId event_id;
    int status;
    int server_if;
    int srvc_handle;
    int incl_srvc_handle;
} GattsIncludedServiceAddedEvent;

typedef struct
{
    BluetoothEventId event_id;
    int status;
    int server_if;
    bt_uuid_t *char_id;
    int srvc_handle;
    int char_handle;
} GattsCharacteristicAddedEvent;

typedef struct
{
    BluetoothEventId event_id;
    int status;
    int server_if;
    bt_uuid_t *descr_id;
    int srvc_handle;
    int descr_handle;
} GattsDescriptorAddedEvent;

typedef struct
{
    BluetoothEventId event_id;
    int status;
    int server_if;
    int srvc_handle;
} GattsServiceStartedEvent;

typedef struct
{
    BluetoothEventId event_id;
    int status;
    int server_if;
    int srvc_handle;
} GattsServiceStoppedEvent;

typedef struct
{
    BluetoothEventId event_id;
    int status;
    int server_if;
    int srvc_handle;
} GattsServiceDeletedEvent;

typedef struct
{
    BluetoothEventId event_id;
    int conn_id;
    int trans_id;
    bt_bdaddr_t *bda;
    int attr_handle;
    int offset;
    bool is_long;
} GattsRequestReadEvent;

typedef struct
{
    BluetoothEventId event_id;
    int conn_id;
    int trans_id;
    bt_bdaddr_t *bda;
    int attr_handle;
    int offset;
    int length;
    bool need_rsp;
    bool is_prep;
    uint8_t* value;
} GattsRequestWriteEvent;

typedef struct
{
    BluetoothEventId event_id;
    int conn_id;
    int trans_id;
    bt_bdaddr_t *bda;
    int exec_write;
} GattsRequestExecWriteEvent;

typedef struct
{
    BluetoothEventId event_id;
    int status;
    int handle;
} GattsResponseConfirmationEvent;

typedef struct
{
    BluetoothEventId event_id;
    int conn_id;
    int status;
} GattsIndicationSentEvent;

typedef struct
{
    BluetoothEventId event_id;
    int conn_id;
    bool congested;
} GattsCongestionEvent;

typedef struct
{
    BluetoothEventId event_id;
    int conn_id;
    int mtu;
} GattsMTUchangedEvent;

/* Remote start profile support */
typedef struct {
    BluetoothEventId event_id;
    bt_uuid_t server_uuid;
    bt_uuid_t client_uuid;
    bt_uuid_t service_uuid;
    bt_uuid_t characteristics_uuid;
    bt_uuid_t descriptor_uuid;
} RspEnableEvent;

typedef struct {
    BluetoothEventId event_id;
    int server_if;
} RspDisableEvent;

typedef struct {
    BluetoothEventId event_id;
    int server_if;
} RspAddServiceEvent;

/** Callback for SDP search */
typedef void (*SdpSearchCb)(bt_status_t status, bt_bdaddr_t *bd_addr, uint8_t* uuid,
    bluetooth_sdp_record *record, bool more_result);

typedef void (*SdpAddRecordCb)(bt_status_t status, int handle);

typedef void (*SdpRemoveRecordCb)(bt_status_t status);

typedef struct {
    BluetoothEventId        event_id;
    bt_bdaddr_t             bd_addr;
    uint8_t                 *uuid;
    SdpSearchCb             searchCb;
    SdpAddRecordCb          addRecordCb;
    SdpRemoveRecordCb       removeRecordCb;
    bluetooth_sdp_record    record;
    int                     rec_handle;
} SdpClientEvent;

#ifdef USE_BT_OBEX
typedef struct {
    BluetoothEventId    event_id;
    bt_bdaddr_t         bd_addr;
    char                value[256];
    uint32_t            max_list_count;
    uint32_t            list_start_offset;
} PbapClientEvent;

typedef struct {
    BluetoothEventId    event_id;
    bt_bdaddr_t         bd_addr;
    char                value[256];
    bool                accept;
} OppEvent;
#endif

/**
  * @brief BT IPC message between qcbtdaemon & btapp
  */
typedef struct{
    /**
     * It can be any value of bt_ipc_type
     */
    uint8_t type;
    /**
     * It can be any value of bt_ipc_status
     */
    uint8_t status;
} BtIpcMsg;

typedef struct {
    BluetoothEventId event_id;
    BtIpcMsg ipc_msg;
} BtIpcMsgEvent;

typedef struct {
    BluetoothEventId   event_id;
    ProfileIdType      profile_id;
    ControlRequestType request_type;
} BTAMControlRequest;

typedef struct {
    BluetoothEventId   event_id;
    ControlStatusType  status_type;
} BTAMControlStatus;

typedef struct {
    BluetoothEventId   event_id;
    ProfileIdType      profile_id;
} BTAMControlRelease;

typedef union {
    BluetoothEventId                        event_id;
    GapAppEvent                             state_event;
    SSPRequestEvent                         ssp_request_event;
    SSPReplyEvent                           ssp_reply_event;
    PINRequestEvent                         pin_request_event;
    PINReplyEvent                           pin_reply_event;
    DiscoveryStateEvent                     discovery_state_event;
    ACLStateEvent                           acl_state_event;
    DeviceBondStateEvent                    bond_state_event;
    DeviceBondStateEventInt                 bond_state_event_int;
    DeviceFoundEvent                        device_found_event;
    DeviceFoundEventInt                     device_found_event_int;
    SetDeviceName                           set_device_name_event;
    RemotePropertiesEvent                   remote_properties_event;
    AdapterPropertiesEvent                  adapater_properties_event;
    DeviceDiscoverRequest                   discover_request;
    DeviceBondRequest                       bond_device;
    ProfileStartRequest                     profile_start_request;
    ProfileStopRequest                      profile_stop_request;
    ProfileStartEvent                       profile_start_event;
    ProfileStopEvent                        profile_stop_event;
    A2dpSinkEvent                           a2dpSinkEvent;
    A2dpSinkStreamingEvent                  a2dpSinkStreamingEvent;
    AvrcpCtrlPassThruCmdReq                 avrcpCtrlPassThruEvent;
    A2dpCodecListEvent                      a2dpCodecListEvent;
    A2dpSourceEvent                         a2dpSourceEvent;
    AvrcpCtrlEvent                          avrcpCtrlEvent;
    AvrcpTargetEvent                        avrcpTargetEvent;
    HfpClientEvent                          hfp_client_event;
    HfpAGEvent                              hfp_ag_event;
    BTAMControlRequest                      btamControlReq;
    BTAMControlStatus                       btamControlStatus;
    BTAMControlRelease                      btamControlRelease;

    PanControlStateEvent                    pan_control_state_event;
    PanConnectionStateEvent                 pan_connection_state_event;
    PanSetTetheringEvent                    pan_set_tethering_event;
    PanGetModeEvent                         pan_get_mode_event;
    PanDeviceDisconnectEvent                pan_device_disconnect_event;
    PanDeviceConnectEvent                   pan_device_connect_event;
    PanDeviceConnectedListEvent             pan_device_connected_list_event;

    GattsRegisterAppEvent                   gatts_register_app_event;
    GattsConnectionEvent                    gatts_connection_event;
    GattsServiceAddedEvent                  gatts_service_added_event;
    GattsIncludedServiceAddedEvent          gatts_included_service_added_event;
    GattsCharacteristicAddedEvent           gatts_characteristic_added_event;
    GattsDescriptorAddedEvent               gatts_descriptor_added_event;
    GattsServiceStartedEvent                gatts_service_started_event;
    GattsServiceStoppedEvent                gatts_service_stopped_event;
    GattsServiceDeletedEvent                gatts_service_deleted_event;
    GattsRequestReadEvent                   gatts_request_read_event;
    GattsRequestWriteEvent                  gatts_request_write_event;
    GattsRequestExecWriteEvent              gatts_request_exec_write_event;
    GattsResponseConfirmationEvent          gatts_response_confirmation_event;
    GattsIndicationSentEvent                gatts_indication_sent_event;
    GattsCongestionEvent                    gatts_congestion_event;
    GattsMTUchangedEvent                    gatts_mtu_changed_event;

    GattcRegisterAppEvent                   gattc_register_app_event;
    GattcScanResultEvent                    gattc_scan_result_event;
    GattcOpenEvent                          gattc_open_event;
    GattcCloseEvent                         gattc_close_event;
    GattcSearchCompleteEvent                gattc_search_complete_event;
    GattcSearchResultEvent                  gattc_search_result_event;
    GattcGetCharacteristicEvent             gattc_get_characteristic_event;
    GattcGetDescriptorEvent                 gattc_get_descriptor_event;
    GattcGetIncludedServiceEvent            gattc_get_included_service_event;
    GattcRegisterForNotificationEvent       gattc_register_for_notification_event;
    GattcNotifyEvent                        gattc_notify_event;
    GattcReadCharacteristicEvent            gattc_read_characteristic_event;
    GattcWriteCharacteristicEvent           gattc_write_characteristic_event;
    GattcReadDescriptorEvent                gattc_read_descriptor_event;
    GattcWriteDescriptorEvent               gattc_write_descriptor_event;
    GattcExecuteWriteEvent                  gattc_execute_write_event;
    GattcRemoteRssiEvent                    gattc_remote_rssi_event;
    GattcAdvertiseEvent                     gattc_advertise_event;
    GattcConfigureMtuEvent                  gattc_configure_mtu_event;
    GattcScanFilterCfgEvent                 gattc_scan_filter_cfg_event;
    GattcScanFilterParamEvent               gattc_scan_filter_param_event;
    GattcScanFilterStatusEvent              gattc_scan_filter_status_event;
    GattcMultiadvEnableEvent                gattc_multiadv_enable_event;
    GattcMultiadvUpdateEvent                gattc_multiadv_update_event;
    GattcMultiadvSetadvDataEvent            gattc_multiadv_setadv_data_event;
    GattcMultiadvDisableEvent               gattc_multiadv_disable_event;
    GattcCongestionEvent                    gattc_congestion_event;
    GattcBatchscanCfgStorageEvent           gattc_batchscan_cfg_storage_event;
    GattcBatchscanStartstopEvent            gattc_batchscan_startstop_event;
    GattcBatchscanReportsEvent              gattc_batchscan_reports_event;
    GattcBatchscanThresholdEvent            gattc_batchscan_threshold_event;
    GattcTrackAdvEventEvent                 gattc_track_adv_event_event;
    GattcScanParameterSetupCompletedEvent   gattc_scan_parameter_setup_completed_event;
    GattcGetGattDbEvent                     gattc_get_gatt_db_event;

    RspEnableEvent                          rsp_enable_event;
    RspDisableEvent                         rsp_disable_event;
    SdpClientEvent                          sdp_client_event;
#ifdef USE_BT_OBEX
    PbapClientEvent                         pbap_client_event;
    OppEvent                                opp_event;
#endif
    BtIpcMsgEvent                           bt_ipc_msg_event;
} BtEvent;

/**
  * @brief BT IPC message type
  */
typedef enum{
    /**
     * ipc message to enable tethering
     */
    BT_IPC_ENABLE_TETHERING = 0x01,
    /**
     * ipc message to disable tethering
     */
    BT_IPC_DISABLE_TETHERING,
    /**
     * ipc message to enable reverse tethering
     */
    BT_IPC_ENABLE_REVERSE_TETHERING,
    /**
     * ipc message to disable reverse tethering
     */
    BT_IPC_DISABLE_REVERSE_TETHERING,
    /**
     * ipc message to start WLAN
     */
    BT_IPC_REMOTE_START_WLAN,

    BT_IPC_INAVALID = 0xFF
} bt_ipc_type;

/**
 * @brief BT IPC message status
 */
typedef enum{
    SUCCESS = 0x00,
    FAILED,
    INITIATED,
    INVALID = 0xFF
} bt_ipc_status;

#ifdef __cplusplus
extern "C" {
#endif

typedef char bdstr_t[MAX_BD_STR_LEN];
void PostMessage(ThreadIdType thread_id, void *msg);
void BtGapMsgHandler(void *context);
void BtMainMsgHandler(void *context);
void BtSocketMsgHandler (void *context);
void BtA2dpSinkMsgHandler(void *msg);
void BtPanMsgHandler(void *context);
void BtGattMsgHandler(void *context);
void BtHfpClientMsgHandler (void *context);
void BtHfpAgMsgHandler (void *context);
void BtAudioManagerHandler(void *msg);
void BtSdpClientMsgHandler(void *context);
void BtAvrcpMsgHandler(void *msg);
#ifdef USE_BT_OBEX
void BtPbapClientMsgHandler(void *context);
void BtOppMsgHandler(void *context);
#endif
void BtA2dpSourceMsgHandler(void *msg);
#ifdef __cplusplus
}
#endif

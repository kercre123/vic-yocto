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


#ifndef BT_APP_HPP
#define BT_APP_HPP

#include "osi/include/thread.h"
#include "osi/include/reactor.h"
#include "osi/include/alarm.h"
#include "osi/include/config.h"
#include "gap/include/Gap.hpp"
#include <hardware/bluetooth.h>
#include "include/ipc.h"
#include "utils.h"
#include "Rsp.hpp"

#include <cutils/sockets.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif

/**
 * @file Main.hpp
 * @brief Main header file for the BT application
*/

/**
 * Maximum argument length
 */
#define COMMAND_ARG_SIZE     200

/**
 * Maximum command length
 */
#define COMMAND_SIZE        200

/**
 * Maximum arguments count
 */
#define MAX_ARGUMENTS        20 //TODO

/**
 * Maximum sub-arguments count
 */
#define MAX_SUB_ARGUMENTS    10


#define BTM_MAX_LOC_BD_NAME_LEN     248

/**
 * Macro used to find the total commands number
 */
#define  NO_OF_COMMANDS(x)  (sizeof(x) / sizeof((x)[0]))

/**
 * The Configuration options
 */
const char *BT_SOCKET_ENABLED      = "BtSockInputEnabled";
const char *BT_ENABLE_DEFAULT      = "BtEnableByDefault";
const char *BT_USER_INPUT          = "UserInteractionNeeded";
const char *BT_A2DP_SINK_ENABLED   = "BtA2dpSinkEnable";
const char *BT_A2DP_SOURCE_ENABLED = "BtA2dpSourceEnable";
const char *BT_HFP_CLIENT_ENABLED  = "BtHfClientEnable";
const char *BT_HFP_AG_ENABLED      = "BtHfpAGEnable";
const char *BT_AVRCP_ENABLED       = "BtAvrcpEnable";
const char *BT_ENABLE_EXT_POWER    = "BtEnableExtPower";
const char *BT_ENABLE_FW_SNOOP     = "BtEnableFWSnoop";
const char *BT_ENABLE_SOC_LOG      = "BtEnableSocLog";
/**
 * The Configuration file path
 */
const char *CONFIG_FILE_PATH       = "/data/misc/bluetooth/bt_app.conf";

/**
 * To track user command status
 */
typedef enum  {
    COMMAND_NONE = 0,
    COMMAND_INPROGRESS,
    COMMAND_COMPLETE,
} CommandStatus;

/**
 * To track user command status
 */
typedef struct {
    CommandStatus enable_cmd;
    CommandStatus enquiry_cmd;
    CommandStatus stop_enquiry_cmd;
    CommandStatus disable_cmd;
    CommandStatus pairing_cmd;
} UiCommandStatus;

/**
 * list of supported commands
 */
typedef enum {
    BT_ENABLE,
    BT_DISABLE,
    START_ENQUIRY,
    CANCEL_ENQUIRY,
    MAIN_EXIT,
    START_PAIR,
    INQUIRY_LIST,
    BONDED_LIST,
    GET_BT_NAME,
    GET_BT_ADDR,
    SET_BT_NAME,
    UNPAIR,
    GET_BT_STATE,
    TEST_MODE,
    GAP_OPTION,
    TEST_ON_OFF,
    A2DP_SINK,
    A2DP_SOURCE,
    CONNECT,
    DISCONNECT,
    PLAY,
    PAUSE,
    STOP,
    FASTFORWARD,
    REWIND,
    FORWARD,
    BACKWARD,
    VOL_UP,
    VOL_DOWN,
    CODEC_LIST,
    TRACK_CHANGE,
    SET_ABS_VOL,
    SEND_VOL_UP_DOWN,
    VOL_CHANGED_NOTI,
    GET_CAP,
    LIST_PLAYER_SETTING_ATTR,
    LIST_PALYER_SETTING_VALUE,
    GET_PALYER_APP_SETTING,
    SET_PALYER_APP_SETTING,
    GET_ELEMENT_ATTR,
    GET_PLAY_STATUS,
    SET_ADDRESSED_PLAYER,
    SET_BROWSED_PLAYER,
    CHANGE_PATH,
    GETFOLDERITEMS,
    GETITEMATTRIBUTES,
    PLAYITEM,
    ADDTONOWPLAYING,
    SEARCH,
    REG_NOTIFICATION,
    ADDR_PLAYER_CHANGE,
    AVAIL_PLAYER_CHANGE,
    SET_EQUALIZER_VAL,
    SET_REPEAT_VAL,
    SET_SHUFFLE_VAL,
    SET_SCAN_VAL,
    BIGGER_METADATA,
    PAN_OPTION,
    CONNECTED_LIST,
    SET_TETHERING,
    GET_PAN_MODE,
    RSP_OPTION,
    RSP_INIT,
    RSP_START,
#ifdef USE_BT_OBEX
    PBAP_CLIENT_OPTION,
    PBAP_REGISTER,
    PBAP_GET_PHONEBOOK_SIZE,
    PBAP_GET_PHONEBOOK,
    PBAP_GET_VCARD,
    PBAP_GET_VCARD_LISTING,
    PBAP_SET_PATH,
    PBAP_ABORT,
    PBAP_SET_FILTER,
    PBAP_SET_ORDER,
    PBAP_SET_SEARCH_ATTRIBUTE,
    PBAP_SET_SEARCH_VALUE,
    PBAP_SET_PHONE_BOOK,
    PBAP_SET_REPOSITORY,
    PBAP_SET_VCARD_FORMAT,
    PBAP_SET_LIST_COUNT,
    PBAP_SET_START_OFFSET,
    PBAP_GET_FILTER,
    PBAP_GET_ORDER,
    PBAP_GET_SEARCH_ATTRIBUTE,
    PBAP_GET_PHONE_BOOK,
    PBAP_GET_REPOSITORY,
    PBAP_GET_VCARD_FORMAT,
    PBAP_GET_LIST_COUNT,
    PBAP_GET_START_OFFSET,
    OPP_OPTION,
    OPP_REGISTER,
    OPP_SEND,
    OPP_ABORT,
#endif
    HFP_CLIENT,
    CREATE_SCO_CONN,
    DESTROY_SCO_CONN,
    ACCEPT_CALL,
    REJECT_CALL,
    END_CALL,
    HOLD_CALL,
    RELEASE_HELD_CALL,
    RELEASE_ACTIVE_ACCEPT_WAITING_OR_HELD_CALL,
    SWAP_CALLS,
    ADD_HELD_CALL_TO_CONF,
    RELEASE_SPECIFIED_ACTIVE_CALL,
    PRIVATE_CONSULTATION_MODE,
    PUT_INCOMING_CALL_ON_HOLD,
    ACCEPT_HELD_INCOMING_CALL,
    REJECT_HELD_INCOMING_CALL,
    DIAL,
    REDIAL,
    DIAL_MEMORY,
    START_VR,
    STOP_VR,
    CALL_ACTION,
    QUERY_CURRENT_CALLS,
    QUERY_OPERATOR_NAME,
    QUERY_SUBSCRIBER_INFO,
    SCO_VOL_CTRL,
    MIC_VOL_CTRL,
    SPK_VOL_CTRL,
    SEND_DTMF,
    DISABLE_NREC_ON_AG,
    SEND_AT_CMD,
    HFP_AG,
    BACK_TO_MAIN,
    END,
} CommandList;

/**
 * Argument Count
 */
typedef enum {
    ZERO_PARAM,
    ONE_PARAM,
    TWO_PARAM,
    THREE_PARAM,
    FOUR_PARAM,
    FIVE_PARAM,
    SIX_PARAM,
} MaxParamCount;

typedef enum {
    MAIN_MENU,
    GAP_MENU,
    TEST_MENU,
    A2DP_SINK_MENU,
    HFP_CLIENT_MENU,
    PAN_MENU,
    RSP_MENU,
#ifdef USE_BT_OBEX
    PBAP_CLIENT_MENU,
    OPP_MENU,
#endif
    HFP_AG_MENU,
    A2DP_SOURCE_MENU
} MenuType;

/**
 * Default menu_type is Main Menu
 */
MenuType menu_type = MAIN_MENU;

/**
 * UserMenuList
 */
typedef struct {
    CommandList cmd_id;
    const char cmd_name[COMMAND_SIZE];
    MaxParamCount max_param;
    const char cmd_help[COMMAND_SIZE];
} UserMenuList;

/**
 * list of supported commands for GAP
 */
UserMenuList GapMenu[] = {
    {BT_ENABLE,             "enable",           ZERO_PARAM,    "enable"},
    {BT_DISABLE,            "disable",          ZERO_PARAM,    "disable"},
    {START_ENQUIRY,         "inquiry",          ZERO_PARAM,    "inquiry"},
    {CANCEL_ENQUIRY,        "cancel_inquiry",   ZERO_PARAM,    "cancel_inquiry"},
    {START_PAIR,            "pair",             ONE_PARAM,    "pair<space><bt_address> \
    eg. pair 00:11:22:33:44:55"},
    {UNPAIR,                "unpair",           ONE_PARAM,    "unpair<space><bt_address> \
    eg. unpair 00:11:22:33:44:55"},
    {INQUIRY_LIST,          "inquiry_list",     ZERO_PARAM,    "inquiry_list"},
    {BONDED_LIST,           "bonded_list",      ZERO_PARAM,    "bonded_list"},
    {GET_BT_STATE,          "get_state",        ZERO_PARAM,    "get_state"},
    {GET_BT_NAME,           "get_bt_name",      ZERO_PARAM,    "get_bt_name"},
    {GET_BT_ADDR,           "get_bt_address",   ZERO_PARAM,    "get_bt_address"},
    {SET_BT_NAME,           "set_bt_name",      ONE_PARAM,    "set_bt_name<space><bt name> \
    eg. set_bt_name MDM_Fluoride"},
    {BACK_TO_MAIN,          "main_menu",        ZERO_PARAM,    "main_menu"},
};

/**
 * list of supported commands for Main Menu
 */
UserMenuList MainMenu[] = {
    {GAP_OPTION,            "gap_menu",         ZERO_PARAM,   "gap_menu"},
    {PAN_OPTION,            "pan_menu",         ZERO_PARAM,   "pan_menu"},
    {RSP_OPTION,            "rsp_menu",         ZERO_PARAM,   "rsp_menu"},
    {TEST_MODE,             "test_menu",        ZERO_PARAM,   "test_menu"},
    {A2DP_SINK,             "a2dp_sink_menu",   ZERO_PARAM,   "a2dp_sink_menu"},
    {HFP_CLIENT,            "hfp_client_menu",  ZERO_PARAM,   "hfp_client_menu"},
#ifdef USE_BT_OBEX
    {PBAP_CLIENT_OPTION,    "pbap_client_menu", ZERO_PARAM,   "pbap_client_menu"},
    {OPP_OPTION,            "opp_menu",         ZERO_PARAM,   "opp_menu"},
#endif
    {HFP_AG,                "hfp_ag_menu",      ZERO_PARAM,   "hfp_ag_menu"},
    {A2DP_SOURCE,           "a2dp_source_menu", ZERO_PARAM,   "a2dp_source_menu"},
    {MAIN_EXIT,             "exit",             ZERO_PARAM,   "exit"},
};

/**
 * list of supported commands for PAN
 */
UserMenuList PanMenu[] = {
    {SET_TETHERING,  "enable_tethering",         ONE_PARAM, \
    "enable_tethering<space><true or false> eg. enable_tethering true"},
    {GET_PAN_MODE,   "get_mode",                 ZERO_PARAM, "get_mode"},
    {CONNECT,        "connect",                  ONE_PARAM, \
    "connect<space><bt_address> eg. connect 00:11:22:33:44:55"},
    {DISCONNECT,     "disconnect",               ONE_PARAM, \
    "disconnect<space><bt_address> eg. disconnect 00:11:22:33:44:55"},
    {CONNECTED_LIST, "connected_device_list",    ZERO_PARAM, "connected_device_list"},
    {BACK_TO_MAIN,   "main_menu",                ZERO_PARAM, "main_menu"},
};
/**
 * list of supported commands for Test Menu
 */
UserMenuList TestMenu[] = {
    {TEST_ON_OFF,           "on_off",    ONE_PARAM,     "<on_off> <number>   eg: on_off 100"},
    {BACK_TO_MAIN,          "main_menu", ZERO_PARAM,    "main_menu"},
};

/**
 * list of supported commands for RSP Menu
 */
UserMenuList RspMenu[] = {
    {RSP_INIT,              "rsp_init",  ZERO_PARAM,    "rsp_init (only for Init time)"},
    {RSP_START,             "rsp_start", ZERO_PARAM,    "rsp_start would (re)start adv"},
    {BACK_TO_MAIN,          "main_menu",  ZERO_PARAM, "main_menu"},
};

/**
 * list of supported commands for A2DP_SINK Menu
 */
UserMenuList A2dpSinkMenu[] = {
    {CONNECT,               "connect",          ONE_PARAM,    "connect<space><bt_address>"},
    {DISCONNECT,            "disconnect",       ONE_PARAM,    "disconnect<space><bt_address>"},
    {PLAY,                  "play",             ONE_PARAM,    "play<space><bt_address>"},
    {PAUSE,                 "pause",            ONE_PARAM,    "pause<space><bt_address>"},
    {STOP,                  "stop",             ONE_PARAM,    "stop<space><bt_address>"},
    {REWIND,                "rewind",           ONE_PARAM,    "rewind<space><bt_address>"},
    {FASTFORWARD,           "fastforward",      ONE_PARAM,    "fastforward<space><bt_address>"},
    {FORWARD,               "forward",          ONE_PARAM,    "forward<space><bt_address>"},
    {BACKWARD,              "backward",         ONE_PARAM,    "backward<space><bt_address>"},
    {VOL_UP,                "volup",            ONE_PARAM,    "volup<space><bt_address>"},
    {VOL_DOWN,              "voldown",          ONE_PARAM,    "voldown<space><bt_address>"},
    {VOL_CHANGED_NOTI,      "volchangednoti",   ONE_PARAM,    "volchangednoti<space><vol level>"},
    {CODEC_LIST,        "codec_list",       ONE_PARAM,  "codec_list<space><codec1,param1,"
        "param2,codec2,param1,param2,....>"},
    {GET_CAP,               "getcap",           TWO_PARAM,    "getcap<space><bt_address><space><cap_ID>"},
    {LIST_PLAYER_SETTING_ATTR,      "listplayersettingattr",   ONE_PARAM,    "listplayersettingattr<space><bt_address>"},
    {LIST_PALYER_SETTING_VALUE,     "listplayersettingvalue",  TWO_PARAM,    "listplayersettingvalue<space><bt_address><space><attri_ID>"},
    {GET_PALYER_APP_SETTING,    "getplayersetting",  TWO_PARAM,  "getplayersetting<space><bt_address><space><attri_IDs>"},
    {SET_PALYER_APP_SETTING,    "setplayersetting",  THREE_PARAM,  "setplayersetting<space><bt_address><space><attri_IDs><space><attri_Values>"},
    {GET_ELEMENT_ATTR,  "getelementattr",  TWO_PARAM,  "getelementattr<space><bt_address><space><attribute_IDs>"},
    {GET_PLAY_STATUS,   "getplayerstatus",  ONE_PARAM,  "getplayerstatus<space><bt_address>"},
    {REG_NOTIFICATION,  "regnotification",  TWO_PARAM,  "regnotification<space><bt_address><space><event_ID>"},
    {SET_ADDRESSED_PLAYER,  "setaddressedplayer",  TWO_PARAM,  "setaddressedplayer<space><bt_address><space><player_ID>"},
    {SET_BROWSED_PLAYER,  "setbrowsedplayer",  TWO_PARAM,  "setbrowsedplayer<space><bt_address><space><player_ID>"},
    {CHANGE_PATH,  "changepath",  THREE_PARAM,  "changepath<space><bt_address><space><direction><space><folder_uID>"},
    {GETFOLDERITEMS,  "getfolderitems",  SIX_PARAM,  "getfolderitems<space><bt_address><space><scopeID><space><startItem><space><endItem><space><num_attrb><space><attrib_IDs>"},
    {GETITEMATTRIBUTES,  "getitemattributes",  SIX_PARAM,  "getitemattributes<space><bt_address><space><scopeID><space><uID><space><uID_Counter><space><num_attrb><space><attrib_IDs>"},
    {PLAYITEM,  "playitem",  FOUR_PARAM,  "playitem<space><bt_address><space><scopeID><space><uID><space><uID_Counter>"},
    {ADDTONOWPLAYING,  "addtonowplaying",  FOUR_PARAM,  "addtonowplaying<space><bt_address><space><scopeID><space><uID><space><uID_Counter>"},
    {SEARCH,  "search",  THREE_PARAM,  "search<space><bt_address><space><length><space><string>"},
    {BACK_TO_MAIN,          "main_menu",        ZERO_PARAM,   "main_menu"},
};

/**
 * list of supported commands for A2DP_SOURCE Menu
 */
UserMenuList A2dpSourceMenu[] = {
    {CONNECT,               "connect",          ONE_PARAM,    "connect<space><bt_address>"},
    {DISCONNECT,            "disconnect",       ONE_PARAM,    "disconnect<space><bt_address>"},
    {PLAY,                  "start",            ZERO_PARAM,   "start"},
    {PAUSE,                 "suspend",          ZERO_PARAM,   "suspend"},
    {STOP,                  "stop",             ZERO_PARAM,   "stop"},
    {TRACK_CHANGE,          "trackchange",      ZERO_PARAM,   "trackchange"},
    {SET_ABS_VOL,           "setabsolutevol",   ONE_PARAM,
            "setabsolutevol<space><volstep>  eg: setabsolutevol 10 (range 0-15)"},
    {SEND_VOL_UP_DOWN,      "sendvolupdown",    ONE_PARAM,
            "sendvolupdown<space><1/0>  eg: sendvolupdown 1 (1-up, 0-down)"},
    {ADDR_PLAYER_CHANGE,    "addrplayerchange", ONE_PARAM,
            "addrplayerchange<space><1/0>  eg: addrplayerchange 1 "},
    {AVAIL_PLAYER_CHANGE,   "availplayerchange",ZERO_PARAM,   "availplayerchange"},
    {BIGGER_METADATA,       "biggermetadata",   ZERO_PARAM,   "biggermetadata"},
    {CODEC_LIST,            "codec_list",       ONE_PARAM,  "codec_list<space><codec1,param1,"
        "param2,param3....,codec2,param1,param2,param3....>"},
    {SET_EQUALIZER_VAL,     "setequalizerval",  ONE_PARAM,     "setequalizerval<space><val> (1/2)"},
    {SET_REPEAT_VAL,     "setrepeatval",  ONE_PARAM,     "setrepeatval<space><val> (1 to 4)"},
    {SET_SHUFFLE_VAL,     "setshuffleval",  ONE_PARAM,     "setshuffleval<space><val>(1 to 3)"},
    {SET_SCAN_VAL,     "setscanval",  ONE_PARAM,     "setscanval<space><val> (1 to 3)"},
    {BACK_TO_MAIN,          "main_menu",        ZERO_PARAM,   "main_menu"},
};

/**
 * list of supported commands for HFP_CLIENT Menu
 */
UserMenuList HfpClientMenu[] = {
    {CONNECT,               "connect",       ONE_PARAM,    "connect<space><bt_address>"},
    {DISCONNECT,            "disconnect",    ONE_PARAM,    "disconnect<space><bt_address>"},
    {CREATE_SCO_CONN,       "create_sco",    ONE_PARAM,    "create_sco<space><bt_address>"},
    {DESTROY_SCO_CONN,      "destroy_sco",   ONE_PARAM,    "destroy_sco<space><bt_address>"},
    {ACCEPT_CALL,           "accept_call",   ZERO_PARAM,   "accept_call"},
    {REJECT_CALL,           "reject_call",   ZERO_PARAM,   "reject_call"},
    {END_CALL,              "end_call",      ZERO_PARAM,   "end_call"},
    {HOLD_CALL,             "hold_call",     ZERO_PARAM,   "hold_call"},
    {RELEASE_HELD_CALL,     "release_held_call", ZERO_PARAM,   "release_held_call"},
    {RELEASE_ACTIVE_ACCEPT_WAITING_OR_HELD_CALL,  "release_active_accept_waiting_or_held_call",
      ZERO_PARAM,"release_active_accept_waiting_or_held_call"},
    {SWAP_CALLS,            "swap_calls", ZERO_PARAM,   "swap_calls"},
    {ADD_HELD_CALL_TO_CONF, "add_held_call_to_conference", ZERO_PARAM,   "add_held_call_to_conference"},
    {RELEASE_SPECIFIED_ACTIVE_CALL, "release_specified_active_call", ONE_PARAM,
      "release_specified_active_call<space><index of the call>"},
    {PRIVATE_CONSULTATION_MODE, "private_consultation_mode", ONE_PARAM,
      "private_consultation_mode<space><index of the call>"},
    {PUT_INCOMING_CALL_ON_HOLD, "put_incoming_call_on_hold", ZERO_PARAM,   "put_incoming_call_on_hold"},
    {ACCEPT_HELD_INCOMING_CALL, "accept_held_incoming_call", ZERO_PARAM,   "accept_held_incoming_call"},
    {REJECT_HELD_INCOMING_CALL, "reject_held_incoming_call", ZERO_PARAM,   "reject_held_incoming_call"},
    {DIAL,                  "dial",          ONE_PARAM,    "dial<space><phone_number>"},
    {REDIAL,                "redial",        ZERO_PARAM,   "redial"},
    {DIAL_MEMORY,           "dial_memory",   ONE_PARAM,    "dial_memory<space><memory_location>"},
    {START_VR,              "start_vr",      ZERO_PARAM,   "start_vr"},
    {STOP_VR,               "stop_vr",       ZERO_PARAM,   "stop_vr"},
    {QUERY_CURRENT_CALLS,   "query_current_calls", ZERO_PARAM,   "query_current_calls"},
    {QUERY_OPERATOR_NAME,   "query_operator_name", ZERO_PARAM,   "query_operator_name"},
    {QUERY_SUBSCRIBER_INFO, "query_subscriber_info", ZERO_PARAM, "query_subscriber_info"},
    {MIC_VOL_CTRL,          "mic_volume_control",   ONE_PARAM,   "mic_volume_control<space><value>"},
    {SPK_VOL_CTRL,          "speaker_volume_control",   ONE_PARAM,   "speaker_volume_control<space><value>"},
    {SEND_DTMF,             "send_dtmf",   ONE_PARAM,    "send_dtmf<space><code>"},
    {DISABLE_NREC_ON_AG,    "disable_nrec_on_AG",       ZERO_PARAM,   "disable_nrec_on_AG"},
    {BACK_TO_MAIN,          "main_menu",     ZERO_PARAM,   "main_menu"},
};

#ifdef USE_BT_OBEX
/**
 * list of supported commands for PBAP_CLIENT Menu
 */
UserMenuList PbapClientMenu[] = {
    {PBAP_REGISTER,             "register",             ZERO_PARAM, "register"},
    {CONNECT,                   "connect",              ONE_PARAM,  "connect<space><bt_address>"},
    {DISCONNECT,                "disconnect",           ONE_PARAM,  "disconnect<space><bt_address>"},
    {PBAP_ABORT,                "abort",                ZERO_PARAM, "abort"},
    {PBAP_GET_PHONEBOOK_SIZE,   "get_phonebook_size",   ZERO_PARAM, "get_phonebook_size"},
    {PBAP_GET_PHONEBOOK,        "get_phonebook",        ZERO_PARAM, "get_phonebook"},
    {PBAP_GET_VCARD,            "get_vcard",            ONE_PARAM,  "get_vcard<space><vcard handle>"},
    {PBAP_GET_VCARD_LISTING,    "get_vcard_listing",    ZERO_PARAM, "get_vcard_listing"},
    {PBAP_SET_PATH,             "set_path",             ONE_PARAM,  "set_path<space><path>"},
    {PBAP_SET_PHONE_BOOK,       "set_phone_book",       ONE_PARAM,  "set_phone_book<space><phonebook>"},
    {PBAP_GET_PHONE_BOOK,       "get_phone_book",       ZERO_PARAM, "get_phone_book"},
    {PBAP_SET_REPOSITORY,       "set_repository",       ONE_PARAM,  "set_repository<space><repository>"},
    {PBAP_GET_REPOSITORY,       "get_repository",       ZERO_PARAM, "get_repository"},
    {PBAP_SET_ORDER,            "set_sort_order",       ONE_PARAM,  "set_sort_order<space><order>"},
    {PBAP_GET_ORDER,            "get_sort_order",       ZERO_PARAM, "get_sort_order"},
    {PBAP_SET_SEARCH_ATTRIBUTE, "set_search_attribute", ONE_PARAM,  "set_search_attribute<space><search_attribute>"},
    {PBAP_GET_SEARCH_ATTRIBUTE, "get_search_attribute", ZERO_PARAM, "get_search_attribute"},
    {PBAP_SET_SEARCH_VALUE,     "set_search_value",     ONE_PARAM,  "set_search_value<space><value>"},
    {PBAP_SET_FILTER,           "set_filter",           ONE_PARAM,  "set_filter<space><filter1,filter2,...>"},
    {PBAP_GET_FILTER,           "get_filter",           ZERO_PARAM, "get_filter"},
    {PBAP_SET_VCARD_FORMAT,     "set_vcard_format",     ONE_PARAM,  "set_vcard_format<space><format>"},
    {PBAP_GET_VCARD_FORMAT,     "get_vcard_format",     ZERO_PARAM, "get_vcard_format"},
    {PBAP_SET_LIST_COUNT,       "set_list_count",       ONE_PARAM,  "set_list_count<space><listcount>"},
    {PBAP_GET_LIST_COUNT,       "get_list_count",       ZERO_PARAM, "get_list_count"},
    {PBAP_SET_START_OFFSET,     "set_start_offset",     ONE_PARAM,  "set_start_offset<space><startoffset>"},
    {PBAP_GET_START_OFFSET,     "get_start_offset",     ZERO_PARAM, "get_start_offset"},
    {BACK_TO_MAIN,              "main_menu",            ZERO_PARAM, "main_menu"},
};

/**
 * list of supported commands for OPP Menu
 */
UserMenuList OppMenu[] = {
    {OPP_REGISTER,              "register",             ZERO_PARAM, "register"},
    {OPP_SEND,                  "send",                 TWO_PARAM,  "send<space><bt_address><space><file_name>"},
    {OPP_ABORT,                 "abort",                ZERO_PARAM, "abort"},
    {BACK_TO_MAIN,              "main_menu",            ZERO_PARAM, "main_menu"},
};
#endif


/**
 * list of supported commands for HFP_CLIENT Menu
 */
UserMenuList HfpAGMenu[] = {
    {CONNECT,               "connect",       ONE_PARAM,    "connect<space><bt_address>"},
    {DISCONNECT,            "disconnect",    ONE_PARAM,    "disconnect<space><bt_address>"},
    {CREATE_SCO_CONN,       "create_sco",    ONE_PARAM,    "create_sco<space><bt_address>"},
    {DESTROY_SCO_CONN,      "destroy_sco",   ONE_PARAM,    "destroy_sco<space><bt_address>"},
#if defined(BT_MODEM_INTEGRATION)
    {ACCEPT_CALL,           "accept_call",   ZERO_PARAM,   "accept_call"},
    {REJECT_CALL,           "reject_call",   ZERO_PARAM,   "reject_call"},
    {END_CALL,              "end_call",      ZERO_PARAM,   "end_call"},
    {HOLD_CALL,             "hold_call",     ZERO_PARAM,   "hold_call"},
    {RELEASE_HELD_CALL,     "release_held_call", ZERO_PARAM,   "release_held_call"},
    {SWAP_CALLS,            "swap_calls", ZERO_PARAM,   "swap_calls"},
    {ADD_HELD_CALL_TO_CONF, "add_held_call_to_conference", ZERO_PARAM,   "add_held_call_to_conference"},
    {DIAL,                  "dial",          ONE_PARAM,    "dial<space><phone_number>"},
    {QUERY_CURRENT_CALLS,   "query_current_calls", ZERO_PARAM,   "query_current_calls"},
    {QUERY_OPERATOR_NAME,   "query_operator_name", ZERO_PARAM,   "query_operator_name"},
    {QUERY_SUBSCRIBER_INFO, "query_subscriber_info", ZERO_PARAM, "query_subscriber_info"},
    {MIC_VOL_CTRL,          "mic_volume_control",   ONE_PARAM,   "mic_volume_control<space><value>"},
    {SPK_VOL_CTRL,          "speaker_volume_control",   ONE_PARAM,   "speaker_volume_control<space><value>"},
    {SEND_DTMF,             "send_dtmf",   ONE_PARAM,    "send_dtmf<space><code>"},
#endif
    {BACK_TO_MAIN,          "main_menu",     ZERO_PARAM,   "main_menu"},
};

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * @brief DisplayMenu
 *
 *  It will display list of supported commands based an argument @ref MenuType
 *
 * @param[in] menu_type @ref MenuType specify which menu commands need to display
 * @return none
 */
static void DisplayMenu(MenuType menu_type);

/**
 * @brief HandleUserInput
 *
 *  It will parse user input.
 *
 * @param[out]  int :    cmd_id has command id from @ref CommandList
 * @param[out]  char[][] : input_args contains the command and arguments
 * @param[in]  MenuType : It refers to Menu type currently user in
 */
static bool HandleUserInput (int *cmd_id, char input_args[][COMMAND_ARG_SIZE],
                                                          MenuType menu_type);
/**
 * @brief SignalHandler
 *
 *  It will handle SIGINT signal.
 *
 * @param[in]  int signal number
 * @return none
 */
static void SignalHandler(int sig);

/**
 * @brief ExitHandler
 *
 * This function can be called from main thead or signal handler thread to exit from
 * bt-app
 *
 */
static void ExitHandler(void);

/**
 * @brief HandleMainCommand
 *
 * This will handle all the commands in @ref MainMenu
 *
 * @param[in]  int cmd_id has command id from @ref CommandList
 * @param[in]  char[][] user_cmd has parsed command with arguments passed by user
 * @return none
 */
static void HandleMainCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]);

/**
 * @brief HandleTestCommand
 *
 *  This function will handle all the commands in @ref TestMenu
 *
 * @param[in] cmd_id It has command id from @ref CommandList
 * @param[in] user_cmd It has parsed commands with arguments passed by user
 * @return none
 */
static void HandleTestCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]);

/**
 * @brief HandleRspCommand
 *
 *  This function will handle all the commands in @ref RspMenu
 *
 * @param[in] cmd_id It has command id from @ref CommandList
 * @param[in] user_cmd It has parsed commands with arguments passed by user
 * @return none
 */
static void HandleRspCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]);


/**
 * @brief HandleGapCommand
 *
 *  This function will handle all the commands in @ref GapMenu
 *
 * @param[in] cmd_id It has command id from @ref CommandList
 * @param[out] user_cmd It has parsed commands with arguments passed by user
 * @return none
 */
static void HandleGapCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]);

/**
 * @brief BtCmdHandler
 *
 *  This function will take the command as input from the user. And process as per
 *  command recieved
 *
 * @param[in] context
 * @return none
 */
static void BtCmdHandler (void *context);


/**
 * @brief BtCmdHandler
 *
 *  This function will accept the events from other threads and performs action
 *  based on event
 *
 * @param[in] context
 * @return none
 */
void BtMainMsgHandler (void *context);

#ifdef __cplusplus
}
#endif

/**
 * @class BluetoothApp
 *
 * @brief This module will take inputs from command line and also from
 * socket interface. Perform action based on inputs.
 *
 */

class BluetoothApp {
  private:
    config_t *config;
    bool is_bt_enable_default_;
    bool is_user_input_enabled_;
    bool is_socket_input_enabled_;
    bool is_a2dp_sink_enabled_;
    bool is_avrcp_enabled_;
    bool is_a2dp_source_enabled_;
    bool is_hfp_client_enabled_;
    bool is_hfp_ag_enabled_;
    bool is_pan_enable_default_;
    bool is_gatt_enable_default_;
#ifdef USE_BT_OBEX
    bool is_obex_enabled_;
    bool is_pbap_client_enabled_;
    bool is_opp_enabled_;
#endif
    reactor_object_t *cmd_reactor_;
    struct hw_device_t *device_;
    bluetooth_device_t *bt_device_;
    bool LoadConfigParameters(const char *config_path);
    void InitHandler();
    void DeInitHandler();
    bool LoadBtStack();
    void UnLoadBtStack();
    int LocalSocketCreate(void);

  public:
    int listen_socket_local_;
    int client_socket_;
    bool ssp_notification;
    bool pin_notification;
#ifdef USE_BT_OBEX
    bool incoming_file_notification;
#endif

    /**
     * structure object for standard Bluetooth DM interface
     */
    const bt_interface_t *bt_interface;

    reactor_object_t *listen_reactor_;
    reactor_object_t *accept_reactor_;
    UiCommandStatus status;
    bt_state_t bt_state;
    bt_discovery_state_t bt_discovery_state;

    //key is bt_bdaddr_t storing as a string in map
    std::map < std::string, std::string> bonded_devices;
    std::map <std::string, std::string> inquiry_list;
    SSPReplyEvent   ssp_data;
    PINReplyEvent   pin_reply;

    /**
     * @brief AddFoundedDevice
     *
     *This function will stores the device discovered to @ref inquiry_list
     *
     * @param none
     * @return none
     */
    bt_bdaddr_t AddFoundedDevice(std::string bdName, const bt_bdaddr_t bd_addr);
    /**
     * @brief
     * This function will display inquiry list
     *
     * @param none
     * @return none
     */
    void PrintInquiryList();
    /**
     * @brief
     * This function will display Bonded Device list
     *
     * @param none
     * @return none
     */
    void PrintBondedDeviceList();

    /**
     * @brief
     * This function will display Bonded Device list
     *
     * @param none
     * @return none
     */
    void HandleBondState(bt_bond_state_t new_state, const bt_bdaddr_t bd_addr,
                                                    std::string bd_name);

    /**
     * @brief
     *
     * This function will call remove_bond api of stack and removes device from
     * Bonded Device list
     *
     * @param none
     * @return none
     */
    void HandleUnPair(bt_bdaddr_t bd_addr);

    /**
     * @brief Bluetooth Application Constructor
     *
     * It will initialize class members to default values and calls the
     * @ref LoadConfigParameters, this function reads config file @ref CONFIG_FILE_PATH
     */
    BluetoothApp();

    /**
     * @ref Bluetooth Application Distructor
     *
     * It will free the config parameter
     *
     */

    ~BluetoothApp();
    /**
     * @brief GetState
     *
     *  This function will returns the current BT state
     *
     * @return bt_state_t
     */
    bt_state_t GetState();
    /**
     * @brief HandleSspInput
     *
     *  This function will handles the SSP Input, it shows a message on console
     * for user input
     *
     * @return bool
     */
    bool HandleSspInput(char user_cmd[][COMMAND_ARG_SIZE]);
    /**
     * @brief HandlePinInput
     *
     *  This function will take the PIN from user
     *
     * @param[in] user_cmd
     * @return bool
     */
    bool HandlePinInput(char user_cmd[][COMMAND_ARG_SIZE]);

#ifdef USE_BT_OBEX
    /**
     * @brief HandleIncomingFile
     *
     * This function will handle the Incoming File acceptance or rejection from user,
     * it shows a message on console for user input
     *
     * @param[in] user_cmd" Can be either "accept" or "reject"
     * @return bool
     */
    bool HandleIncomingFile(char user_cmd[][COMMAND_ARG_SIZE]);
#endif

    /**
     *@brief ProcessEvent
     *
     * It will handle incomming events
     *
     * @param BtEvent
     * @return none
     */
    void ProcessEvent(BtEvent * pEvent);
};

#endif

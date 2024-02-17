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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <hardware/hardware.h>
#include <iostream>
#include <iomanip>
#include "Main.hpp"
#include "A2dp_Sink.hpp"
#include "HfpClient.hpp"
#include "Pan.hpp"
#include "Gatt.hpp"
#include "HfpAG.hpp"
#include "Audio_Manager.hpp"
#include "SdpClient.hpp"
#include "Rsp.hpp"
#ifdef USE_BT_OBEX
#include "PbapClient.hpp"
#include "Opp.hpp"
#endif
#include "osi/include/compat.h"
#include "A2dp_Src.hpp"
#include "Avrcp.hpp"

#include "utils.h"

#define LOGTAG  "MAIN "
#define LOCAL_SOCKET_NAME "/data/misc/bluetooth/btappsocket"
#define SOCKETNAME  "/data/misc/bluetooth/btprop"
static int bt_prop_socket;

extern Gap *g_gap;
extern A2dp_Sink *pA2dpSink;
extern A2dp_Source *pA2dpSource;
extern Pan *g_pan;
extern Gatt *g_gatt;
extern BT_Audio_Manager *pBTAM;
extern Rsp *rsp;

extern SdpClient *g_sdpClient;
#ifdef USE_BT_OBEX
extern PbapClient *g_pbapClient;
extern Opp *g_opp;
extern const char *BT_OBEX_ENABLED;
#endif
static BluetoothApp *g_bt_app = NULL;
extern ThreadInfo threadInfo[THREAD_ID_MAX];
extern Hfp_Client *pHfpClient;
extern Hfp_Ag *pHfpAG;
extern Avrcp *pAvrcp;
#ifdef USE_BT_OBEX
static alarm_t *opp_incoming_file_accept_timer = NULL;
#define USER_ACCEPTANCE_TIMEOUT 25000
#endif

#ifdef __cplusplus
extern "C"
{
#endif

thread_t *test_thread_id = NULL;
static void SendDisableCmdToGap();
void opensocket()
{
     int len;    /* length of sockaddr */
      struct sockaddr_un name;
      if( (bt_prop_socket = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0) {
        perror("socket");
        exit(1);
      }
      /*Create the address of the server.*/
      memset(&name, 0, sizeof(struct sockaddr_un));
      name.sun_family = AF_UNIX;
      strlcpy(name.sun_path, SOCKETNAME, sizeof(name.sun_path));
      len = sizeof(name.sun_family) + strlen(name.sun_path);
      /*Connect to the server.*/
     if (connect(bt_prop_socket, (struct sockaddr *) &name, len) < 0){
        perror("connect");
        exit(1);
      }
}
void closesocket()
{
    shutdown(bt_prop_socket, SHUT_RDWR);
    close(bt_prop_socket);
}

int property_get_bt(const char *key, char *value, const char *default_value)
{
    char prop_string[200] = {'\0'};
    int ret, bytes_read = 0, i = 0;

    snprintf(prop_string, sizeof(prop_string), "get_property %s,", key);
    ret = send(bt_prop_socket, prop_string, strlen(prop_string), 0);
    memset(value, 0, sizeof(value));
    do
    {
        bytes_read = recv(bt_prop_socket, &value[i], 1, 0);
        if (bytes_read == 1)
        {
            if (value[i] == ',')
            {
                value[i] = '\0';
                break;
            }
            i++;
        }
    } while(1);
    ALOGD("property_get_bt: key(%s) has value: %s", key, value);
    if (!i && default_value)
    {
        ALOGD("property_get_bt: Copied default =%s", default_value);
        strlcpy(value, default_value, strlen(default_value)+1);
        return 1;
    }
    return 0;
}

/* property_set_bt: returns 0 on success, < 0 on failure
*/
int property_set_bt(const char *key, const char *value)
{
    char prop_string[200] = {'\0'};
    int ret;
    snprintf(prop_string, sizeof(prop_string), "set_property %s %s,", key, value);
    ALOGD("property_set_bt: setting key(%s) to value: %s\n", key, value);
    ret = send(bt_prop_socket, prop_string, strlen(prop_string), 0);
    return 0;
}

/**
 * @brief main function
 *
 *
 *  This is main function of BT Application
 *
 * @param  argc
 * @param  *argv[]
 *
 */
int main (int argc, char *argv[]) {

    // initialize signal handler
    signal(SIGINT, SignalHandler);

    ThreadInfo *main_thread = &threadInfo[THREAD_ID_MAIN];
#ifndef USE_ANDROID_LOGGING
    openlog ("bt-app", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
#endif
    main_thread->thread_id = thread_new (main_thread->thread_name);
    if (main_thread->thread_id) {
        BtEvent *event = new BtEvent;
        event->event_id = MAIN_API_INIT;
        ALOGV (LOGTAG " Posting init to Main thread\n");
        PostMessage (THREAD_ID_MAIN, event);

        // wait for Main thread to exit
        thread_join (main_thread->thread_id);
        thread_free (main_thread->thread_id);
    }
#ifndef USE_ANDROID_LOGGING
    closelog ();
#endif
    return 0;
}

static bool HandleUserInput (int *cmd_id, char input_args[][COMMAND_ARG_SIZE],
                                                          MenuType menu_type) {
    char user_input[COMMAND_SIZE] = {'\0'};
    int index = 0 , found_index = -1, num_cmds;
    char *temp_arg = NULL;
    char delim[] = " ";
    char *ptr1;
    int param_count = 0;
    bool status = false;
    int max_param = 0;
    UserMenuList *menu = NULL;

    // validate the input string
    if(((fgets (user_input, sizeof (user_input), stdin)) == NULL) ||
       (user_input[0] == '\n')) {
        return status;
    }
    // remove trialing \n character
    user_input[strlen(user_input) - 1] = '\0';

    // According to the current menu assign Command menu
    switch(menu_type) {
        case GAP_MENU:
            menu = &GapMenu[0];
            num_cmds  = NO_OF_COMMANDS(GapMenu);
            break;
        case PAN_MENU:
            menu = &PanMenu[0];
            num_cmds  = NO_OF_COMMANDS(PanMenu);
            break;
        case TEST_MENU:
            menu = &TestMenu[0];
            num_cmds  = NO_OF_COMMANDS(TestMenu);
            break;
        case RSP_MENU:
            menu = &RspMenu[0];
            num_cmds  = NO_OF_COMMANDS(RspMenu);
            break;
        case A2DP_SINK_MENU:
            menu = &A2dpSinkMenu[0];
            num_cmds  = NO_OF_COMMANDS(A2dpSinkMenu);
            break;
        case A2DP_SOURCE_MENU:
            menu = &A2dpSourceMenu[0];
            num_cmds  = NO_OF_COMMANDS(A2dpSourceMenu);
            break;
        case HFP_CLIENT_MENU:
            menu = &HfpClientMenu[0];
            num_cmds  = NO_OF_COMMANDS(HfpClientMenu);
            break;
#ifdef USE_BT_OBEX
        case PBAP_CLIENT_MENU:
            menu = &PbapClientMenu[0];
            num_cmds  = NO_OF_COMMANDS(PbapClientMenu);
            break;
        case OPP_MENU:
            menu = &OppMenu[0];
            num_cmds  = NO_OF_COMMANDS(OppMenu);
            break;
#endif
        case HFP_AG_MENU:
            menu = &HfpAGMenu[0];
            num_cmds  = NO_OF_COMMANDS(HfpAGMenu);
            break;
        case MAIN_MENU:
        // fallback to default main menu
        default:
            menu = &MainMenu[0];
            num_cmds  = NO_OF_COMMANDS(MainMenu);
            break;
    }

    if ( (temp_arg = strtok_r(user_input, delim, &ptr1)) != NULL ) {
        // find out the command name
        for (index = 0; index < num_cmds; index++) {
            if(!strcasecmp (menu[index].cmd_name, temp_arg)) {
                *cmd_id = menu[index].cmd_id;
                found_index = index;
                strlcpy(input_args[param_count], temp_arg, COMMAND_ARG_SIZE);
                input_args[param_count++][COMMAND_ARG_SIZE - 1] = '\0';
                break;
            }
        }

        // validate the command parameters
        if (found_index != -1 ) {
            max_param = menu[found_index].max_param;
            while ((temp_arg = strtok_r(NULL, delim, &ptr1)) &&
                    (param_count < max_param + 1)) {
                strlcpy(input_args[param_count], temp_arg, COMMAND_ARG_SIZE);
                input_args[param_count++][COMMAND_ARG_SIZE - 1] = '\0';
            }

            // consider command as other param
            if(param_count == max_param + 1) {
                if(temp_arg != NULL) {
                    fprintf( stdout, " Maximum params reached\n");
                    fprintf( stdout, " Refer help: %s\n", menu[found_index].cmd_help);
                } else {
                    status = true;
                }
            } else if(param_count < max_param + 1) {
                fprintf( stdout, " Missing required parameters\n");
                fprintf( stdout, " Refer help: %s\n", menu[found_index].cmd_help);
            }
        } else {
            // to handle the paring inputs
            if(temp_arg != NULL) {
                strlcpy(input_args[param_count], temp_arg, COMMAND_ARG_SIZE);
                input_args[param_count++][COMMAND_ARG_SIZE - 1] = '\0';
            }
        }
    }
    return status;
}

static void DisplayMenu(MenuType menu_type) {

    UserMenuList *menu = NULL;
    int index = 0, num_cmds = 0;

    switch(menu_type) {
        case GAP_MENU:
            menu = &GapMenu[0];
            num_cmds  = NO_OF_COMMANDS(GapMenu);
            break;
        case PAN_MENU:
            menu = &PanMenu[0];
            num_cmds  = NO_OF_COMMANDS(PanMenu);
            break;
        case TEST_MENU:
            menu = &TestMenu[0];
            num_cmds  = NO_OF_COMMANDS(TestMenu);
            break;
        case RSP_MENU:
            menu = &RspMenu[0];
            num_cmds  = NO_OF_COMMANDS(RspMenu);
            break;
        case MAIN_MENU:
            menu = &MainMenu[0];
            num_cmds  = NO_OF_COMMANDS(MainMenu);
            break;
        case A2DP_SINK_MENU:
            menu = &A2dpSinkMenu[0];
            num_cmds  = NO_OF_COMMANDS(A2dpSinkMenu);
            break;
        case A2DP_SOURCE_MENU:
            menu = &A2dpSourceMenu[0];
            num_cmds  = NO_OF_COMMANDS(A2dpSourceMenu);
            break;
        case HFP_CLIENT_MENU:
            menu = &HfpClientMenu[0];
            num_cmds  = NO_OF_COMMANDS(HfpClientMenu);
            break;
#ifdef USE_BT_OBEX
        case PBAP_CLIENT_MENU:
            menu = &PbapClientMenu[0];
            num_cmds  = NO_OF_COMMANDS(PbapClientMenu);
            break;
        case OPP_MENU:
            menu = &OppMenu[0];
            num_cmds  = NO_OF_COMMANDS(OppMenu);
            break;
#endif
        case HFP_AG_MENU:
            menu = &HfpAGMenu[0];
            num_cmds  = NO_OF_COMMANDS(HfpAGMenu);
            break;
    }
    fprintf (stdout, " \n***************** Menu *******************\n");
    for (index = 0; index < num_cmds; index++)
        fprintf (stdout, "\t %s \n",  menu[index].cmd_help);
    fprintf (stdout, " ******************************************\n");
}

static void SignalHandler(int sig) {
    signal(SIGINT, SIG_IGN);
    ExitHandler();
}

static void ExitHandler(void) {

    // post the disable message to GAP incase BT is on
    if ( g_bt_app && g_bt_app->bt_state == BT_STATE_ON) {
        SendDisableCmdToGap();
        sleep(3);
        system("killall -KILL wcnssfilter");
        usleep(200);
    }

    // TODO to wait for complete turn off before proceeding

    if (g_bt_app) {
        BtEvent *event = new BtEvent;
        event->event_id = MAIN_API_DEINIT;
        g_bt_app->ProcessEvent (event);
        delete event;
        delete g_bt_app;
        g_bt_app = NULL;
    }

    // stop the reactor for self exit of main thread
    reactor_stop (thread_get_reactor (threadInfo[THREAD_ID_MAIN].thread_id));
}

static int GetArgsFromString(char cmdString[COMMAND_ARG_SIZE], uint8_t* nArgs){
    int     i;
    char    *p;
    char    *p_s;
    bool     cont= false;

    p_s = cmdString; i = 0;
    while(p_s)
    {
        /* skip to comma delimiter */
        for(p = p_s; *p != ',' && *p != 0; p++);

        /* get integre value */
        if (*p != 0)
        {
            *p = 0;
            cont = true;
        }
        else
            cont = false;

        nArgs[i] = atoi(p_s);

        if (cont && i < MAX_SUB_ARGUMENTS-1)
        {
            p_s = p + 1;
            i++;
        }
        else
            break;
    }
    ++i;

    for(int n=0; n < i; n++)
        ALOGV (LOGTAG " GetArgsFromString Arg%d: %d\n",n, nArgs[n]);
    ALOGV (LOGTAG " GetArgsFromString return %d\n", i);
    return i;
}

static int Get32ArgsFromString(char cmdString[COMMAND_ARG_SIZE], uint32_t* nArgs){
    int     i;
    char    *p;
    char    *p_s;
    bool     cont= false;

    p_s = cmdString; i = 0;
    while(p_s)
    {
        /* skip to comma delimiter */
        for(p = p_s; *p != ',' && *p != 0; p++);

        /* get integre value */
        if (*p != 0)
        {
            *p = 0;
            cont = true;
        }
        else
            cont = false;

        nArgs[i] = atoi(p_s);

        if (cont && i < MAX_SUB_ARGUMENTS-1)
        {
            p_s = p + 1;
            i++;
        }
        else
            break;
    }
    ++i;

    for(int n=0; n < i; n++)
        ALOGV (LOGTAG " GetArgsFromString Arg%d: %d\n",n, nArgs[n]);
    ALOGV (LOGTAG " GetArgsFromString return %d\n", i);
    return i;
}

static void HandleA2dpSinkCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {
    ALOGD(LOGTAG "HandleA2DPSinkCommand cmd_id = %d", cmd_id);
    BtEvent *event = NULL;
    uint8_t* pAttr = NULL;
    uint32_t* pAttr32 = NULL;
    uint8_t  num_Attr = 0;
    uint64_t* pData = NULL;
    switch (cmd_id) {
        case CONNECT:
        {
            bt_bdaddr_t address;
            string_to_bdaddr(user_cmd[ONE_PARAM], &address);
            if (!g_gap->IsDeviceBonded(address)) {
                fprintf( stdout, " Please pair with the device before A2DPSink connection\n");
                break;
            }
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->a2dpSinkEvent.event_id = A2DP_SINK_API_CONNECT_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->a2dpSinkEvent.bd_addr);
            PostMessage (THREAD_ID_A2DP_SINK, event);
            break;
        }
        case DISCONNECT:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->a2dpSinkEvent.event_id = A2DP_SINK_API_DISCONNECT_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->a2dpSinkEvent.bd_addr);
            PostMessage (THREAD_ID_A2DP_SINK, event);
            break;
        case PLAY:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_PASS_THRU_CMD_REQ;
            event->avrcpCtrlPassThruEvent.key_id = CMD_ID_PLAY;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlPassThruEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case PAUSE:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_PASS_THRU_CMD_REQ;
            event->avrcpCtrlPassThruEvent.key_id = CMD_ID_PAUSE;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlPassThruEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case STOP:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_PASS_THRU_CMD_REQ;
            event->avrcpCtrlPassThruEvent.key_id = CMD_ID_STOP;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlPassThruEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case FASTFORWARD:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_PASS_THRU_CMD_REQ;
            event->avrcpCtrlPassThruEvent.key_id = CMD_ID_FF;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlPassThruEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case REWIND:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_PASS_THRU_CMD_REQ;
            event->avrcpCtrlPassThruEvent.key_id = CMD_ID_REWIND;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlPassThruEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case FORWARD:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_PASS_THRU_CMD_REQ;
            event->avrcpCtrlPassThruEvent.key_id = CMD_ID_FORWARD;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlPassThruEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case BACKWARD:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_PASS_THRU_CMD_REQ;
            event->avrcpCtrlPassThruEvent.key_id = CMD_ID_BACKWARD;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlPassThruEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case VOL_UP:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_PASS_THRU_CMD_REQ;
            event->avrcpCtrlPassThruEvent.key_id = CMD_ID_VOL_UP;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlPassThruEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case VOL_DOWN:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_PASS_THRU_CMD_REQ;
            event->avrcpCtrlPassThruEvent.key_id = CMD_ID_VOL_DOWN;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlPassThruEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case VOL_CHANGED_NOTI:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_VOL_CHANGED_NOTI_REQ;
            event->avrcpCtrlPassThruEvent.arg1 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case GET_CAP:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_GET_CAP_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg1 = atoi(user_cmd[TWO_PARAM]);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case LIST_PLAYER_SETTING_ATTR:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_LIST_PALYER_SETTING_ATTR_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case LIST_PALYER_SETTING_VALUE:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_LIST_PALYER_SETTING_VALUE_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg1 = atoi(user_cmd[TWO_PARAM]);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case GET_PALYER_APP_SETTING:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            pAttr = new uint8_t[MAX_SUB_ARGUMENTS];
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_GET_PALYER_APP_SETTING_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            num_Attr = GetArgsFromString(user_cmd[TWO_PARAM],pAttr);
            if(num_Attr)
            {
                event->avrcpCtrlEvent.num_attrb = num_Attr;
                event->avrcpCtrlEvent.buf_ptr = pAttr;
                PostMessage (THREAD_ID_AVRCP, event);
            }
            break;
        case SET_PALYER_APP_SETTING:
            {
                event = new BtEvent;
                memset(event, 0, sizeof(BtEvent));
                pAttr = new uint8_t[MAX_SUB_ARGUMENTS];
                uint8_t* pValue = new uint8_t[MAX_SUB_ARGUMENTS];
                event->avrcpCtrlEvent.event_id = AVRCP_CTRL_SET_PALYER_APP_SETTING_VALUE_REQ;
                string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
                num_Attr = GetArgsFromString(user_cmd[TWO_PARAM],pAttr);
                GetArgsFromString(user_cmd[THREE_PARAM],pValue);
                if(num_Attr)
                {
                    event->avrcpCtrlEvent.num_attrb = num_Attr;
                    event->avrcpCtrlEvent.buf_ptr = pAttr;
                    event->avrcpCtrlEvent.arg6 = (uint64_t)pValue;
                    PostMessage (THREAD_ID_AVRCP, event);
                }
                break;
            }

        case GET_ELEMENT_ATTR:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            pAttr32 = new uint32_t[MAX_SUB_ARGUMENTS];
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_GET_ELEMENT_ATTR_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            num_Attr = Get32ArgsFromString(user_cmd[TWO_PARAM],pAttr32);
            if(num_Attr)
            {
                event->avrcpCtrlEvent.num_attrb = num_Attr;
                event->avrcpCtrlEvent.buf_ptr32 = pAttr32;
                PostMessage (THREAD_ID_AVRCP, event);
            }
            break;
        case GET_PLAY_STATUS:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_GET_PLAY_STATUS_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case SET_ADDRESSED_PLAYER:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_SET_ADDRESSED_PLAYER_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg3 = atoi(user_cmd[TWO_PARAM]);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case SET_BROWSED_PLAYER:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_SET_BROWSED_PLAYER_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg3 = atoi(user_cmd[TWO_PARAM]);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case CHANGE_PATH:{
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_CHANGE_PATH_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg1 = atoi(user_cmd[TWO_PARAM]);
            event->avrcpCtrlEvent.arg6 = strtoull(user_cmd[THREE_PARAM],NULL,10);

            PostMessage (THREAD_ID_AVRCP, event);
            }
            break;
        case GETFOLDERITEMS:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_GET_FOLDER_ITEMS_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg1 = atoi(user_cmd[TWO_PARAM]);
            event->avrcpCtrlEvent.arg4 = atoi(user_cmd[THREE_PARAM]);
            event->avrcpCtrlEvent.arg5 = atoi(user_cmd[FOUR_PARAM]);
            event->avrcpCtrlEvent.arg2 = atoi(user_cmd[FIVE_PARAM]);
            event->avrcpCtrlEvent.buf_ptr32 = new uint32_t[MAX_SUB_ARGUMENTS];
            num_Attr = Get32ArgsFromString(user_cmd[SIX_PARAM],event->avrcpCtrlEvent.buf_ptr32);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case GETITEMATTRIBUTES:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_GET_ITEM_ATTRIBUTES_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg1 = atoi(user_cmd[TWO_PARAM]);
            event->avrcpCtrlEvent.arg6 = strtoull(user_cmd[THREE_PARAM],NULL,10);
            event->avrcpCtrlEvent.arg3 = atoi(user_cmd[FOUR_PARAM]);
            event->avrcpCtrlEvent.arg2 = atoi(user_cmd[FIVE_PARAM]);
            event->avrcpCtrlEvent.buf_ptr32 = new uint32_t[MAX_SUB_ARGUMENTS];
            num_Attr = Get32ArgsFromString(user_cmd[SIX_PARAM],event->avrcpCtrlEvent.buf_ptr32);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case PLAYITEM:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_PLAY_ITEMS_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg1 = atoi(user_cmd[TWO_PARAM]);
            event->avrcpCtrlEvent.arg6 = strtoull(user_cmd[THREE_PARAM],NULL,10);
            event->avrcpCtrlEvent.arg3 = atoi(user_cmd[FOUR_PARAM]);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case ADDTONOWPLAYING:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_ADDTO_NOW_PLAYING_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg1 = atoi(user_cmd[TWO_PARAM]);
            event->avrcpCtrlEvent.arg6 = strtoull(user_cmd[THREE_PARAM],NULL,10);
            event->avrcpCtrlEvent.arg3 = atoi(user_cmd[FOUR_PARAM]);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case SEARCH:{
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_SEARCH_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            int length = atoi(user_cmd[TWO_PARAM]);
            event->avrcpCtrlEvent.arg3 = length;
            event->avrcpCtrlEvent.buf_ptr = new uint8_t(length+1);
            memset(event->avrcpCtrlEvent.buf_ptr, 0, length+1);
            strncpy((char*)event->avrcpCtrlEvent.buf_ptr,user_cmd[THREE_PARAM],length);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
            }
        case REG_NOTIFICATION:
            event = new BtEvent;
            memset(event, 0, sizeof(BtEvent));
            event->avrcpCtrlEvent.event_id = AVRCP_CTRL_REG_NOTIFICATION_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->avrcpCtrlEvent.bd_addr);
            event->avrcpCtrlEvent.arg1 = atoi(user_cmd[TWO_PARAM]);
            PostMessage (THREAD_ID_AVRCP, event);
            break;
        case CODEC_LIST:
            event = new BtEvent;
            event->a2dpCodecListEvent.event_id = A2DP_SINK_CODEC_LIST;
            memset( (void *) event->a2dpCodecListEvent.codec_list, '\0',
                sizeof(event->a2dpCodecListEvent.codec_list));
            strlcpy(event->a2dpCodecListEvent.codec_list, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_A2DP_SINK, event);
            break;
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;
    }
}

static void HandleA2dpSourceCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {
    ALOGV(LOGTAG, "HandleA2DPSourceCommand cmd_id = %d", cmd_id);
    BtEvent *event = NULL;
    switch (cmd_id) {
        case CONNECT:
        {
            bt_bdaddr_t address;
            string_to_bdaddr(user_cmd[ONE_PARAM], &address);
            if (!g_gap->IsDeviceBonded(address)){
                fprintf( stdout, " Please pair with the device before A2DPSource connection\n");
                break;
            }
            event = new BtEvent;
            event->a2dpSourceEvent.event_id = A2DP_SOURCE_API_CONNECT_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->a2dpSourceEvent.bd_addr);
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        }
        case DISCONNECT:
            event = new BtEvent;
            event->a2dpSourceEvent.event_id = A2DP_SOURCE_API_DISCONNECT_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->a2dpSourceEvent.bd_addr);
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case PLAY:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = A2DP_SOURCE_AUDIO_CMD_REQ;
            event->avrcpTargetEvent.key_id = CMD_ID_PLAY;
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case PAUSE:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = A2DP_SOURCE_AUDIO_CMD_REQ;
            event->avrcpTargetEvent.key_id = CMD_ID_PAUSE;
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case STOP:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = A2DP_SOURCE_AUDIO_CMD_REQ;
            event->avrcpTargetEvent.key_id = CMD_ID_STOP;
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case TRACK_CHANGE:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_TARGET_TRACK_CHANGED;
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case SET_ABS_VOL:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_TARGET_SET_ABS_VOL;
            event->avrcpTargetEvent.arg3 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case SEND_VOL_UP_DOWN:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_TARGET_SEND_VOL_UP_DOWN;
            event->avrcpTargetEvent.arg3 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case ADDR_PLAYER_CHANGE:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_TARGET_ADDR_PLAYER_CHANGED;
            event->avrcpTargetEvent.arg3 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case AVAIL_PLAYER_CHANGE:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_TARGET_AVAIL_PLAYER_CHANGED;
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case BIGGER_METADATA:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_TARGET_USE_BIGGER_METADATA;
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case CODEC_LIST:
            event = new BtEvent;
            event->a2dpCodecListEvent.event_id = A2DP_SOURCE_CODEC_LIST;
            memset( (void *) event->a2dpCodecListEvent.codec_list, '\0',
                sizeof(event->a2dpCodecListEvent.codec_list));
            strlcpy(event->a2dpCodecListEvent.codec_list, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
        case SET_EQUALIZER_VAL:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_SET_EQUALIZER_VAL;
            event->avrcpTargetEvent.arg3 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case SET_REPEAT_VAL:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_SET_REPEAT_VAL;
            event->avrcpTargetEvent.arg3 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case SET_SHUFFLE_VAL:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_SET_SHUFFLE_VAL;
            event->avrcpTargetEvent.arg3 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case SET_SCAN_VAL:
            event = new BtEvent;
            event->avrcpTargetEvent.event_id = AVRCP_SET_SCAN_VAL;
            event->avrcpTargetEvent.arg3 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_A2DP_SOURCE, event);
            break;
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;
    }
}

static void HandleHfpClientCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {
    ALOGD(LOGTAG, "HandleHfpClientCommand cmd_id = %d", cmd_id);
    BtEvent *event = NULL;
    switch (cmd_id) {
        case CONNECT:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_CONNECT_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->hfp_client_event.bd_addr);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case DISCONNECT:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_DISCONNECT_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->hfp_client_event.bd_addr);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case CREATE_SCO_CONN:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_CONNECT_AUDIO_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->hfp_client_event.bd_addr);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case DESTROY_SCO_CONN:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_DISCONNECT_AUDIO_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->hfp_client_event.bd_addr);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case ACCEPT_CALL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_ACCEPT_CALL_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case REJECT_CALL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_REJECT_CALL_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case END_CALL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_END_CALL_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case HOLD_CALL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_HOLD_CALL_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case RELEASE_HELD_CALL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_RELEASE_HELD_CALL_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case RELEASE_ACTIVE_ACCEPT_WAITING_OR_HELD_CALL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_RELEASE_ACTIVE_ACCEPT_WAITING_OR_HELD_CALL_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case SWAP_CALLS:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_SWAP_CALLS_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case ADD_HELD_CALL_TO_CONF:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_ADD_HELD_CALL_TO_CONF_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case RELEASE_SPECIFIED_ACTIVE_CALL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_RELEASE_SPECIFIED_ACTIVE_CALL_REQ;
            event->hfp_client_event.arg1 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case PRIVATE_CONSULTATION_MODE:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_PRIVATE_CONSULTATION_MODE_REQ;
            event->hfp_client_event.arg1 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case PUT_INCOMING_CALL_ON_HOLD:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_PUT_INCOMING_CALL_ON_HOLD_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case ACCEPT_HELD_INCOMING_CALL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_ACCEPT_HELD_INCOMING_CALL_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case REJECT_HELD_INCOMING_CALL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_REJECT_HELD_INCOMING_CALL_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case DIAL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_DIAL_REQ;
            strlcpy(event->hfp_client_event.str, user_cmd[ONE_PARAM], 20);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case REDIAL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_REDIAL_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case DIAL_MEMORY:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_DIAL_MEMORY_REQ;
            event->hfp_client_event.arg1 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case START_VR:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_START_VR_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case STOP_VR:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_STOP_VR_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case CALL_ACTION:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_CALL_ACTION_REQ;
            event->hfp_client_event.arg1 = atoi(user_cmd[ONE_PARAM]);
            event->hfp_client_event.arg2 = atoi(user_cmd[TWO_PARAM]);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case QUERY_CURRENT_CALLS:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_QUERY_CURRENT_CALLS_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case QUERY_OPERATOR_NAME:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_QUERY_OPERATOR_NAME_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case QUERY_SUBSCRIBER_INFO:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_QUERY_SUBSCRIBER_INFO_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case SCO_VOL_CTRL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_SCO_VOL_CTRL_REQ;
            event->hfp_client_event.arg1 = atoi(user_cmd[ONE_PARAM]);
            event->hfp_client_event.arg2 = atoi(user_cmd[TWO_PARAM]);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case MIC_VOL_CTRL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_MIC_VOL_CTRL_REQ;
            event->hfp_client_event.arg1 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case SPK_VOL_CTRL:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_SPK_VOL_CTRL_REQ;
            event->hfp_client_event.arg1 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case SEND_DTMF:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_SEND_DTMF_REQ;
            strlcpy(event->hfp_client_event.str, user_cmd[ONE_PARAM], 20);
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case DISABLE_NREC_ON_AG:
            event = new BtEvent;
            event->hfp_client_event.event_id = HFP_CLIENT_API_DISABLE_NREC_ON_AG_REQ;
            PostMessage (THREAD_ID_HFP_CLIENT, event);
            break;
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;
    }
}

static void HandleHfpAGCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {
    ALOGD(LOGTAG, "HandleHfpAGCommand cmd_id = %d", cmd_id);
    fprintf(stdout, "HandleHfpAGCommand cmd_id = %d\n" , cmd_id);
    BtEvent *event = NULL;
    switch (cmd_id) {
        case CONNECT:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_CONNECT_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->hfp_ag_event.bd_addr);
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case DISCONNECT:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_DISCONNECT_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->hfp_ag_event.bd_addr);
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case CREATE_SCO_CONN:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_CONNECT_AUDIO_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->hfp_ag_event.bd_addr);
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case DESTROY_SCO_CONN:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_DISCONNECT_AUDIO_REQ;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->hfp_ag_event.bd_addr);
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case ACCEPT_CALL:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_ACCEPT_CALL_REQ;
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case REJECT_CALL:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_REJECT_CALL_REQ;
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case END_CALL:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_END_CALL_REQ;
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case HOLD_CALL:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_HOLD_CALL_REQ;
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case SWAP_CALLS:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_SWAP_CALLS_REQ;
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case ADD_HELD_CALL_TO_CONF:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_ADD_HELD_CALL_TO_CONF_REQ;
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case DIAL:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_DIAL_REQ;
            strncpy(event->hfp_ag_event.str, user_cmd[ONE_PARAM], 20);
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case START_VR:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_START_VR_REQ;
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case STOP_VR:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_STOP_VR_REQ;
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case QUERY_CURRENT_CALLS:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_QUERY_CURRENT_CALLS_REQ;
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case MIC_VOL_CTRL:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_MIC_VOL_CTRL_REQ;
            event->hfp_ag_event.arg1 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case SPK_VOL_CTRL:
            event = new BtEvent;
            event->hfp_ag_event.event_id = HFP_AG_API_SPK_VOL_CTRL_REQ;
            event->hfp_ag_event.arg1 = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_HFP_AG, event);
            break;
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;
    }
}

static void HandleMainCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {

    switch (cmd_id) {
        case GAP_OPTION:
            menu_type = GAP_MENU;
            DisplayMenu(menu_type);
            break;
        case PAN_OPTION:
            menu_type = PAN_MENU;
            DisplayMenu(menu_type);
            break;
        case TEST_MODE:
            menu_type = TEST_MENU;
            DisplayMenu(menu_type);
            break;
        case RSP_OPTION:
            menu_type = RSP_MENU;
            DisplayMenu(menu_type);
            break;
        case A2DP_SINK:
            menu_type = A2DP_SINK_MENU;
            DisplayMenu(menu_type);
            break;
        case A2DP_SOURCE:
            menu_type = A2DP_SOURCE_MENU;
            DisplayMenu(menu_type);
            break;
        case HFP_CLIENT:
            menu_type = HFP_CLIENT_MENU;
            DisplayMenu(menu_type);
            break;
#ifdef USE_BT_OBEX
        case PBAP_CLIENT_OPTION:
            menu_type = PBAP_CLIENT_MENU;
            DisplayMenu(menu_type);
            break;
        case OPP_OPTION:
            menu_type = OPP_MENU;
            DisplayMenu(menu_type);
            break;
#endif
        case HFP_AG:
            menu_type = HFP_AG_MENU;
            DisplayMenu(menu_type);
            break;
        case MAIN_EXIT:
            ALOGV (LOGTAG " Self exit of Main thread");
            ExitHandler();
            break;
         default:
            ALOGV (LOGTAG " Command not handled");
            break;
    }
}


void HandleOnOffTest (void *context) {
    char *end;
    int index = 0;
    long  num = (long) context;
    for( index = 0; index < (long)num; index++) {

        BtEvent *event_on = new BtEvent;
        event_on->event_id = MAIN_API_ENABLE;
        fprintf( stdout, "Iteration: %d : Posting enable\n", index + 1);
        PostMessage (THREAD_ID_MAIN, event_on);
        sleep(5);
        BtEvent *event_off = new BtEvent;
        event_off->event_id = MAIN_API_DISABLE;
        fprintf( stdout, "Iteration: %d : Posting disable\n", index + 1);
        PostMessage (THREAD_ID_MAIN, event_off);
        sleep(5);
    }
    reactor_stop(thread_get_reactor(test_thread_id));
    test_thread_id = NULL;
}

static void HandleTestCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {

    long num = 0;
    char *end;
    int index = 0;
    switch (cmd_id) {
        case TEST_ON_OFF:
            if ((user_cmd[ONE_PARAM][0] != '\0')  && (!test_thread_id)) {
                errno = 0;
                num = strtol(user_cmd[ONE_PARAM], &end, 0);
                if (*end != '\0' || errno != 0 || num < INT_MIN || num > INT_MAX){
                    fprintf( stdout, " Enter numeric Value\n");
                    break;
                }

                test_thread_id = thread_new ("test_thread");
                if (test_thread_id)
                    thread_post(test_thread_id, HandleOnOffTest, (void *) num);

            } else if (test_thread_id) {
                fprintf( stdout, "Test is ongoing, please wait until it finishes\n");
            }
            break;

        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;
        default:
            ALOGV (LOGTAG " Command not handled");
            break;
    }
}

static void HandleRspCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {

    long num;
    char *end;
    int index = 0;
    switch (cmd_id) {
        case RSP_INIT:
            if ((g_bt_app->bt_state == BT_STATE_ON)) {
                fprintf( stdout, "ENABLE RSP\n");
                if (rsp) {
                   fprintf(stdout,"rsp already initialized \n");
                   return;
                } else {
                  if (g_gatt) {
                     rsp = new Rsp(g_gatt->GetGattInterface(),g_gatt);
                     if (rsp) {
                        rsp->EnableRSP();
                        fprintf(stdout, " EnableRSP done \n");
                     }
                     else {
                        fprintf(stdout, " RSP Alloc failed return failure \n");
                     }
                  } else {
                     fprintf(stdout," gatt interface us null \n");
                  }
                }
             }
             else {
                fprintf( stdout, "BT is in OFF State now \n");
             }
            break;

        case RSP_START:
            if ((g_bt_app->bt_state == BT_STATE_ON)) {
                if (rsp) {
                    fprintf( stdout, "(Re)start Advertisement \n");
                    rsp->StartAdvertisement();
                } else {
                    fprintf(stdout , "Do Init first\n");
                }
            } else {
                fprintf( stdout, "BT is in OFF State now \n");
            }
            break;

        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;

        default:
            fprintf(stdout, " Command not handled\n");
            break;
    }
}

static void SendEnableCmdToGap() {

    if ((g_bt_app->status.enable_cmd != COMMAND_INPROGRESS) &&
        (g_bt_app->status.disable_cmd != COMMAND_INPROGRESS) &&
        (g_bt_app->bt_state == BT_STATE_OFF)) {

        g_bt_app->status.enable_cmd = COMMAND_INPROGRESS;
        // Killing previous iteration filter if they still exists
        system("killall -KILL wcnssfilter");
        system("killall -KILL btsnoop");
        system("killall -KILL qcbtdaemon");
        usleep(200);

        BtEvent *event = new BtEvent;
        event->event_id = GAP_API_ENABLE;
        ALOGV (LOGTAG " Posting BT enable to GAP thread");
        PostMessage (THREAD_ID_GAP, event);
    } else if ( g_bt_app->status.enable_cmd == COMMAND_INPROGRESS ) {
        fprintf( stdout, "BT enable is already in process\n");
    } else if ( g_bt_app->status.disable_cmd == COMMAND_INPROGRESS ) {
        fprintf( stdout, "Previous BT disable is still in progress\n");
    } else {
        fprintf( stdout, "Currently BT is already ON\n");
    }
}

static void SendDisableCmdToGap() {

    if ((g_bt_app->status.disable_cmd != COMMAND_INPROGRESS) &&
        (g_bt_app->status.enable_cmd != COMMAND_INPROGRESS) &&
        (g_bt_app->bt_state == BT_STATE_ON)) {

        g_bt_app->status.disable_cmd = COMMAND_INPROGRESS;

        BtEvent *event = new BtEvent;
        event->event_id = GAP_API_DISABLE;
        ALOGV (LOGTAG " Posting disable to GAP thread");
        PostMessage (THREAD_ID_GAP, event);
    } else if (g_bt_app->status.disable_cmd == COMMAND_INPROGRESS) {
        fprintf( stdout, " disable command is already in process\n");
    } else if (g_bt_app->status.enable_cmd == COMMAND_INPROGRESS) {
        fprintf( stdout, " Previous enable command is still in process\n");
    } else {
        fprintf( stdout, "Currently BT is already OFF\n");
    }
}

static void HandleGapCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {
    BtEvent *event = NULL;

    switch (cmd_id) {
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;

        case BT_ENABLE:
            SendEnableCmdToGap();
            break;

        case BT_DISABLE:
            SendDisableCmdToGap();
            break;

        case START_ENQUIRY:

            if ((g_bt_app->status.enquiry_cmd != COMMAND_INPROGRESS) &&
                                (g_bt_app->bt_state == BT_STATE_ON)) {

                g_bt_app->inquiry_list.clear();
                g_bt_app->status.enquiry_cmd = COMMAND_INPROGRESS;
                event = new BtEvent;
                event->event_id = GAP_API_START_INQUIRY;
                ALOGV (LOGTAG " Posting inquiry to GAP thread");
                PostMessage (THREAD_ID_GAP, event);

            } else if (g_bt_app->status.enquiry_cmd == COMMAND_INPROGRESS) {
                fprintf( stdout, " The inquiry is already in process\n");

            } else {
                fprintf( stdout, "currently BT is OFF\n");
            }
            break;

        case CANCEL_ENQUIRY:

            if ((g_bt_app->status.stop_enquiry_cmd != COMMAND_INPROGRESS) &&
                (g_bt_app->bt_discovery_state == BT_DISCOVERY_STARTED) &&
                        (g_bt_app->bt_state == BT_STATE_ON)) {
                g_bt_app->status.stop_enquiry_cmd = COMMAND_INPROGRESS;
                event = new BtEvent;
                event->event_id = GAP_API_STOP_INQUIRY;
                ALOGV (LOGTAG " Posting stop inquiry to GAP thread");
                PostMessage (THREAD_ID_GAP, event);

            } else if (g_bt_app->status.stop_enquiry_cmd == COMMAND_INPROGRESS) {
                fprintf( stdout, " The stop inquiry is already in process\n");

            } else if (g_bt_app->bt_state == BT_STATE_OFF) {
                fprintf( stdout, "currently BT is OFF\n");

            } else if (g_bt_app->bt_discovery_state != BT_DISCOVERY_STARTED) {
                fprintf( stdout,"Inquiry is not started, ignoring the stop inquiry\n");
            }
            break;

        case START_PAIR:
            if ((g_bt_app->status.pairing_cmd != COMMAND_INPROGRESS) &&
                (g_bt_app->bt_state == BT_STATE_ON)) {
                if (string_is_bdaddr(user_cmd[ONE_PARAM])) {
                    g_bt_app->status.pairing_cmd = COMMAND_INPROGRESS;
                    event = new BtEvent;
                    event->event_id = GAP_API_CREATE_BOND;
                    string_to_bdaddr(user_cmd[ONE_PARAM], &event->bond_device.bd_addr);
                    PostMessage (THREAD_ID_GAP, event);
                } else {
                 fprintf( stdout, " BD address is NULL/Invalid \n");
                }
            } else if (g_bt_app->status.pairing_cmd == COMMAND_INPROGRESS) {
                fprintf( stdout, " Pairing is already in process\n");
            } else {
                fprintf( stdout, " Currently BT is OFF\n");
            }
            break;

        case UNPAIR:
            if ( g_bt_app->bt_state == BT_STATE_ON ) {
                if (string_is_bdaddr(user_cmd[ONE_PARAM])) {
                    bt_bdaddr_t bd_addr;
                    string_to_bdaddr(user_cmd[ONE_PARAM], &bd_addr);
                    g_bt_app->HandleUnPair(bd_addr);
                } else {
                    fprintf( stdout, " BD address is NULL/Invalid \n");
                }
            } else {
                fprintf( stdout, " Currently BT is OFF\n");
            }
            break;

        case INQUIRY_LIST:
            if (!g_bt_app->inquiry_list.empty()) {
                g_bt_app->PrintInquiryList();
            } else {
                fprintf( stdout, " Empty Inquiry list\n");
            }
            break;

        case BONDED_LIST:
            if (!g_bt_app->bonded_devices.empty()) {
                g_bt_app->PrintBondedDeviceList();
            } else {
                fprintf( stdout, " Empty bonded list\n");
            }
            break;

        case GET_BT_STATE:
           if ( g_bt_app->GetState() == BT_STATE_ON )
               fprintf( stdout, "ON\n" );
            else if ( g_bt_app->GetState() == BT_STATE_OFF)
                fprintf( stdout, "OFF\n");
            break;

        case GET_BT_NAME:
            if ( g_bt_app->GetState() == BT_STATE_ON ) {
                fprintf(stdout, "BT Name : %s\n", g_gap->GetBtName());
            } else {
                fprintf( stdout, "No Name due to BT is OFF\n");
            }
            break;

        case GET_BT_ADDR:
            if ( g_bt_app->GetState() == BT_STATE_ON ) {
                bdstr_t bd_str;
                bt_bdaddr_t *bd_addr = g_gap->GetBtAddress();
                bdaddr_to_string(bd_addr, &bd_str[0], sizeof(bd_str));
                fprintf(stdout, " BT Address : %s\n", bd_str);
            } else {
                fprintf( stdout, "No Addr due to BT is OFF\n");
            }
            break;

        case SET_BT_NAME:
            if ( g_bt_app->GetState() == BT_STATE_ON ) {
                if (strlen(user_cmd[ONE_PARAM]) < BTM_MAX_LOC_BD_NAME_LEN &&
                    (user_cmd[ONE_PARAM] != NULL) ) {
                    bt_bdname_t bd_name;
                    event = new BtEvent;
                    event->event_id = GAP_API_SET_BDNAME;
                    event->set_device_name_event.prop.type = BT_PROPERTY_BDNAME;
                    strlcpy((char*)&bd_name.name[0],user_cmd[ONE_PARAM],COMMAND_SIZE);
                    event->set_device_name_event.prop.val = &bd_name;
                    event->set_device_name_event.prop.len = strlen((char*)bd_name.name);
                    PostMessage (THREAD_ID_GAP, event);
                } else {
                 fprintf( stdout, " BD Name is NULL/more than required legnth\n");
                }
            } else {
                fprintf( stdout, " Currently BT is OFF\n");
            }
            break;

        default:
            ALOGV (LOGTAG " Command not handled");
            break;
    }
}

static void HandlePanCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {

    long num;
    char *end;
    int index = 0;

    switch (cmd_id) {
        case CONNECT:
            if ((g_bt_app->bt_state == BT_STATE_ON)) {
                if (string_is_bdaddr(user_cmd[ONE_PARAM])) {
                    BtEvent *event = new BtEvent;
                    event->event_id = PAN_EVENT_DEVICE_CONNECT_REQ;
                    string_to_bdaddr(user_cmd[ONE_PARAM],
                            &event->pan_device_connect_event.bd_addr);
                    PostMessage (THREAD_ID_PAN, event);
                } else {
                    fprintf(stdout, " BD address is NULL/Invalid \n");
                }
            } else {
                fprintf(stdout, " Currently BT is in OFF state\n");
            }
            break;

        case DISCONNECT:
            if ((g_bt_app->bt_state == BT_STATE_ON)) {
                if (string_is_bdaddr(user_cmd[ONE_PARAM])) {
                    BtEvent *event = new BtEvent;
                    event->event_id = PAN_EVENT_DEVICE_DISCONNECT_REQ;
                    string_to_bdaddr(user_cmd[ONE_PARAM],
                            &event->pan_device_disconnect_event.bd_addr);
                    PostMessage (THREAD_ID_PAN, event);
                } else {
                    fprintf(stdout, " BD address is NULL/Invalid\n");
                }
            } else {
                fprintf(stdout, " Currently BT is in OFF state\n");
            }
            break;

        case CONNECTED_LIST:
            if ((g_bt_app->bt_state == BT_STATE_ON)) {
                BtEvent *event = new BtEvent;
                event->event_id = PAN_EVENT_DEVICE_CONNECTED_LIST_REQ;
                PostMessage (THREAD_ID_PAN, event);
            } else {
                fprintf(stdout," Currently BT is in OFF state\n");
            }
            break;

        case SET_TETHERING:

            if ((g_bt_app->bt_state == BT_STATE_ON)) {
                bool is_tethering_enable;

                if (!strcasecmp (user_cmd[ONE_PARAM], "true")) {
                    is_tethering_enable = true;
                } else if (!strcasecmp (user_cmd[ONE_PARAM], "false")) {
                    is_tethering_enable = false;
                } else {
                    fprintf(stdout, " Wrong option selected\n");
                    return;
                }

                BtEvent *event = new BtEvent;
                event->event_id = PAN_EVENT_SET_TETHERING_REQ;
                event->pan_set_tethering_event.is_tethering_on = is_tethering_enable;
                PostMessage (THREAD_ID_PAN, event);
            } else {
                fprintf(stdout, " Currently BT is in OFF state\n");
            }
            break;

        case GET_PAN_MODE:
            if ((g_bt_app->bt_state == BT_STATE_ON)) {
                BtEvent *event = new BtEvent;
                event->event_id = PAN_EVENT_GET_MODE_REQ;
                PostMessage (THREAD_ID_PAN, event);
            } else {
                fprintf(stdout, " Currently BT is in OFF state\n");
            }
            break;

        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;

        default:
            ALOGV (LOGTAG " Command not handled: %d", cmd_id);
            break;
    }
}

#ifdef USE_BT_OBEX
static void HandlePbapClientCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {

    long num;
    char *end;
    int index = 0;
    BtEvent *event = NULL;

    if (g_bt_app && g_bt_app->bt_state != BT_STATE_ON) {
        ALOGE(LOGTAG "BT not switched on, can't handle PBAP Client commands");
        return;
    }

    switch (cmd_id) {

        case PBAP_REGISTER:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_REGISTER;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case CONNECT:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_CONNECT;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->pbap_client_event.bd_addr);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case DISCONNECT:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_DISCONNECT;
            string_to_bdaddr(user_cmd[ONE_PARAM], &event->pbap_client_event.bd_addr);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_ABORT:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_ABORT;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_PHONEBOOK_SIZE:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_PHONEBOOK_SIZE;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_PHONEBOOK:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_PHONEBOOK;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_VCARD:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_VCARD;
            memset( (void *) event->pbap_client_event.value, '\0',
                sizeof(event->pbap_client_event.value));
            strlcpy(event->pbap_client_event.value, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_VCARD_LISTING:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_VCARD_LISTING;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_PATH:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_PATH;
            memset( (void *) event->pbap_client_event.value, '\0',
                sizeof(event->pbap_client_event.value));
            strlcpy(event->pbap_client_event.value, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_FILTER:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_FILTER;
            memset( (void *) event->pbap_client_event.value, '\0',
                sizeof(event->pbap_client_event.value));
            strlcpy(event->pbap_client_event.value, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_FILTER:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_FILTER;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_ORDER:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_ORDER;
            memset( (void *) event->pbap_client_event.value, '\0',
                sizeof(event->pbap_client_event.value));
            strlcpy(event->pbap_client_event.value, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_ORDER:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_ORDER;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_SEARCH_ATTRIBUTE:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_SEARCH_ATTRIBUTE;
            memset( (void *) event->pbap_client_event.value, '\0',
                sizeof(event->pbap_client_event.value));
            strlcpy(event->pbap_client_event.value, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_SEARCH_ATTRIBUTE:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_SEARCH_ATTRIBUTE;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_SEARCH_VALUE:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_SEARCH_VALUE;
            memset( (void *) event->pbap_client_event.value, '\0',
                sizeof(event->pbap_client_event.value));
            strlcpy(event->pbap_client_event.value, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_PHONE_BOOK:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_PHONE_BOOK;
            memset( (void *) event->pbap_client_event.value, '\0',
                sizeof(event->pbap_client_event.value));
            strlcpy(event->pbap_client_event.value, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_PHONE_BOOK:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_PHONE_BOOK;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_REPOSITORY:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_REPOSITORY;
            memset( (void *) event->pbap_client_event.value, '\0',
                sizeof(event->pbap_client_event.value));
            strlcpy(event->pbap_client_event.value, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_REPOSITORY:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_REPOSITORY;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_VCARD_FORMAT:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_VCARD_FORMAT;
            memset( (void *) event->pbap_client_event.value, '\0',
                sizeof(event->pbap_client_event.value));
            strlcpy(event->pbap_client_event.value, user_cmd[ONE_PARAM],
                COMMAND_SIZE);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_VCARD_FORMAT:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_VCARD_FORMAT;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_LIST_COUNT:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_LIST_COUNT;
            event->pbap_client_event.max_list_count = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_LIST_COUNT:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_LIST_COUNT;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_SET_START_OFFSET:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_SET_START_OFFSET;
            event->pbap_client_event.list_start_offset = atoi(user_cmd[ONE_PARAM]);
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case PBAP_GET_START_OFFSET:
            event = new BtEvent;
            event->pbap_client_event.event_id = PBAP_CLIENT_GET_START_OFFSET;
            PostMessage (THREAD_ID_PBAP_CLIENT, event);
            break;
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;

        default:
        ALOGV (LOGTAG " Command not handled: %d", cmd_id);
        break;
    }
}

static void HandleOppCommand(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE]) {

    long num;
    char *end;
    int index = 0;
    BtEvent *event = NULL;

    if (g_bt_app && g_bt_app->bt_state != BT_STATE_ON) {
        ALOGE(LOGTAG "BT not switched on, can't handle OPP commands");
        return;
    }

    switch (cmd_id) {

        case OPP_REGISTER:
            event = new BtEvent;
            event->opp_event.event_id = OPP_SRV_REGISTER;
            PostMessage (THREAD_ID_OPP, event);
            break;
        case OPP_SEND:
            event = new BtEvent;
            event->opp_event.event_id = OPP_SEND_DATA;
            if(string_to_bdaddr(user_cmd[ONE_PARAM],
                &event->opp_event.bd_addr)) {
                memset( (void *) event->opp_event.value, 0,
                    sizeof(event->opp_event.value));
                strlcpy(event->opp_event.value, user_cmd[TWO_PARAM],
                    COMMAND_SIZE);
                PostMessage (THREAD_ID_OPP, event);
            } else {
                ALOGV (LOGTAG " Please enter valid BD Address %s",
                    user_cmd[ONE_PARAM]);
            }
            break;
        case OPP_ABORT:
            event = new BtEvent;
            event->opp_event.event_id = OPP_ABORT_TRANSFER;
            PostMessage (THREAD_ID_OPP, event);
            break;
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            DisplayMenu(menu_type);
            break;

        default:
        ALOGV (LOGTAG " Command not handled: %d", cmd_id);
        break;
    }
}
#endif

void BtSocketDataHandler (void *context) {
    char ipc_msg[BT_IPC_MSG_LEN]  = {0};
    int len;
    if(g_bt_app->client_socket_ != -1) {
        len = recv(g_bt_app->client_socket_, ipc_msg, BT_IPC_MSG_LEN, 0);

        if (len <= 0) {
            ALOGE("Not able to receive msg to remote dev: %s", strerror(errno));
            reactor_unregister (g_bt_app->accept_reactor_);
            g_bt_app->accept_reactor_ = NULL;
            close(g_bt_app->client_socket_);
            g_bt_app->client_socket_ = -1;
        } else if(len == BT_IPC_MSG_LEN) {
            BtEvent *event = new BtEvent;
            event->event_id = SKT_API_IPC_MSG_READ;
            event->bt_ipc_msg_event.ipc_msg.type = ipc_msg[0];
            event->bt_ipc_msg_event.ipc_msg.status = ipc_msg[1];

            switch (event->bt_ipc_msg_event.ipc_msg.type) {
                /*fall through for PAN IPC message*/
                case BT_IPC_ENABLE_TETHERING:
                case BT_IPC_DISABLE_TETHERING:
                case BT_IPC_ENABLE_REVERSE_TETHERING:
                case BT_IPC_DISABLE_REVERSE_TETHERING:
                    ALOGV (LOGTAG "  Posting IPC_MSG to PAN thread");
                    PostMessage (THREAD_ID_PAN, event);
                    break;
                case BT_IPC_REMOTE_START_WLAN:
                    ALOGV (LOGTAG "  Posting IPC_MSG to GATT thread");
                    PostMessage (THREAD_ID_GATT, event);
                    break;
                default:
                    delete event;
                break;
            }
        }
    }
}

void BtSocketListenHandler (void *context) {
    struct sockaddr_un cliaddr;
    int length;

    if(g_bt_app->client_socket_ == -1) {
        g_bt_app->client_socket_ = accept(g_bt_app->listen_socket_local_,
            (struct sockaddr*) &cliaddr, ( socklen_t *) &length);
        if (g_bt_app->client_socket_ == -1) {
            ALOGE (LOGTAG "%s error accepting LOCAL socket: %s",
                        __func__, strerror(errno));
        } else {
            g_bt_app->accept_reactor_ = reactor_register
                (thread_get_reactor (threadInfo[THREAD_ID_MAIN].thread_id),
                g_bt_app->client_socket_, NULL, BtSocketDataHandler, NULL);
        }
    } else {
        ALOGI (LOGTAG " Accepting and closing the next connection .\n");
        int accept_socket = accept(g_bt_app->listen_socket_local_,
            (struct sockaddr*) &cliaddr, ( socklen_t *) &length);
        if(accept_socket)
            close(accept_socket);
    }
}

static void BtCmdHandler (void *context) {

    int cmd_id = -1;
    char user_cmd[MAX_ARGUMENTS][COMMAND_ARG_SIZE];
    int index = 0;
    memset( (void *) user_cmd, '\0', sizeof(user_cmd));

    if (HandleUserInput (&cmd_id, user_cmd, menu_type)) {
        ALOGI (LOGTAG "BtCmdHandler menu_type:%d cmd_id:%d", menu_type, cmd_id);
        switch(menu_type) {
            case GAP_MENU:
                HandleGapCommand(cmd_id,user_cmd);
                break;
            case PAN_MENU:
                HandlePanCommand(cmd_id, user_cmd);
                break;
            case TEST_MENU:
                HandleTestCommand(cmd_id, user_cmd);
                break;
            case RSP_MENU:
                HandleRspCommand(cmd_id, user_cmd);
                break;
            case MAIN_MENU:
                HandleMainCommand(cmd_id,user_cmd );
                break;
            case A2DP_SINK_MENU:
                HandleA2dpSinkCommand(cmd_id,user_cmd );
                break;
            case A2DP_SOURCE_MENU:
                HandleA2dpSourceCommand(cmd_id, user_cmd );
                break;
            case HFP_CLIENT_MENU:
                HandleHfpClientCommand(cmd_id,user_cmd );
                break;
#ifdef USE_BT_OBEX
            case PBAP_CLIENT_MENU:
                HandlePbapClientCommand(cmd_id,user_cmd );
                break;
            case OPP_MENU:
                HandleOppCommand(cmd_id,user_cmd );
                break;
#endif
            case HFP_AG_MENU:
                HandleHfpAGCommand(cmd_id, user_cmd );
                break;
        }
    } else if (g_bt_app->ssp_notification && user_cmd[0][0] &&
                        (!strcasecmp (user_cmd[ZERO_PARAM], "yes") ||
                        !strcasecmp (user_cmd[ZERO_PARAM], "no"))
                        && g_bt_app->HandleSspInput(user_cmd)) {
        // validate the user input for SSP
        g_bt_app->ssp_notification = false;
    } else if (g_bt_app->pin_notification && user_cmd[0][0] &&
                        (strcasecmp (user_cmd[ZERO_PARAM], "yes") &&
                        strcasecmp (user_cmd[ZERO_PARAM], "no") &&
                        strcasecmp (user_cmd[ZERO_PARAM], "accept") &&
                        strcasecmp (user_cmd[ZERO_PARAM], "reject"))
                        && g_bt_app->HandlePinInput(user_cmd)) {
        // validate the user input for PIN
        g_bt_app->pin_notification = false;
    }
#ifdef USE_BT_OBEX
    else if (g_bt_app->incoming_file_notification && user_cmd[0][0] &&
                        (!strcasecmp (user_cmd[ZERO_PARAM], "accept") ||
                        !strcasecmp (user_cmd[ZERO_PARAM], "reject"))
                        && g_bt_app->HandleIncomingFile(user_cmd)) {
        // validate the user input for OPP Incoming File
        g_bt_app->incoming_file_notification = false;
    }
#endif
    else {
        fprintf( stdout, " Wrong option selected\n");
        DisplayMenu(menu_type);
        // TODO print the given input string
        return;
    }
}

void BtMainMsgHandler (void *context) {

    BtEvent *event = NULL;
    if (!context) {
        ALOGI (LOGTAG " Msg is null, return.\n");
        return;
    }
    event = (BtEvent *) context;

    switch (event->event_id) {
        case SKT_API_IPC_MSG_WRITE:
            ALOGV (LOGTAG "client_socket: %d", g_bt_app->client_socket_);
            if(g_bt_app->client_socket_ != -1) {
                int len;
                if((len = send(g_bt_app->client_socket_, &(event->bt_ipc_msg_event.ipc_msg),
                    BT_IPC_MSG_LEN, 0)) < 0) {
                    reactor_unregister (g_bt_app->accept_reactor_);
                    close(g_bt_app->client_socket_);
                    g_bt_app->client_socket_ = -1;
                    ALOGE (LOGTAG "Local socket send fail %s", strerror(errno));
                }
                ALOGV (LOGTAG "sent %d bytes", len);
            }
            delete event;
            break;

        case MAIN_API_INIT:
            if (!g_bt_app)
                g_bt_app = new BluetoothApp();
        // fallback to default handler
        default:
            if (g_bt_app)
                g_bt_app->ProcessEvent ((BtEvent *) context);
            delete event;
            break;
    }
}

#ifdef __cplusplus
}
#endif

bool BluetoothApp :: HandlePinInput(char user_cmd[][COMMAND_ARG_SIZE]) {
    BtEvent *bt_event = new BtEvent;

    if (pin_reply.secure  == true ) {
        if ( strlen(user_cmd[ZERO_PARAM]) != 16){
            fprintf(stdout, " Minimum 16 digit pin required\n");
            return false;
        }
    } else if(strlen(user_cmd[ZERO_PARAM]) > 16 ) {
        return false;
    }

    memset(&pin_reply.pincode, 0, sizeof(bt_pin_code_t));
    memcpy(&pin_reply.pincode.pin, user_cmd[ZERO_PARAM],
                        strlen(user_cmd[ZERO_PARAM]));
    memcpy(&bt_event->pin_reply_event.bd_addr, &pin_reply.bd_addr,
                                            sizeof(bt_bdaddr_t));
    bt_event->pin_reply_event.pin_len = strlen(user_cmd[ZERO_PARAM]);
    memcpy(&bt_event->pin_reply_event.bd_name, &pin_reply.bd_name,
                                        sizeof(bt_bdname_t));
    memcpy(&bt_event->pin_reply_event.pincode.pin, &pin_reply.pincode.pin,
                                        sizeof(bt_pin_code_t));
    bt_event->event_id = GAP_API_PIN_REPLY;
    PostMessage (THREAD_ID_GAP, bt_event);
    return true;
}


bool BluetoothApp :: HandleSspInput(char user_cmd[][COMMAND_ARG_SIZE]) {


    BtEvent *bt_event = new BtEvent;
    if (!strcasecmp (user_cmd[ZERO_PARAM], "yes")) {
        ssp_data.accept = true;
    }
    else if (!strcasecmp (user_cmd[ZERO_PARAM], "no")) {
        ssp_data.accept = false;
    } else {
        fprintf( stdout, " Wrong option selected\n");
        return false;
    }

    memcpy(&bt_event->ssp_reply_event.bd_addr, &ssp_data.bd_addr,
                                            sizeof(bt_bdaddr_t));
    memcpy(&bt_event->ssp_reply_event.bd_name, &ssp_data.bd_name,
                                        sizeof(bt_bdname_t));
    bt_event->ssp_reply_event.cod = ssp_data.cod;
    bt_event->ssp_reply_event.pairing_variant = ssp_data.pairing_variant;
    bt_event->ssp_reply_event.pass_key = ssp_data.pass_key;
    bt_event->ssp_reply_event.accept = ssp_data.accept;
    bt_event->event_id = GAP_API_SSP_REPLY;
    PostMessage (THREAD_ID_GAP, bt_event);
    return true;
}

#ifdef USE_BT_OBEX
bool BluetoothApp :: HandleIncomingFile(char user_cmd[][COMMAND_ARG_SIZE]) {
    BtEvent *bt_event = new BtEvent;
    if (!strcasecmp (user_cmd[ZERO_PARAM], "accept")) {
        bt_event->opp_event.accept = true;
    } else if (!strcasecmp (user_cmd[ZERO_PARAM], "reject")) {
        bt_event->opp_event.accept = false;
    } else {
        fprintf( stdout, "Wrong option entered\n");
        return false;
    }
    /* Cancel the timer */
    alarm_cancel(opp_incoming_file_accept_timer);
    bt_event->event_id = OPP_INCOMING_FILE_RESPONSE;
    PostMessage (THREAD_ID_OPP, bt_event);
    return true;
}

void user_acceptance_timer_expired(void *context) {
    BtEvent *bt_event = new BtEvent;
    ALOGD(LOGTAG " user_acceptance_timer_expired, rejecting incoming file");
    fprintf(stdout,"No user input for %d seconds, rejecting the file\n",
        USER_ACCEPTANCE_TIMEOUT/1000);

    bt_event->opp_event.accept = false;
    g_bt_app->incoming_file_notification = false;
    bt_event->event_id = OPP_INCOMING_FILE_RESPONSE;
    PostMessage (THREAD_ID_OPP, bt_event);
}
#endif

void BluetoothApp :: ProcessEvent (BtEvent * event) {

    ALOGD (LOGTAG " Processing event %d", event->event_id);
    const uint8_t *ptr = NULL;

    switch (event->event_id) {
        case MAIN_API_INIT:
            InitHandler();
            break;

        case MAIN_API_DEINIT:
            DeInitHandler();
            break;

        case MAIN_API_ENABLE:
            SendEnableCmdToGap();
            break;

        case MAIN_API_DISABLE:
            SendDisableCmdToGap();
            break;

        case MAIN_EVENT_ENABLED:
            bt_state = event->state_event.status;
            if (event->state_event.status == BT_STATE_OFF) {
                fprintf(stdout," Error in Enabling BT\n");
            } else {
               fprintf(stdout," BT State is ON\n");
            }
            status.enable_cmd = COMMAND_COMPLETE;
            break;

        case MAIN_EVENT_DISABLED:
            bt_state = event->state_event.status;
            if (event->state_event.status == BT_STATE_ON) {
                fprintf(stdout, " Error in disabling BT\n");
            } else {
                // clear the inquiry related cmds
                status.enquiry_cmd = COMMAND_COMPLETE;
                status.stop_enquiry_cmd = COMMAND_COMPLETE;
                bt_discovery_state = BT_DISCOVERY_STOPPED;
                // clearing bond_devices list and inquiry_list
                bonded_devices.clear();
                inquiry_list.clear();
                system("killall -KILL wcnssfilter");
                usleep(200);
                fprintf(stdout, " BT State is OFF\n");
            }
            status.disable_cmd = COMMAND_COMPLETE;
            break;

        case MAIN_EVENT_ACL_CONNECTED:
            ALOGD (LOGTAG " MAIN_EVENT_ACL_CONNECTED\n");
            break;

        case MAIN_EVENT_ACL_DISCONNECTED:
            ALOGD (LOGTAG " MAIN_EVENT_ACL_DISCONNECTED\n");
            break;

        case MAIN_EVENT_INQUIRY_STATUS:
            if (event->discovery_state_event.state == BT_DISCOVERY_STARTED) {
                fprintf(stdout, " Inquiry Started\n");
            } else if (event->discovery_state_event.state == BT_DISCOVERY_STOPPED) {
                if ( status.enquiry_cmd == COMMAND_INPROGRESS) {
                    if ((bt_discovery_state == BT_DISCOVERY_STARTED) &&
                       (status.stop_enquiry_cmd != COMMAND_INPROGRESS))
                        fprintf(stdout, " Inquiry Stopped automatically\n");
                    else if (bt_discovery_state == BT_DISCOVERY_STOPPED)
                        fprintf(stdout, " Unable to start Inquiry\n");
                    status.enquiry_cmd = COMMAND_COMPLETE;
                }
                if (status.stop_enquiry_cmd == COMMAND_INPROGRESS) {
                    status.stop_enquiry_cmd = COMMAND_COMPLETE;
                    fprintf(stdout," Inquiry Stopped due to user input\n");
                }
            }
            bt_discovery_state = event->discovery_state_event.state;
            break;

        case MAIN_EVENT_DEVICE_FOUND:
           {
           bdstr_t bd_str;
            std::map<std::string, std::string>::iterator it;
            bdaddr_to_string(&event->device_found_event.remoteDevice.address, &bd_str[0], sizeof(bd_str));
            std::string deviceAddress(bd_str);

            it = bonded_devices.find(deviceAddress);
            if (it != bonded_devices.end())
            {
                break;
            }
            fprintf(stdout, "Device Found details: \n");
            AddFoundedDevice(event->device_found_event.remoteDevice.name,
                                    event->device_found_event.remoteDevice.address);
            ptr = (event->device_found_event.remoteDevice.address.address);
            fprintf(stdout,"Found device Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                                ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
            fprintf(stdout, "Found device Name: %s\n", event->device_found_event.
                                                    remoteDevice.name);
            fprintf(stdout, "Device class is: %d\n", event->device_found_event.
                                        remoteDevice.bluetooth_class);
            break;
        }
        case MAIN_EVENT_BOND_STATE: {
            std::string bd_name((const char*)event->bond_state_event.bd_name.name);
            HandleBondState(event->bond_state_event.state,
                                    event->bond_state_event.bd_addr, bd_name);
            }
            break;

        case MAIN_EVENT_SSP_REQUEST:
            memcpy(&ssp_data.bd_addr, &event->ssp_request_event.bd_addr,
                                            sizeof(bt_bdaddr_t));
            memcpy(&ssp_data.bd_name, &event->ssp_request_event.bd_name,
                                                    sizeof(bt_bdname_t));
            ssp_data.cod = event->ssp_request_event.cod;
            ssp_data.pairing_variant = event->ssp_request_event.pairing_variant;
            ssp_data.pass_key = event->ssp_request_event.pass_key;
            // instruct the cmd handler to treat the next inputs for SSP
            fprintf(stdout, "\n*************************************************");
            fprintf(stdout, "\n BT pairing request::Device %s::Pairing Code:: %d",
                                    ssp_data.bd_name.name, ssp_data.pass_key);
            fprintf(stdout, "\n*************************************************\n");
            fprintf(stdout, " ** Please enter yes / no **\n");
            ssp_notification = true;
            break;

        case MAIN_EVENT_PIN_REQUEST:

            memcpy(&pin_reply.bd_addr, &event->pin_request_event.bd_addr,
                                            sizeof(bt_bdaddr_t));
            memcpy(&pin_reply.bd_name, &event->pin_request_event.bd_name,
                                            sizeof(bt_bdname_t));
            fprintf(stdout, "\n*************************************************");
            fprintf(stdout, "\n BT Legacy pairing request::Device %s::",
                                    pin_reply.bd_name.name);
            fprintf(stdout, "\n*************************************************\n");
            fprintf(stdout, " ** Please enter valid PIN key **\n");
            pin_reply.secure = event->pin_request_event.secure;
            // instruct the cmd handler to treat the next inputs for PIN
            pin_notification = true;
            break;

#ifdef USE_BT_OBEX
        case MAIN_EVENT_INCOMING_FILE_REQUEST:

            fprintf(stdout, "\n*************************************************");
            fprintf(stdout, "\n Incoming File Request");
            fprintf(stdout, "\n*************************************************\n");
            fprintf(stdout, " ** Please enter \"accept\" / \"reject\" **\n");
            // instruct the cmd handler to treat the next inputs for OPP
            incoming_file_notification = true;
            if (opp_incoming_file_accept_timer) {
                 // start the user acceptance/rejection rimer
                alarm_set(opp_incoming_file_accept_timer, USER_ACCEPTANCE_TIMEOUT,
                                    user_acceptance_timer_expired, NULL);
            } else {
                fprintf(stdout, "\n Already pending user acceptance request\n");
            }
            break;
#endif

        default:
            ALOGD (LOGTAG " Default Case");
            break;
    }
}

void BluetoothApp:: HandleBondState(bt_bond_state_t new_state, const bt_bdaddr_t
                                        bd_addr, std::string bd_name ) {
    std::map<std::string, std::string>::iterator it;
    std::map<std::string, std::string>::iterator it_inquiry;
    bdstr_t bd_str;
    bdaddr_to_string(&bd_addr, &bd_str[0], sizeof(bd_str));
    std::string deviceAddress(bd_str);
    it = bonded_devices.find(deviceAddress);
    it_inquiry= inquiry_list.find(deviceAddress);
    if(new_state == BT_BOND_STATE_BONDED) {
        if (it == bonded_devices.end()) {
            bonded_devices[deviceAddress] = bd_name;
        }
       if(it_inquiry!=inquiry_list.end())
        {
            inquiry_list.erase(it_inquiry);
        }

        fprintf(stdout, "\n*************************************************");
        fprintf(stdout, "\n Pairing state for %s is BONDED", bd_name.c_str());
        fprintf(stdout, "\n*************************************************\n");
        g_bt_app->status.pairing_cmd = COMMAND_COMPLETE;

    } else if (new_state == BT_BOND_STATE_NONE) {
        if (it != bonded_devices.end()) {
            bonded_devices.erase(it);
        }
        fprintf(stdout, "\n*************************************************");
        fprintf(stdout, "\n Pairing state for %s is BOND NONE", bd_name.c_str());
        fprintf(stdout, "\n*************************************************\n");
        g_bt_app->status.pairing_cmd = COMMAND_COMPLETE;
    }
}

void BluetoothApp:: HandleUnPair(bt_bdaddr_t bd_addr ) {
    bdstr_t bd_str;
    std::map<std::string, std::string>::iterator it;
    bdaddr_to_string(&bd_addr, &bd_str[0], sizeof(bd_str));
    std::string deviceAddress(bd_str);

    it = bonded_devices.find(deviceAddress);
    if (it != bonded_devices.end())
        bt_interface->remove_bond(&bd_addr);
    else
        fprintf( stdout, " Device is not in bonded list\n");
}

bt_bdaddr_t BluetoothApp:: AddFoundedDevice(std::string bd_name, bt_bdaddr_t bd_addr ) {

    ALOGI(LOGTAG " Adding Device to inquiry list");
    std::map<std::string, std::string>::iterator it;
    bdstr_t bd_str;
    bdaddr_to_string(&bd_addr, &bd_str[0], sizeof(bd_str));
    std::string deviceAddress(bd_str);

    it = inquiry_list.find(deviceAddress);
    if (it != inquiry_list.end()) {
        return (bd_addr);
    } else {
        inquiry_list[deviceAddress] = bd_name;
        return bd_addr;
    }
}

bt_state_t BluetoothApp:: GetState() {
    return bt_state;
}

void BluetoothApp:: PrintInquiryList() {

    fprintf(stdout, "\n**************************** Inquiry List \
*********************************\n");
    std::map<std::string, std::string>::iterator it;
    for (it = inquiry_list.begin(); it != inquiry_list.end(); ++it)
        fprintf(stdout, "%-*s %s\n", 50, it->second.data(), it->first.data());
    fprintf(stdout, "**************************** End of List \
*********************************\n");
}


void BluetoothApp:: PrintBondedDeviceList() {

    fprintf(stdout, "\n**************************** Bonded Device List \
**************************** \n");
    std::map<std::string, std::string>::iterator it;
    for (it = bonded_devices.begin(); it != bonded_devices.end(); ++it)
        fprintf(stdout, "%-*s %s\n", 50, it->second.data(), it->first.data());

    fprintf(stdout, "****************************  End of List \
*********************************\n");
}

bool BluetoothApp :: LoadBtStack (void) {
    hw_module_t *module;

    if (hw_get_module (BT_STACK_MODULE_ID, (hw_module_t const **) &module)) {
        ALOGE(LOGTAG " hw_get_module failed");
        return false;
    }

    if (module->methods->open (module, BT_STACK_MODULE_ID, &device_)) {
        return false;
    }

    bt_device_ = (bluetooth_device_t *) device_;
    bt_interface = bt_device_->get_bluetooth_interface ();
    if (!bt_interface) {
        bt_device_->common.close ((hw_device_t *) & bt_device_->common);
        bt_device_ = NULL;
        return false;
    }
    return true;
}


void BluetoothApp :: UnLoadBtStack (void)
{
    if (bt_interface) {
        bt_interface->cleanup ();
        bt_interface = NULL;
    }

    if (bt_device_) {
        bt_device_->common.close ((hw_device_t *) & bt_device_->common);
        bt_device_ = NULL;
    }
}


void BluetoothApp :: InitHandler (void) {

    if (!LoadBtStack())
        return;
    // Starting GAP Thread
    threadInfo[THREAD_ID_GAP].thread_id = thread_new (
            threadInfo[THREAD_ID_GAP].thread_name);

    if (threadInfo[THREAD_ID_GAP].thread_id) {
        g_gap = new Gap (bt_interface, config);
    }

    if ((is_hfp_client_enabled_) || (is_a2dp_sink_enabled_)) {
        // we need to start BT-AM if either of A2DP_SINK or HFP-Client is enabled
        threadInfo[THREAD_ID_BT_AM].thread_id = thread_new (
                        threadInfo[THREAD_ID_BT_AM].thread_name);
        if (threadInfo[THREAD_ID_BT_AM].thread_id) {
             pBTAM = new BT_Audio_Manager (bt_interface, config);
        }
    }

    if(is_a2dp_sink_enabled_) {
        threadInfo[THREAD_ID_A2DP_SINK].thread_id = thread_new (
                threadInfo[THREAD_ID_A2DP_SINK].thread_name);

        if (threadInfo[THREAD_ID_A2DP_SINK].thread_id) {
            pA2dpSink = new A2dp_Sink (bt_interface, config);
        }
    }

    if(is_avrcp_enabled_) {
        threadInfo[THREAD_ID_AVRCP].thread_id = thread_new (
                threadInfo[THREAD_ID_AVRCP].thread_name);

        if (threadInfo[THREAD_ID_AVRCP].thread_id) {
            pAvrcp = new Avrcp (bt_interface, config);
        }

    }

    if(is_a2dp_source_enabled_) {
        threadInfo[THREAD_ID_A2DP_SOURCE].thread_id = thread_new (
                threadInfo[THREAD_ID_A2DP_SOURCE].thread_name);

        if (threadInfo[THREAD_ID_A2DP_SOURCE].thread_id) {
            pA2dpSource = new A2dp_Source (bt_interface, config);
        }
    }

    if(is_hfp_client_enabled_) {
        threadInfo[THREAD_ID_HFP_CLIENT].thread_id = thread_new (
                threadInfo[THREAD_ID_HFP_CLIENT].thread_name);

        if (threadInfo[THREAD_ID_HFP_CLIENT].thread_id) {
            pHfpClient = new Hfp_Client(bt_interface, config);
        }
    }

    if(is_hfp_ag_enabled_) {
        threadInfo[THREAD_ID_HFP_AG].thread_id = thread_new (
                threadInfo[THREAD_ID_HFP_AG].thread_name);

        if (threadInfo[THREAD_ID_HFP_AG].thread_id) {
            pHfpAG = new Hfp_Ag(bt_interface, config);
        }
    }

    // registers reactors for socket
    if (is_socket_input_enabled_) {
        if(LocalSocketCreate() != -1) {
            listen_reactor_ = reactor_register (
                thread_get_reactor (threadInfo[THREAD_ID_MAIN].thread_id),
                listen_socket_local_, NULL, BtSocketListenHandler, NULL);
        }
    }

    // Enable Bluetooth
    if (is_bt_enable_default_) {

        BtEvent *event = new BtEvent;
        event->event_id = GAP_API_ENABLE;
        ALOGV (LOGTAG "  Posting enable to GAP thread");
        PostMessage (THREAD_ID_GAP, event);

    }

    threadInfo[THREAD_ID_SDP_CLIENT].thread_id = thread_new (
        threadInfo[THREAD_ID_SDP_CLIENT].thread_name);

    if (threadInfo[THREAD_ID_SDP_CLIENT].thread_id)
        g_sdpClient = new SdpClient(bt_interface, config);

    if (is_pan_enable_default_) {
        // Starting PAN Thread
        threadInfo[THREAD_ID_PAN].thread_id = thread_new (
            threadInfo[THREAD_ID_PAN].thread_name);

        if (threadInfo[THREAD_ID_PAN].thread_id)
            g_pan = new Pan (bt_interface, config);
    }

    if (is_gatt_enable_default_) {
        ALOGV (LOGTAG "  Starting GATT thread");
        threadInfo[THREAD_ID_GATT].thread_id = thread_new (
            threadInfo[THREAD_ID_GATT].thread_name);

        if (threadInfo[THREAD_ID_GATT].thread_id)
            g_gatt = new Gatt(bt_interface, config);
    }

#ifdef USE_BT_OBEX
    if (is_obex_enabled_ && is_pbap_client_enabled_) {
        threadInfo[THREAD_ID_PBAP_CLIENT].thread_id = thread_new (
            threadInfo[THREAD_ID_PBAP_CLIENT].thread_name);

        if (threadInfo[THREAD_ID_PBAP_CLIENT].thread_id)
            g_pbapClient = new PbapClient(bt_interface, config);
    }
    if (is_obex_enabled_ && is_opp_enabled_) {
        threadInfo[THREAD_ID_OPP].thread_id = thread_new (
            threadInfo[THREAD_ID_OPP].thread_name);

        if (threadInfo[THREAD_ID_OPP].thread_id)
            g_opp = new Opp(bt_interface, config);
    }
    opp_incoming_file_accept_timer = NULL;
    if( !(opp_incoming_file_accept_timer = alarm_new())) {
        ALOGE(LOGTAG " unable to create opp_connect_timer");
        opp_incoming_file_accept_timer = NULL;
    }
#endif

    // Enable Command line input
    if (is_user_input_enabled_) {
        cmd_reactor_ = reactor_register (thread_get_reactor
                        (threadInfo[THREAD_ID_MAIN].thread_id),
                        STDIN_FILENO, NULL, BtCmdHandler, NULL);
    }
}


void BluetoothApp :: DeInitHandler (void) {
    UnLoadBtStack ();

    ALOGV (LOGTAG "  %s:",__func__);
     // de-register reactors for socket
    if (is_socket_input_enabled_) {
        if(listen_reactor_)
        {
            reactor_unregister ( listen_reactor_);
            listen_reactor_ = NULL;
        }
        if(accept_reactor_)
        {
            reactor_unregister ( accept_reactor_);
            accept_reactor_ = NULL;
        }
    }

    if ((is_hfp_client_enabled_) || (is_a2dp_sink_enabled_)) {
        if (threadInfo[THREAD_ID_BT_AM].thread_id != NULL) {
            thread_free (threadInfo[THREAD_ID_BT_AM].thread_id);
            if ( pBTAM != NULL)
                delete pBTAM;
        }
    }

    if(is_a2dp_sink_enabled_) {
        //STOP A2dp Sink thread
        if (threadInfo[THREAD_ID_A2DP_SINK].thread_id != NULL) {
            thread_free (threadInfo[THREAD_ID_A2DP_SINK].thread_id);
            if ( pA2dpSink != NULL)
                delete pA2dpSink;
        }
    }

    if(is_avrcp_enabled_) {
        //STOP Avrcp thread
        if (threadInfo[THREAD_ID_AVRCP].thread_id != NULL) {
            thread_free (threadInfo[THREAD_ID_AVRCP].thread_id);
            if ( pAvrcp != NULL)
                delete pAvrcp;
        }
    }

    if(is_a2dp_source_enabled_) {
        //STOP A2dp Source thread
        if (threadInfo[THREAD_ID_A2DP_SOURCE].thread_id != NULL) {
            thread_free (threadInfo[THREAD_ID_A2DP_SOURCE].thread_id);
            if ( pA2dpSource!= NULL)
                delete pA2dpSource;
        }
    }

    if(is_hfp_client_enabled_) {
        //STOP HFP client thread
        if (threadInfo[THREAD_ID_HFP_CLIENT].thread_id != NULL) {
            thread_free (threadInfo[THREAD_ID_HFP_CLIENT].thread_id);
            if ( pHfpClient != NULL)
                delete pHfpClient;
        }
    }

    if(is_hfp_ag_enabled_) {
        //STOP HFP AG thread
        if (threadInfo[THREAD_ID_HFP_AG].thread_id != NULL) {
            thread_free (threadInfo[THREAD_ID_HFP_AG].thread_id);
            if ( pHfpAG != NULL)
                delete pHfpAG;
        }
    }

    // Stop GAP Thread
    if (threadInfo[THREAD_ID_GAP].thread_id != NULL) {
        thread_free (threadInfo[THREAD_ID_GAP].thread_id);
        if ( g_gap != NULL)
            delete g_gap;
    }

    // Stop SDP Client Thread
    if (threadInfo[THREAD_ID_SDP_CLIENT].thread_id != NULL) {
        thread_free (threadInfo[THREAD_ID_SDP_CLIENT].thread_id);
        if ( g_sdpClient != NULL)
            delete g_sdpClient;
    }

    if (is_pan_enable_default_) {
        // Stop PAN Thread
        if (threadInfo[THREAD_ID_PAN].thread_id != NULL) {
            thread_free (threadInfo[THREAD_ID_PAN].thread_id);
            if (g_pan != NULL)
                delete g_pan;
        }
    }

    if (is_gatt_enable_default_) {
        if (threadInfo[THREAD_ID_GATT].thread_id != NULL){
            thread_free(threadInfo[THREAD_ID_GATT].thread_id);
            if (g_gatt != NULL)
                delete g_gatt;
        }
    }

#ifdef USE_BT_OBEX
    if (opp_incoming_file_accept_timer) {
        alarm_free(opp_incoming_file_accept_timer);
        opp_incoming_file_accept_timer = NULL;
    }
    if (is_obex_enabled_ && is_pbap_client_enabled_) {
        // Stop PBAP Client Thread
        if (threadInfo[THREAD_ID_PBAP_CLIENT].thread_id != NULL) {
            thread_free (threadInfo[THREAD_ID_PBAP_CLIENT].thread_id);
            if (g_pbapClient!= NULL)
                delete g_pbapClient;
        }
    }
    if (is_obex_enabled_ && is_opp_enabled_) {
        // Stop Opp Thread
        if (threadInfo[THREAD_ID_OPP].thread_id != NULL) {
            thread_free (threadInfo[THREAD_ID_OPP].thread_id);
            if (g_opp!= NULL)
                delete g_opp;
        }
    }
#endif

    // Stop Command Handler
    if (is_user_input_enabled_) {
        reactor_unregister (cmd_reactor_);
    }
}

BluetoothApp :: BluetoothApp () {

    // Initial values
    is_bt_enable_default_ = false;
    is_user_input_enabled_ = false;
    ssp_notification = false;
    pin_notification =false;
    listen_socket_local_ = -1;
    client_socket_ = -1;
    cmd_reactor_ = NULL;
    listen_reactor_ = NULL;
    accept_reactor_ = NULL;

    bt_state = BT_STATE_OFF;
    bt_discovery_state = BT_DISCOVERY_STOPPED;

    memset (&status, '\0', sizeof (UiCommandStatus));
    config = NULL;

    if (!LoadConfigParameters (CONFIG_FILE_PATH))
        ALOGE (LOGTAG " Error in Loading config file");
}

BluetoothApp :: ~BluetoothApp () {
    if (config)
        config_remove(config);

    bonded_devices.clear();
    inquiry_list.clear();
}

int BluetoothApp:: LocalSocketCreate(void) {
  int conn_sk, length;
  struct sockaddr_un addr;

  listen_socket_local_ = socket(AF_LOCAL, SOCK_STREAM, 0);
  if(listen_socket_local_ < 0) {
    ALOGE (LOGTAG "Failed to create Local Socket 1 (%s)", strerror(errno));
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_LOCAL;
  strlcpy(addr.sun_path, LOCAL_SOCKET_NAME, sizeof(addr.sun_path));
  unlink(LOCAL_SOCKET_NAME);
  if (bind(listen_socket_local_, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    ALOGE (LOGTAG "Failed to create Local Socket (%s)", strerror(errno));
    return -1;
  }

  if (listen(listen_socket_local_, 1) < 0) {
    ALOGE (LOGTAG "Local socket listen failed (%s)", strerror(errno));
    close(listen_socket_local_);
    return -1;
  }
  return listen_socket_local_;
}

bool BluetoothApp::LoadConfigParameters (const char *configpath) {

    bool is_bt_ext_ldo, fw_snoop_enable,soc_log_enable;
    config = config_new (configpath);
    if (!config) {
        ALOGE (LOGTAG " Unable to open config file");
        return false;
    }
    opensocket();
    is_bt_ext_ldo = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_ENABLE_EXT_POWER, false);
    if(is_bt_ext_ldo){
        property_set_bt("wc_transport.extldo", "enabled");
    }else{
        property_set_bt("wc_transport.extldo", "disabled");
    }

    fw_snoop_enable = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_ENABLE_FW_SNOOP, false);
    if(fw_snoop_enable){
        property_set_bt("persist.service.bdroid.fwsnoop", "true");
    }else{
        property_set_bt("persist.service.bdroid.fwsnoop", "false");
    }

    soc_log_enable = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_ENABLE_SOC_LOG, false);
    if(soc_log_enable){
        property_set_bt("persist.service.bdroid.soclog", "true");
    }else{
        property_set_bt("persist.service.bdroid.soclog", "false");
    }

    closesocket();
    // checking for the BT Enable option in config file
    is_bt_enable_default_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_ENABLE_DEFAULT, false);

    //checking for user input
    is_user_input_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_USER_INPUT, false);
    //checking for socket handler
    is_socket_input_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_SOCKET_ENABLED, false);

    //checking for a2dp sink
    is_a2dp_sink_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_A2DP_SINK_ENABLED, false);
    //checking for avrcp
    is_avrcp_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_AVRCP_ENABLED, false);

    //checking for a2dp source
    is_a2dp_source_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_A2DP_SOURCE_ENABLED, false);

    //checking for hfp client
    is_hfp_client_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_HFP_CLIENT_ENABLED, false);
    //checking for hfp ag
    is_hfp_ag_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_HFP_AG_ENABLED, false);

    if (is_hfp_client_enabled_ == true && is_hfp_ag_enabled_ == true) {
        ALOGE (LOGTAG " Both HFP AG and Client are enabled, disabling AG. Set \
           BtHfpAGEnable to true, BtHfClientEnable to false in bt_app.conf to \
           enable only AG");
        fprintf(stdout, " Both HFP AG and Client are enabled, disabling AG. Set \
           BtHfpAGEnable to true, BtHfClientEnable to false in bt_app.conf to \
           enable only AG\n" );
        is_hfp_ag_enabled_ = false;
    }
    //checking for Pan handler
    is_pan_enable_default_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_PAN_ENABLED, false);

    //checking for Gatt handler
    is_gatt_enable_default_= config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_GATT_ENABLED, false);

#ifdef USE_BT_OBEX
    //checking for OBEX handler
    is_obex_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_OBEX_ENABLED, false);

    //checking for Pbap Client handler
    is_pbap_client_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_PBAP_CLIENT_ENABLED, false);

    //checking for OPP handler
    is_opp_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_OPP_ENABLED, false);
#endif
    return true;
}

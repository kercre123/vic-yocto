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

#include <list>
#include <map>
#include <iostream>
#include <string.h>
#include <hardware/bluetooth.h>
#include <hardware/hardware.h>
#include <hardware/bt_rc.h>

#include "Avrcp.hpp"
#include "A2dp_Sink_Streaming.hpp"
#include "Gap.hpp"
#include "hardware/bt_rc_vendor.h"
#include "A2dp_Sink.hpp"
#include <math.h>
#include <algorithm>

#define LOGTAG "AVRCP"
#define LOGTAG_CTRL "AVRCP_CTRL"

using namespace std;
using std::list;
using std::string;

Avrcp *pAvrcp = NULL;
extern A2dp_Sink_Streaming *pA2dpSinkStream;
extern A2dp_Sink *pA2dpSink;

static const bt_bdaddr_t bd_addr_null= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#define ABS_VOL_BASE 127
#define AUDIO_MAX_VOL_LEVEL 15
int curr_audio_index = 1;
#ifdef __cplusplus
extern "C" {
#endif

#define AVRC_CAP_COMPANY_ID                     0x02
#define AVRC_CAP_EVENTS_SUPPORTED               0x03

#define AVRC_ITEM_PLAYER            0x01
#define AVRC_ITEM_FOLDER            0x02
#define AVRC_ITEM_MEDIA             0x03

#define BE_STREAM_TO_UINT64(u64, p) {u64 = ((uint64_t)(*((p) + 7)) + ((uint64_t)(*((p) + 6)) << 8) + \
                                    ((uint64_t)(*((p) + 5)) << 16) + ((uint64_t)(*((p) + 4)) << 24) + \
                                    ((uint64_t)(*((p) + 3)) << 32) + ((uint64_t)(*((p) + 2)) << 40) + \
                                    ((uint64_t)(*((p) + 1)) << 48) + ((uint64_t)(*(p)) << 56));}



/* Define the events that can be registered for notifications
*/
#define AVRC_EVT_PLAY_STATUS_CHANGE             0x01
#define AVRC_EVT_TRACK_CHANGE                   0x02
#define AVRC_EVT_TRACK_REACHED_END              0x03
#define AVRC_EVT_TRACK_REACHED_START            0x04
#define AVRC_EVT_PLAY_POS_CHANGED               0x05
#define AVRC_EVT_BATTERY_STATUS_CHANGE          0x06
#define AVRC_EVT_SYSTEM_STATUS_CHANGE           0x07
#define AVRC_EVT_APP_SETTING_CHANGE             0x08
/* added in AVRCP 1.4 */
#define AVRC_EVT_NOW_PLAYING_CHANGE             0x09
#define AVRC_EVT_AVAL_PLAYERS_CHANGE            0x0a
#define AVRC_EVT_ADDR_PLAYER_CHANGE             0x0b
#define AVRC_EVT_UIDS_CHANGE                    0x0c
#define AVRC_EVT_VOLUME_CHANGE                  0x0d


void BtAvrcpMsgHandler(void *msg) {
    BtEvent* pEvent = NULL;
    BtEvent* pCleanupEvent = NULL;
    if(!msg) {
        printf("Msg is NULL, return.\n");
        return;
    }

    pEvent = ( BtEvent *) msg;
    switch(pEvent->event_id) {
        case PROFILE_API_START:
            ALOGD(LOGTAG " enable avrcp");
            if (pAvrcp) {
                pAvrcp->HandleEnableAvrcp();
            }
            break;
        case PROFILE_API_STOP:
            ALOGD(LOGTAG " disable avrcp");
            if (pAvrcp) {
                pAvrcp->HandleDisableAvrcp();
            }
            break;
        case AVRCP_CLEANUP_REQ:
            ALOGD(LOGTAG " cleanup a2dp avrcp");
            pCleanupEvent = new BtEvent;
            pCleanupEvent->event_id = AVRCP_CLEANUP_DONE;
            PostMessage(THREAD_ID_GAP, pCleanupEvent);
            break;
        case AVRCP_CTRL_CONNECTED_CB:
        case AVRCP_CTRL_DISCONNECTED_CB:
        case AVRCP_CTRL_PASS_THRU_CMD_REQ:
        case AVRCP_CTRL_REG_NOTI_ABS_VOL_CB:
        case AVRCP_CTRL_VOL_CHANGED_NOTI_REQ:
        case AVRCP_CTRL_SET_ABS_VOL_CMD_CB:
            ALOGD( LOGTAG_CTRL " handle avrcp ctrl pass through events ");
            if (pAvrcp) {
                pAvrcp->HandleAvrcpCTPassThruEvents(( BtEvent *) msg);
            }
            break;
        case AVRCP_CTRL_GET_CAP_REQ:
        case AVRCP_CTRL_LIST_PALYER_SETTING_ATTR_REQ:
        case AVRCP_CTRL_LIST_PALYER_SETTING_VALUE_REQ:
        case AVRCP_CTRL_GET_PALYER_APP_SETTING_REQ:
        case AVRCP_CTRL_SET_PALYER_APP_SETTING_VALUE_REQ:
        case AVRCP_CTRL_GET_ELEMENT_ATTR_REQ:
        case AVRCP_CTRL_GET_PLAY_STATUS_REQ:
        case AVRCP_CTRL_REG_NOTIFICATION_REQ:
        case AVRCP_CTRL_SET_ADDRESSED_PLAYER_REQ:
        case AVRCP_CTRL_SET_BROWSED_PLAYER_REQ:
        case AVRCP_CTRL_CHANGE_PATH_REQ:
        case AVRCP_CTRL_GET_FOLDER_ITEMS_REQ:
        case AVRCP_CTRL_GET_ITEM_ATTRIBUTES_REQ:
        case AVRCP_CTRL_PLAY_ITEMS_REQ:
        case AVRCP_CTRL_ADDTO_NOW_PLAYING_REQ:
        case AVRCP_CTRL_SEARCH_REQ:

            ALOGD( LOGTAG_CTRL " handle avrcp ctrl events ");
            if (pAvrcp) {
                pAvrcp->HandleAvrcpCTEvents(( BtEvent *) msg);
            }
            if(pEvent->avrcpCtrlEvent.buf_ptr != NULL)
                delete pEvent->avrcpCtrlEvent.buf_ptr;
            if(pEvent->avrcpCtrlEvent.buf_ptr32 != NULL)
                delete pEvent->avrcpCtrlEvent.buf_ptr32;
            break;
        default:
            break;
    }
    delete pEvent;
}

#ifdef __cplusplus
}
#endif


static void btavrcpctrl_passthru_rsp_vendor_callback(int id, int key_state, bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_passthru_rsp_vendor_callback id = %d key_state = %d",
            id, key_state);
    if (id == CMD_ID_PAUSE && key_state == 1 &&
            !memcmp(&pA2dpSinkStream->mStreamingDevice, bd_addr, sizeof(bt_bdaddr_t)))
    {
        ALOGD(LOGTAG_CTRL " need to flush both stack queue and audio queue ");
        BtEvent *pFlushAudioPackets = new BtEvent;
        pFlushAudioPackets->a2dpSinkStreamingEvent.event_id = A2DP_SINK_STREAMING_FLUSH_AUDIO;
        memcpy(&pFlushAudioPackets->a2dpSinkStreamingEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
        if (pA2dpSinkStream) {
            thread_post(pA2dpSinkStream->threadInfo.thread_id,
            pA2dpSinkStream->threadInfo.thread_handler, (void*)pFlushAudioPackets);
        }
    }
}

static void btavrcpctrl_passthru_rsp_callback(int id, int key_state) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_passthru_rsp_callback id = %d key_state = %d", id, key_state);
}

static void btavrcpctrl_groupnavigation_rsp_callback(int id, int key_state) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_groupnavigation_rsp_callback id = %d key_state = %d", id, key_state);
}

static void btavrcpctrl_setplayerapplicationsetting_rsp_callback(bt_bdaddr_t *bd_addr, uint8_t accepted) {
    ALOGD(LOGTAG_CTRL " btavrcctrl_setplayerapplicationsetting_rsp_callback accepted = %d", accepted);
    fprintf(stdout, "<-- setplayerapplicationsetting rsp message received! \n" );
    fprintf(stdout, "     accepted : 0x%02x   \n", accepted);

}

static void btavrcpctrl_playerapplicationsetting_callback(bt_bdaddr_t *bd_addr, uint8_t num_attr,
                                                          btrc_player_app_attr_t *app_attrs,
                                                          uint8_t num_ext_attr, btrc_player_app_ext_attr_t *ext_attrs) {
     ALOGD(LOGTAG_CTRL " btavrcpctrl_playerapplicationsetting_callback");
}
 
static void btavrcpctrl_playerapplicationsetting_changed_callback(bt_bdaddr_t *bd_addr, btrc_player_settings_t *p_vals) {
     ALOGD(LOGTAG_CTRL " btrc_ctrl_playerapplicationsetting_changed_callback");
}

static void btavrcpctrl_track_changed_callback(bt_bdaddr_t *bd_addr, uint8_t num_attr,
                                                     btrc_element_attr_val_t *p_attrs) {
    ALOGD(LOGTAG_CTRL "btrc_ctrl_track_changed_callback");
}

static void btavrcpctrl_play_position_changed_callback(bt_bdaddr_t *bd_addr,
                                                          uint32_t song_len, uint32_t song_pos, btrc_play_status_t play_status) {
    ALOGD(LOGTAG_CTRL "btrc_ctrl_play_position_changed_callback");
}

static void btavrcpctrl_play_status_changed_callback(bt_bdaddr_t *bd_addr, btrc_play_status_t play_status) {
    ALOGD(LOGTAG_CTRL "btrc_ctrl_play_status_changed_callback");
}

static void btavrcpctrl_connection_state_callback(bool state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_connection_state_callback state = %d", state);
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->avrcpCtrlPassThruEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    if (state == true)
    {
        fprintf(stdout, "     AVRCP_CTRL_CONNECTED_CB\n");
        pEvent->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_CONNECTED_CB;
    }
    else
    {
        fprintf(stdout, "     AVRCP_CTRL_DISCONNECTED_CB\n");
        pEvent->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_DISCONNECTED_CB;
    }
    PostMessage(THREAD_ID_AVRCP, pEvent);
}

static bt_status_t btavrcpctrl_br_connection_state_vendor_callback(bool state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_br_connection_state_vendor_callback state = %d", state);
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->avrcpCtrlPassThruEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    if (state == true)
    {
        fprintf(stdout, "     AVRCP_CTRL_BR_CONNECTED_CB\n");
    }
    else
    {
        fprintf(stdout, "     AVRCP_CTRL_BR_DISCONNECTED_CB\n");

    }
//    PostMessage(THREAD_ID_AVRCP, pEvent);
}

static void btavrcpctrl_getrcfeatures_callback( bt_bdaddr_t* bd_addr, int features) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_rcfeatures_vendor_callback features = %d", features);
}

static void btavrcpctrl_getcap_rsp_vendor_callback( bt_bdaddr_t *bd_addr, int cap_id,
                uint32_t* supported_values, int num_supported, uint8_t rsp_type) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_getcap_rsp_vendor_callback");
    fprintf(stdout, "<-- getcap rsp message received! \n" );
    fprintf(stdout, "     num_supported:%d, rsp_type:%d \n", num_supported,rsp_type);
    for(int i=0; i < num_supported; i++)
    {
        if(cap_id == AVRC_CAP_COMPANY_ID)
            fprintf(stdout, "     CompanyID%d: 0x%d\n", i,supported_values[i]);
        else if(cap_id == AVRC_CAP_EVENTS_SUPPORTED)
        {
            uint8_t* event = (uint8_t*)supported_values;
            fprintf(stdout, "     SupportedEvent%d: 0x%d\n", i,event[i]);
        }
    }

}

static void btavrcpctrl_listplayerappsettingattrib_rsp_vendor_callback( bt_bdaddr_t *bd_addr,
                          uint8_t* supported_attribs, int num_attrib, uint8_t rsp_type) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_listplayerappsettingattrib_rsp_vendor_callback");
    fprintf(stdout, "<-- listplayerappsettingattrib rsp message received! \n" );
    fprintf(stdout, "     num_attrib:%d, rsp_type:%d \n", num_attrib,rsp_type);
    for(int i=0; i < num_attrib; i++)
        fprintf(stdout, "     PlayerApplicationSettingAttributeID%d: 0x%d\n", i,supported_attribs[i]);
}

static void btavrcpctrl_listplayerappsettingvalue_rsp_vendor_callback( bt_bdaddr_t *bd_addr,
                       uint8_t* supported_val, uint8_t num_supported, uint8_t rsp_type) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_listplayerappsettingvalue_rsp_vendor_callback");
    fprintf(stdout, "<-- listplayerappsettingvalue rsp message received! \n" );
    fprintf(stdout, "     num_supported:%d, rsp_type:%d \n", num_supported, rsp_type);
    for(int i=0; i < num_supported; i++)
        fprintf(stdout, "     PlayerApplicationSettingAttributeValue%d: 0x%x\n", i,supported_val[i]);
}

static void btavrcpctrl_currentplayerappsetting_rsp_vendor_callback( bt_bdaddr_t *bd_addr,
        uint8_t* supported_ids, uint8_t* supported_val, uint8_t num_attrib, uint8_t rsp_type) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_currentplayerappsetting_rsp_vendor_callback");
    fprintf(stdout, "<-- getplayerappsetting rsp message received! \n" );
    fprintf(stdout, "     num_attrib:%d, rsp_type:%d \n", num_attrib, rsp_type);
    for(int i=0; i < num_attrib; i++)
        fprintf(stdout, "     CurrentPlayerApplicationSetting Attribute ID: 0x%x Value: 0x%x\n",
        supported_ids[i], supported_val[i]);
}

static bt_status_t btavrcpctrl_notification_rsp_vendor_callback( bt_bdaddr_t *bd_addr, btrc_event_id_t event_id,
         btrc_notification_type_t type,btrc_register_notification_t *p_param) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_notification_rsp_vendor_callback");
    fprintf(stdout, "<-- notification message received! \n" );
    fprintf(stdout, "     event_id:%d, rsp_type:%d \n", event_id, type);
    switch(event_id)
    {
    case AVRC_EVT_PLAY_STATUS_CHANGE:
        fprintf(stdout, "     EVENT_PLAYBACK_STATUS_CHANGED play_status: 0x%x\n", p_param->play_status);
        break;

    case AVRC_EVT_TRACK_CHANGE:
        fprintf(stdout, "     EVENT_TRACK_CHANGED track: 0x%x\n", p_param->track);
        break;

    case AVRC_EVT_APP_SETTING_CHANGE:
        for(int i=0; i < p_param->player_setting.num_attr; i++)
            fprintf(stdout, "     EVENT_APP_SETTING_CHANGED attr_id%d: 0x%x  attr_value%d: 0x%x\n", i,
            p_param->player_setting.attr_ids[i], i, p_param->player_setting.attr_values[i]);

    break;

    case AVRC_EVT_PLAY_POS_CHANGED:
        fprintf(stdout, "     EVENT_PLAY_POS_CHANGED play_pos: 0x%x\n", p_param->song_pos);
    break;

    case AVRC_EVT_NOW_PLAYING_CHANGE:
        fprintf(stdout, "     EVENT_NOW_PLAYING_CHANGED\n");
    break;

    case AVRC_EVT_AVAL_PLAYERS_CHANGE:
        fprintf(stdout, "     EVENT_AVAL_PLAYERS_CHANGED\n");
    break;

    case AVRC_EVT_ADDR_PLAYER_CHANGE:
        fprintf(stdout, "     EVENT_ADDR_PLAYER_CHANGED player_id: 0x%x uid_counter: 0x%x\n",
            p_param->addr_player.player_id, p_param->addr_player.uid_counter);
    break;

    case AVRC_EVT_VOLUME_CHANGE:
        fprintf(stdout, "     EVENT_VOLUME_CHANGED play_pos: 0x%x\n", p_param->volume);
    break;

    default:
    break;
    }
    return BT_STATUS_SUCCESS;
}

static void btavrcpctrl_getelementattrib_rsp_vendor_callback(bt_bdaddr_t *bd_addr, uint8_t num_attributes,
       btrc_element_attr_val_t* p_attrs, uint8_t rsp_type) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_getelementattrib_rsp_vendor_callback");
    fprintf(stdout, "<-- getelementattrib rsp message received! \n" );
    fprintf(stdout, "     num_attrib:%d, rsp_type:%d \n", num_attributes, rsp_type);
    for(int i=0; i < num_attributes; i++)
        fprintf(stdout, "     AttributeID%d: name: %s\n", p_attrs[i].attr_id, p_attrs[i].text);
}

static bt_status_t btavrcpctrl_getplaystatus_rsp_vendor_callback( bt_bdaddr_t *bd_addr, btrc_play_status_t play_status,
        uint32_t song_len, uint32_t song_pos) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_getplaystatus_rsp_vendor_callback");
    fprintf(stdout, "<-- getplaystatus rsp message received! \n" );
    fprintf(stdout, "     play_status: 0x%x, song_len:%d, song_pos:%d \n",
        play_status, song_len, song_pos);
    return BT_STATUS_SUCCESS;

}

static bt_status_t btavrcpctrl_setaddressedplayer_rsp_vendor_callback(bt_bdaddr_t *bd_addr, btrc_status_t rsp_status)
{
    ALOGD(LOGTAG_CTRL " btavrcpctrl_setaddressedplayer_rsp_vendor_callback");
    fprintf(stdout, "<-- setaddressedplayer rsp message received! \n" );
    fprintf(stdout, "     responsed status: 0x%02x  \n", rsp_status);
    return BT_STATUS_SUCCESS;

}

static bt_status_t btavrcpctrl_setbrowsedplayer_rsp_vendor_callback(bt_bdaddr_t *bd_addr,
    btrc_status_t rsp_status, uint32_t num_items, uint16_t charset_id , uint8_t folder_depth, btrc_folder_name_t *p_folders)

{
    ALOGD(LOGTAG_CTRL " btavrcpctrl_setbrowsedplayer_rsp_vendor_callback");
    fprintf(stdout, "<-- setaddressedplayer rsp message received! \n" );
    fprintf(stdout, "     responsed status: 0x%02x  num_items:%d charset_id:%d folder_depth:%d \n", rsp_status,
        num_items, charset_id, folder_depth);
    for(int i=0; i < folder_depth; i++)
        fprintf(stdout, "     Folder name%d: %s\n", i, p_folders[i].p_str);
    return BT_STATUS_SUCCESS;

}

static bt_status_t btavrcpctrl_changepath_rsp_vendor_callback(bt_bdaddr_t *bd_addr, btrc_status_t rsp_status, uint32_t num_items)
{
    ALOGD(LOGTAG_CTRL " btavrcpctrl_changepath_rsp_vendor_callback");
    fprintf(stdout, "<-- changepath rsp message received! \n" );
    fprintf(stdout, "     responsed status: 0x%02x  \n", rsp_status);
    if(BTRC_STS_NO_ERROR == rsp_status)
        fprintf(stdout, "     number of items: %d  \n", num_items);
    return BT_STATUS_SUCCESS;

}

static bt_status_t btavrcpctrl_getfolderitems_rsp_vendor_callback(bt_bdaddr_t *bd_addr, uint32_t start_item,
    uint32_t end_item, btrc_status_t rsp_status, uint16_t num_items, btrc_folder_items_t *p_items)
{
    ALOGD(LOGTAG_CTRL " btavrcpctrl_getfolderitems_rsp_vendor_callback");
    fprintf(stdout, "<-- getfolderitems rsp message received! \n" );
    fprintf(stdout, "     responsed status: 0x%02x  \n", rsp_status);
    if(BTRC_STS_NO_ERROR == rsp_status)
    {
        fprintf(stdout, "     num_items: %d start: %d end: %d \n", num_items, start_item, end_item);
        if(num_items > 0)
        {
            switch(p_items->item_type)
            {
            case AVRC_ITEM_PLAYER:
                for(int i=0; i < num_items; i++)
                {
                    fprintf(stdout, "     Media Player Item %d name:%s \n",i, p_items[i].player.name);
                    fprintf(stdout, "           player_id:0x%04x  major_type:0x%02x sub_type:0x%02x play_status:0x%02x\n",
                        p_items[i].player.player_id,p_items[i].player.major_type,p_items[i].player.sub_type,p_items[i].player.play_status);
                    fprintf(stdout, "           features: ");
                    for(int n=0; n < 8; n++)
                        fprintf(stdout, "%02x", p_items[i].player.features[n]);
                    fprintf(stdout, "\n");
                }
                break;
            case AVRC_ITEM_FOLDER:
                for(int i=0; i < num_items; i++)
                {
                    fprintf(stdout, "     Folder Item %d name:%s \n.",i, p_items[i].folder.name);
                    uint64_t UID;
                    BE_STREAM_TO_UINT64(UID,p_items[i].folder.uid);
                    fprintf(stdout, "           uid: %lld type:0x%02x playable:0x%02x \n", UID,
                        p_items[i].folder.type, p_items[i].folder.playable);
                }
                break;
            case AVRC_ITEM_MEDIA:
                for(int i=0; i < num_items; i++)
                {
                    fprintf(stdout, "     Media Element Item %d name:%s \n",i, p_items[i].media.name);
                    uint64_t UID;
                    BE_STREAM_TO_UINT64(UID,p_items[i].folder.uid);
                    fprintf(stdout, "           uid: %lld type:0x%02x num_attrs:0x%02x \n",
                        UID, p_items[i].media.type, p_items[i].media.num_attrs);
                    for(int n=0; n < p_items[i].media.num_attrs; n++)
                        fprintf(stdout, "           Attributes%d ID:%02x Name: %s \n",n, p_items[i].media.p_attrs[n].attr_id, p_items[i].media.p_attrs[n].text);
                }
                break;
            default:
                fprintf(stdout, "    unknown item type %d \n", p_items->item_type);
                break;
            }
        }
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t btavrcpctrl_getitemattributes_rsp_vendor_callback(bt_bdaddr_t *bd_addr,
    btrc_status_t rsp_status, uint8_t num_attr, btrc_element_attr_val_t *p_attrs)
{
    ALOGD(LOGTAG_CTRL " btavrcpctrl_getitemattributes_rsp_vendor_callback");
    fprintf(stdout, "<-- getitemattributes rsp message received! \n" );
    fprintf(stdout, "     responsed status: 0x%02x  num_attr:%d \n", rsp_status, num_attr);
    for(int i=0; i < num_attr; i++)
        fprintf(stdout, "     Attribute%d ID:0x%02x name: %s\n", i, p_attrs[i].attr_id, p_attrs[i].text);
    return BT_STATUS_SUCCESS;

}

static bt_status_t btavrcpctrl_playitem_rsp_vendor_callback(bt_bdaddr_t *bd_addr, btrc_status_t rsp_status )
{
    ALOGD(LOGTAG_CTRL " btavrcpctrl_playitem_rsp_vendor_callback");
    fprintf(stdout, "<-- playitem rsp message received! \n" );
    fprintf(stdout, "     responsed status: 0x%02x  \n", rsp_status);
    return BT_STATUS_SUCCESS;

}

static bt_status_t btavrcpctrl_addtonowplaying_rsp_vendor_callback(bt_bdaddr_t *bd_addr, btrc_status_t rsp_status )
{
    ALOGD(LOGTAG_CTRL " btavrcpctrl_addtonowplaying_rsp_vendor_callback");
    fprintf(stdout, "<-- addtonowplaying rsp message received! \n" );
    fprintf(stdout, "     responsed status: 0x%02x  \n", rsp_status);
    return BT_STATUS_SUCCESS;
}

static bt_status_t btavrcpctrl_search_rsp_vendor_callback(bt_bdaddr_t *bd_addr, btrc_status_t rsp_status,uint16_t uid_counter, uint32_t num_item )
{
    ALOGD(LOGTAG_CTRL " btavrcpctrl_search_rsp_vendor_callback");
    fprintf(stdout, "<-- search rsp message received! \n" );
    fprintf(stdout, "     responsed status: 0x%02x  uid_counter: 0x%02x num_item: %d \n",
        rsp_status, uid_counter, num_item);
    return BT_STATUS_SUCCESS;
}

static void btavrcpctrl_setabsvol_cmd_callback(bt_bdaddr_t *bd_addr, uint8_t abs_vol, uint8_t label) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_setabsvol_cmd_vendor_callback");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_SET_ABS_VOL_CMD_CB;
    pEvent->avrcpCtrlPassThruEvent.arg1 = label;
    pEvent->avrcpCtrlPassThruEvent.arg2 = abs_vol;
    memcpy(&pEvent->avrcpCtrlPassThruEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    PostMessage(THREAD_ID_AVRCP, pEvent);
}

static void btavrcpctrl_registernotification_absvol_callback(bt_bdaddr_t *bd_addr, uint8_t label) {
    ALOGD(LOGTAG_CTRL " btavrcpctrl_registernotification_absvol_vendor_callback");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpCtrlPassThruEvent.event_id = AVRCP_CTRL_REG_NOTI_ABS_VOL_CB;
    pEvent->avrcpCtrlPassThruEvent.arg1 = label;
    memcpy(&pEvent->avrcpCtrlPassThruEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    PostMessage(THREAD_ID_AVRCP, pEvent);
}

static btrc_ctrl_callbacks_t sBluetoothAvrcpCtrlCallbacks = {
   sizeof(sBluetoothAvrcpCtrlCallbacks),
   btavrcpctrl_passthru_rsp_callback,
   btavrcpctrl_groupnavigation_rsp_callback,
   btavrcpctrl_connection_state_callback,
   btavrcpctrl_getrcfeatures_callback,
   btavrcpctrl_setplayerapplicationsetting_rsp_callback,
   btavrcpctrl_playerapplicationsetting_callback,
   btavrcpctrl_playerapplicationsetting_changed_callback,
   btavrcpctrl_setabsvol_cmd_callback,
   btavrcpctrl_registernotification_absvol_callback,
   btavrcpctrl_track_changed_callback,
   btavrcpctrl_play_position_changed_callback,
   btavrcpctrl_play_status_changed_callback,
};

static btrc_ctrl_vendor_callbacks_t sBluetoothAvrcpCtrlVendorCallbacks = {
   sizeof(sBluetoothAvrcpCtrlVendorCallbacks),
   btavrcpctrl_getcap_rsp_vendor_callback,
   btavrcpctrl_listplayerappsettingattrib_rsp_vendor_callback,
   btavrcpctrl_listplayerappsettingvalue_rsp_vendor_callback,
   btavrcpctrl_currentplayerappsetting_rsp_vendor_callback,
   btavrcpctrl_notification_rsp_vendor_callback,
   btavrcpctrl_getelementattrib_rsp_vendor_callback,
   btavrcpctrl_getplaystatus_rsp_vendor_callback,
   btavrcpctrl_passthru_rsp_vendor_callback,
   btavrcpctrl_br_connection_state_vendor_callback,
   btavrcpctrl_setaddressedplayer_rsp_vendor_callback,
   btavrcpctrl_setbrowsedplayer_rsp_vendor_callback,
   btavrcpctrl_changepath_rsp_vendor_callback,
   btavrcpctrl_getfolderitems_rsp_vendor_callback,
   btavrcpctrl_getitemattributes_rsp_vendor_callback,
   btavrcpctrl_playitem_rsp_vendor_callback,
   btavrcpctrl_addtonowplaying_rsp_vendor_callback,
   btavrcpctrl_search_rsp_vendor_callback,
};

void Avrcp::SendPassThruCommandNative(uint8_t key_id, bt_bdaddr_t* addr, uint8_t direct) {
    ALOGD(LOGTAG_CTRL " SendPassThruCommandNative ");
    if (memcmp(&pA2dpSinkStream->mStreamingDevice, &bd_addr_null, sizeof(bt_bdaddr_t)) &&
            memcmp(&pA2dpSinkStream->mStreamingDevice, addr, sizeof(bt_bdaddr_t)) &&
            (key_id == CMD_ID_PLAY))
        direct = 1;

    if (!direct && pA2dpSinkStream && pA2dpSinkStream->use_bt_a2dp_hal &&
            ((key_id == CMD_ID_PAUSE) || (key_id == CMD_ID_PLAY))) {
        ALOGD(LOGTAG_CTRL " SendPassThruCommandNative: using bt_a2dp_hal ");
        if (CMD_ID_PAUSE == key_id)
        {
            ALOGD( LOGTAG_CTRL " sending stopped event ");

            if (!memcmp(&pA2dpSinkStream->mStreamingDevice, addr, sizeof(bt_bdaddr_t)))
            {
                pA2dpSinkStream->StopDataFetchTimer();
                pA2dpSinkStream->SuspendInputStream();
            }

        }
        if (CMD_ID_PLAY == key_id)
        {
            ALOGD( LOGTAG_CTRL " sending started event ");
            BtEvent *pEvent = new BtEvent;
            pEvent->a2dpSinkEvent.event_id = A2DP_SINK_AUDIO_STARTED;
            memcpy(&pEvent->a2dpSinkEvent.bd_addr, addr, sizeof(bt_bdaddr_t));
            PostMessage(THREAD_ID_A2DP_SINK, pEvent);
        }
    }
    else if (sBtAvrcpCtrlInterface != NULL) {
        ALOGD(LOGTAG_CTRL " SendPassThruCommandNative send_pass_through_cmd");

        sBtAvrcpCtrlInterface->send_pass_through_cmd(addr, key_id, 0);
        sBtAvrcpCtrlInterface->send_pass_through_cmd(addr, key_id, 1);
    }
}

list<A2dp_Device>::iterator FindAvDeviceByAddr(list<A2dp_Device>& pA2dpDev, bt_bdaddr_t dev) {
    list<A2dp_Device>::iterator p = pA2dpDev.begin();
    while(p != pA2dpDev.end()) {
        if (memcmp(&dev, &p->mDevice, sizeof(bt_bdaddr_t)) == 0) {
            break;
        }
        p++;
    }
    return p;
}

bool Avrcp::is_abs_vol_supported(bt_bdaddr_t bd_addr){
    list<A2dp_Device>::iterator iter;
    iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, bd_addr);
    if (iter != pA2dpSink->pA2dpDeviceList.end()) {
        return iter->mAbsVolNotificationRequested;
    }
    else {
        return false;
    }
}

int Avrcp::get_current_audio_index(){
    return curr_audio_index;
}

int getVolumePercentage() {
    int maxVolume = AUDIO_MAX_VOL_LEVEL;
                  //mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
    int currIndex = curr_audio_index;
                  //mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
    int percentageVol = ((currIndex * ABS_VOL_BASE) / maxVolume);
    return percentageVol;
}

void Avrcp::setAbsVolume(bt_bdaddr_t* dev, int absVol, int label) {
    int maxVolume = AUDIO_MAX_VOL_LEVEL;
                  //mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
    int currIndex = curr_audio_index;
                  //mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);

    // Ignore first volume command since phone may not know difference between stream volume
    // and amplifier volume.
    if (mFirstAbsVolCmdRecvd) {
        int newIndex =(int) round((double) absVol * maxVolume / ABS_VOL_BASE);
        ALOGD(LOGTAG_CTRL " setAbsVol = %d maxVol = %d cur = %d new = %d", absVol,
                                                  maxVolume, currIndex, newIndex);
        /*
              * In some cases change in percentage is not sufficient enough to warrant
              * change in index values which are in range of 0-15. For such cases
              * no action is requiredf
              */
        if (newIndex != currIndex) {
            curr_audio_index = newIndex;
            pA2dpSinkStream->SetStreamVol(curr_audio_index);
        }
    } else {
        mFirstAbsVolCmdRecvd = true;
        absVol = (currIndex * ABS_VOL_BASE) / maxVolume;
        ALOGD(LOGTAG_CTRL " SetAbsVol recvd for first time, respond with absVol %d", absVol);
    }
    sBtAvrcpCtrlInterface->set_volume_rsp(dev, absVol, label);
}


void Avrcp::HandleAvrcpCTPassThruEvents(BtEvent* pEvent) {
    list<A2dp_Device>::iterator iter;
    int perVol = 0;
    bdstr_t bd_str;
    std::list<std::string>::iterator bdstring;
    ALOGD(LOGTAG_CTRL " HandleAvrcpCTPassThruEvents event = %s",
            dump_message(pEvent->avrcpCtrlPassThruEvent.event_id));
    switch(pEvent->avrcpCtrlPassThruEvent.event_id) {
    case AVRCP_CTRL_CONNECTED_CB:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlPassThruEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end())
        {
            ALOGD(LOGTAG_CTRL " Rc connection for AV connected dev, mark avrcp connected");
            iter->mAvrcpConnected = true;
        }
        else
        {
            ALOGE(LOGTAG_CTRL " Rc connection from device without AV connection");
            bdaddr_to_string(&pEvent->avrcpCtrlPassThruEvent.bd_addr, &bd_str[0], sizeof(bd_str));
            std::string deviceAddress(bd_str);
            bdstring = std::find(rc_only_devices.begin(), rc_only_devices.end(), deviceAddress);
            if (bdstring == rc_only_devices.end())
            {
                ALOGE(LOGTAG_CTRL " RC connected for this dev w/o AV, cache this device in list");
                rc_only_devices.push_back(deviceAddress);
            }
            else
            {
                ALOGE(LOGTAG_CTRL " this RC device already in list, should never hit here, ERROR!!!");
            }
        }
        break;
    case AVRCP_CTRL_DISCONNECTED_CB:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlPassThruEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end())
        {
            ALOGD(LOGTAG_CTRL " Rc disconnection for AV connected dev, mark avrcp disconnected");
            iter->mAvrcpConnected = false;
        }
        else
        {
            ALOGE(LOGTAG_CTRL " Rc disconnection from device without AV connection");
            bdaddr_to_string(&pEvent->avrcpCtrlPassThruEvent.bd_addr, &bd_str[0], sizeof(bd_str));
            std::string deviceAddress(bd_str);
            bdstring = std::find(rc_only_devices.begin(), rc_only_devices.end(), deviceAddress);
            if (bdstring != rc_only_devices.end())
            {
                ALOGD (LOGTAG " found match for RC only disconnection, remove from list");
                rc_only_devices.remove(deviceAddress);
            }
            else
            {
                ALOGD (LOGTAG " found no match for RC only disconnection, entry was removed during AV connection");
            }
        }
        break;
    case AVRCP_CTRL_PASS_THRU_CMD_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlPassThruEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL " passthrough cmd for AV & RC connected device, send to stack");
            SendPassThruCommandNative(pEvent->avrcpCtrlPassThruEvent.key_id,
            &pEvent->avrcpCtrlPassThruEvent.bd_addr, 0);
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
    case AVRCP_CTRL_SET_ABS_VOL_CMD_CB:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlPassThruEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL " setabsvol cmd cb for AV & RC connected device, send to stack");
            setAbsVolume(&iter->mDevice, (int)pEvent->avrcpCtrlPassThruEvent.arg2,
                                         (int)pEvent->avrcpCtrlPassThruEvent.arg1);
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
    case AVRCP_CTRL_REG_NOTI_ABS_VOL_CB:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlPassThruEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL " NOTI_ABS_VOL_CB for AV & RC connected device, send to stack");
            iter->mNotificationLabel = (int)pEvent->avrcpCtrlPassThruEvent.arg1;
            iter->mAbsVolNotificationRequested = true;
            perVol = getVolumePercentage();
            ALOGD(LOGTAG_CTRL " Sending Interim Response = %d label %d", perVol,
                                                      iter->mNotificationLabel);
            sBtAvrcpCtrlInterface->register_abs_vol_rsp(&pEvent->avrcpCtrlPassThruEvent.bd_addr,
                    BTRC_NOTIFICATION_TYPE_INTERIM, perVol, iter->mNotificationLabel);
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
    case AVRCP_CTRL_VOL_CHANGED_NOTI_REQ:
        ALOGD(LOGTAG_CTRL " AVRCP_CTRL_VOL_CHANGED_NOTI_REQ, vol level = %d",
                                                 pEvent->avrcpCtrlPassThruEvent.arg1);
        iter = pA2dpSink->pA2dpDeviceList.begin();
        while(iter != pA2dpSink->pA2dpDeviceList.end()) {
                ALOGD(LOGTAG_CTRL " iter->mAvrcpConnected %d ", iter->mAvrcpConnected);
                ALOGD(LOGTAG_CTRL " iter->mAbsVolNotificationRequested %d",
                                    iter->mAbsVolNotificationRequested);
                if (iter->mAvrcpConnected && iter->mAbsVolNotificationRequested)
                {
                    perVol = (((int)pEvent->avrcpCtrlPassThruEvent.arg1*ABS_VOL_BASE)/AUDIO_MAX_VOL_LEVEL);
                    curr_audio_index = (int)pEvent->avrcpCtrlPassThruEvent.arg1;
                    ALOGD(LOGTAG_CTRL " perVol %d & mPreviousPercentageVol %d", perVol,
                                                    mPreviousPercentageVol);
                    if (perVol != mPreviousPercentageVol)
                    {
                        sBtAvrcpCtrlInterface->register_abs_vol_rsp(&iter->mDevice,
                                BTRC_NOTIFICATION_TYPE_CHANGED, perVol, iter->mNotificationLabel);
                        iter->mAbsVolNotificationRequested = false;
                    }
                }
                else
                    ALOGD(LOGTAG_CTRL " iter %x !conn to RC or !reg for Abs vol change noti", iter);
                iter++;
        }
        mPreviousPercentageVol = perVol;
        pA2dpSinkStream->SetStreamVol(curr_audio_index);
        break;

    }
}


void Avrcp::HandleAvrcpCTEvents(BtEvent* pEvent) {
    list<A2dp_Device>::iterator iter;
    int perVol;
    bdstr_t bd_str;
    std::list<std::string>::iterator bdstring;
    ALOGD(LOGTAG_CTRL " HandleAvrcpCTEvents event = %s",
            dump_message(pEvent->avrcpCtrlEvent.event_id));
    switch(pEvent->avrcpCtrlEvent.event_id) {
        case AVRCP_CTRL_GET_CAP_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_GET_CAP_REQ : getcapabilities_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->getcapabilities_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg1))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;

        case AVRCP_CTRL_LIST_PALYER_SETTING_ATTR_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_LIST_PALYER_SETTING_ATTR_REQ : list_player_app_setting_attrib_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->list_player_app_setting_attrib_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;

        case AVRCP_CTRL_LIST_PALYER_SETTING_VALUE_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_LIST_PALYER_SETTING_VALUE_REQ : list_player_app_setting_value_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->list_player_app_setting_value_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg1))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;

        case AVRCP_CTRL_GET_PALYER_APP_SETTING_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_GET_PALYER_APP_SETTING_REQ : get_player_app_setting_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->get_player_app_setting_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.num_attrb,
                    pEvent->avrcpCtrlEvent.buf_ptr))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;

        case AVRCP_CTRL_SET_PALYER_APP_SETTING_VALUE_REQ:
        {
            uint8_t* pValue = NULL;
            iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
            if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
            {
                ALOGD(LOGTAG_CTRL "AVRCP_CTRL_SET_PALYER_APP_SETTING_VALUE_REQ : set_player_app_setting_cmd called!~");
                pValue = (uint8_t*)pEvent->avrcpCtrlEvent.arg6;
                if (sBtAvrcpCtrlVendorInterface != NULL) {
                    if(BT_STATUS_SUCCESS == sBtAvrcpCtrlInterface->set_player_app_setting_cmd(&pEvent->avrcpCtrlEvent.bd_addr,
                        pEvent->avrcpCtrlEvent.num_attrb,
                        pEvent->avrcpCtrlEvent.buf_ptr,
                        pValue))
                        fprintf(stdout, "--> command has been successfully sent.\n" );
                    else
                        fprintf(stdout, "error: command not be accepted!.\n" );
                }
            }
            else
            {
                ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
            }
            if(pValue != NULL)
                delete pValue;
            break;
        }
        case AVRCP_CTRL_GET_ELEMENT_ATTR_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_GET_ELEMENT_ATTR_REQ : get_element_attribute_command_vendor called!~");
          if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->get_element_attribute_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.num_attrb,
                    pEvent->avrcpCtrlEvent.buf_ptr32))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
        case AVRCP_CTRL_GET_PLAY_STATUS_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_GET_PLAY_STATUS_REQ : list_player_app_setting_attrib_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->get_play_status_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
        case AVRCP_CTRL_SET_ADDRESSED_PLAYER_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_SET_ADDRESSED_PLAYER_REQ : set_addressed_player_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->set_addressed_player_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg3))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
        case AVRCP_CTRL_SET_BROWSED_PLAYER_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_SET_BROWSED_PLAYER_REQ : set_addressed_player_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->set_browsed_player_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg3))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
        case AVRCP_CTRL_CHANGE_PATH_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_CHANGE_PATH_REQ : change_path_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->change_folder_path_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg1, (uint8_t*)&pEvent->avrcpCtrlEvent.arg6))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
        case AVRCP_CTRL_GET_FOLDER_ITEMS_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_GET_FOLDER_ITEMS_REQ : get_folder_items_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->get_folder_items_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg1,pEvent->avrcpCtrlEvent.arg4,pEvent->avrcpCtrlEvent.arg5,
                    pEvent->avrcpCtrlEvent.arg2,pEvent->avrcpCtrlEvent.buf_ptr32))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
        case AVRCP_CTRL_GET_ITEM_ATTRIBUTES_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_GET_ITEM_ATTRIBUTES_REQ : get_item_attributes_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->get_item_attributes_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg1,pEvent->avrcpCtrlEvent.arg6,pEvent->avrcpCtrlEvent.arg3,
                    pEvent->avrcpCtrlEvent.arg2,(btrc_media_attr_t*)pEvent->avrcpCtrlEvent.buf_ptr32))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;
        case AVRCP_CTRL_PLAY_ITEMS_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_PLAY_ITEMS_REQ : play_item_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->play_item_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg1,(uint8_t*)&pEvent->avrcpCtrlEvent.arg6,pEvent->avrcpCtrlEvent.arg3))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;

        case AVRCP_CTRL_ADDTO_NOW_PLAYING_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_ADDTO_NOW_PLAYING_REQ : addto_now_playing_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->addto_now_playing_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg1,pEvent->avrcpCtrlEvent.arg6,pEvent->avrcpCtrlEvent.arg3))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;

        case AVRCP_CTRL_SEARCH_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_SEARCH_REQ : search_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->search_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                    pEvent->avrcpCtrlEvent.arg3,pEvent->avrcpCtrlEvent.buf_ptr))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;


        case AVRCP_CTRL_REG_NOTIFICATION_REQ:
        iter = FindAvDeviceByAddr(pA2dpSink->pA2dpDeviceList, pEvent->avrcpCtrlEvent.bd_addr);
        if (iter != pA2dpSink->pA2dpDeviceList.end() && (iter->mAvrcpConnected == true))
        {
            ALOGD(LOGTAG_CTRL "AVRCP_CTRL_REG_NOTIFICATION_REQ : register_notification_command_vendor called!~");
            if (sBtAvrcpCtrlVendorInterface != NULL) {
                if(BT_STATUS_SUCCESS == sBtAvrcpCtrlVendorInterface->register_notification_command_vendor(&pEvent->avrcpCtrlEvent.bd_addr,
                                        pEvent->avrcpCtrlEvent.arg1,0))
                    fprintf(stdout, "--> command has been successfully sent.\n" );
                else
                    fprintf(stdout, "error: command not be accepted!.\n" );
            }
        }
        else
        {
            ALOGD(LOGTAG_CTRL " Avrcp not connected or AV not connected");
        }
        break;

    }
}

void Avrcp::HandleEnableAvrcp(void) {
    BtEvent *pEvent = new BtEvent;
    ALOGD(LOGTAG_CTRL " HandleEnableAvrcp ");

    max_avrcp_conn = config_get_int (config,
            CONFIG_DEFAULT_SECTION, "BtMaxA2dpConn", 1);

    if (bluetooth_interface != NULL)
    {
        // AVRCP CT Initialization
        sBtAvrcpCtrlInterface = (btrc_ctrl_interface_t *)bluetooth_interface->
                get_profile_interface(BT_PROFILE_AV_RC_CTRL_ID);

        // AVRCP CT Vendor Initialization
        sBtAvrcpCtrlVendorInterface = (btrc_ctrl_vendor_interface_t *)bluetooth_interface->
                get_profile_interface(BT_PROFILE_AV_RC_CTRL_VENDOR_ID);

        if (sBtAvrcpCtrlInterface == NULL || sBtAvrcpCtrlVendorInterface == NULL)
        {
             pEvent->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
             pEvent->profile_start_event.profile_id = PROFILE_ID_AVRCP;
             pEvent->profile_start_event.status = false;
             PostMessage(THREAD_ID_GAP, pEvent);
             return;
        }

        if (sBtAvrcpCtrlInterface != NULL) {
            sBtAvrcpCtrlInterface->init(&sBluetoothAvrcpCtrlCallbacks);
        }

        if (sBtAvrcpCtrlVendorInterface != NULL) {
            sBtAvrcpCtrlVendorInterface->
                    init_vendor(&sBluetoothAvrcpCtrlVendorCallbacks, max_avrcp_conn);
        }

        pEvent->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
        pEvent->profile_start_event.profile_id = PROFILE_ID_AVRCP;
        pEvent->profile_start_event.status = true;

        PostMessage(THREAD_ID_GAP, pEvent);
    }
}

void Avrcp::HandleDisableAvrcp(void) {
    ALOGD(LOGTAG_CTRL " HandleDisableAvrcp ");

   if (sBtAvrcpCtrlInterface != NULL) {
       sBtAvrcpCtrlInterface->cleanup();
       sBtAvrcpCtrlInterface = NULL;
   }
   if (sBtAvrcpCtrlVendorInterface != NULL) {
       sBtAvrcpCtrlVendorInterface->cleanup_vendor();
       sBtAvrcpCtrlVendorInterface = NULL;
   }
   BtEvent *pEvent = new BtEvent;
   pEvent->profile_stop_event.event_id = PROFILE_EVENT_STOP_DONE;
       pEvent->profile_stop_event.profile_id = PROFILE_ID_AVRCP;
       pEvent->profile_stop_event.status = true;
       PostMessage(THREAD_ID_GAP, pEvent);
}

char* Avrcp::dump_message(BluetoothEventId event_id) {
    switch(event_id) {
    case AVRCP_CTRL_CONNECTED_CB:
        return "AVRCP_CTRL_CONNECTED_CB";
    case AVRCP_CTRL_DISCONNECTED_CB:
        return "AVRCP_CTRL_DISCONNECTED_CB";
    case AVRCP_CTRL_PASS_THRU_CMD_REQ:
        return "PASS_THRU_CMD_REQ";
    }
    return "UNKNOWN";
}

Avrcp :: Avrcp(const bt_interface_t *bt_interface, config_t *config) {
    this->bluetooth_interface = bt_interface;
    this->config = config;
    sBtAvrcpCtrlInterface = NULL;
    max_avrcp_conn = 0;
    memset(&mConnectedAvrcpDevice, 0, sizeof(bt_bdaddr_t));
    pthread_mutex_init(&this->lock, NULL);
    mPreviousPercentageVol = -1;
    mFirstAbsVolCmdRecvd = false;
}

Avrcp :: ~Avrcp() {
    pthread_mutex_destroy(&lock);
    rc_only_devices.clear();
}

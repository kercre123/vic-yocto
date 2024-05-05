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
#include "Gatt.hpp"
#include <iostream>
#include <hash_map>


#define LOGTAG "GATT "
#define UNUSED
#define MAX_NUM_HANDLES     (1)

using namespace std;
using std::list;
using std::string;
Gatt *g_gatt = NULL;

static const bt_bdaddr_t bd_addr_null={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#ifdef __cplusplus
extern "C"
{
#endif



void btgattc_register_app_cb(int status, int clientIf, bt_uuid_t *app_uuid)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, clientIf);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_REGISTER_APP_EVENT;
    event->gattc_register_app_event.status = status;
    event->gattc_register_app_event.clientIf = clientIf;
    event->gattc_register_app_event.app_uuid = (bt_uuid_t *) osi_malloc (sizeof(bt_uuid_t));;
    memcpy(event->gattc_register_app_event.app_uuid, app_uuid, sizeof(bt_uuid_t));

    PostMessage(THREAD_ID_GATT, event);
}

void btgattc_scan_result_cb(bt_bdaddr_t* bda, int rssi, uint8_t* adv_data)
{
    char c_address[32];
    snprintf(c_address,sizeof(c_address), "%02X:%02X:%02X:%02X:%02X:%02X",
            bda->address[0], bda->address[1], bda->address[2],
            bda->address[3], bda->address[4], bda->address[5]);

    ALOGD(LOGTAG "(%s) rssi (%d) bda (%s)\n",__FUNCTION__, rssi,c_address);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_SCAN_RESULT_EVENT;
    event->gattc_scan_result_event.bda = bda;
    event->gattc_scan_result_event.rssi= rssi;
    event->gattc_scan_result_event.adv_data= adv_data;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_open_cb(int conn_id, int status, int clientIf, bt_bdaddr_t* bda)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, clientIf);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_OPEN_EVENT;
    event->gattc_open_event.status= status;
    event->gattc_open_event.clientIf= clientIf;
    event->gattc_open_event.conn_id= conn_id;
    event->gattc_open_event.bda = bda;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_close_cb(int conn_id, int status, int clientIf, bt_bdaddr_t* bda)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, clientIf);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_CLOSE_EVENT;
    event->gattc_close_event.status= status;
    event->gattc_close_event.clientIf= clientIf;
    event->gattc_close_event.conn_id= conn_id;
    event->gattc_close_event.bda= bda;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_search_complete_cb(int conn_id, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) conn_id (%d)",__FUNCTION__,status, conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_SEARCH_COMPLETE_EVENT;
    event->gattc_search_complete_event.status= status;
    event->gattc_search_complete_event.conn_id= conn_id;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_register_for_notification_cb(int conn_id, int registered,
                                                    int status, uint16_t handle)
{
    ALOGD(LOGTAG "(%s) status (%d) conn_id (%d)",__FUNCTION__,status, conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_REGISTER_FOR_NOTIFICATION_EVENT;
    event->gattc_register_for_notification_event.status= status;
    event->gattc_register_for_notification_event.conn_id= conn_id;
    event->gattc_register_for_notification_event.registered= registered;
    event->gattc_register_for_notification_event.handle= handle;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_notify_cb(int conn_id, btgatt_notify_params_t *p_data)
{
    ALOGD(LOGTAG "(%s) conn_id (%d)",__FUNCTION__,conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_NOTIFY_EVENT;
    event->gattc_notify_event.conn_id= conn_id;
    event->gattc_notify_event.p_data= p_data;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_read_characteristic_cb(int conn_id, int status,
    btgatt_read_params_t *p_data)
{
    ALOGD(LOGTAG "(%s) status (%d) conn_id (%d)",__FUNCTION__,status, conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_READ_CHARACTERISTIC_EVENT;
    event->gattc_read_characteristic_event.status= status;
    event->gattc_read_characteristic_event.conn_id= conn_id;
    event->gattc_read_characteristic_event.p_data= p_data;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_write_characteristic_cb(int conn_id, int status,
    uint16_t handle)
{
    ALOGD(LOGTAG "(%s) status (%d) conn_id (%d)",__FUNCTION__,status, conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_WRITE_CHARACTERISTIC_EVENT;
    event->gattc_write_characteristic_event.status= status;
    event->gattc_write_characteristic_event.conn_id= conn_id;
    event->gattc_write_characteristic_event.handle= handle;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_execute_write_cb(int conn_id, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) conn_id (%d)",__FUNCTION__,status, conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_EXECUTE_WRITE_EVENT;
    event->gattc_execute_write_event.status= status;
    event->gattc_execute_write_event.conn_id= conn_id;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_read_descriptor_cb(int conn_id, int status, btgatt_read_params_t *p_data)
{
    ALOGD(LOGTAG "(%s) status (%d) conn_id (%d)",__FUNCTION__,status, conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_READ_DESCRIPTOR_EVENT;
    event->gattc_read_descriptor_event.status= status;
    event->gattc_read_descriptor_event.conn_id= conn_id;
    event->gattc_read_descriptor_event.p_data= p_data;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_write_descriptor_cb(int conn_id, int status, uint16_t handle)
{
    ALOGD(LOGTAG "(%s) status (%d) conn_id (%d)",__FUNCTION__,status, conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_WRITE_DESCRIPTOR_EVENT;
    event->gattc_write_descriptor_event.status= status;
    event->gattc_write_descriptor_event.conn_id= conn_id;
    event->gattc_write_descriptor_event.handle= handle;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_remote_rssi_cb(int client_if,bt_bdaddr_t* bda, int rssi, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_REMOTE_RSSI_EVENT;
    event->gattc_remote_rssi_event.status = status;
    event->gattc_remote_rssi_event.client_if= client_if;
        event->gattc_remote_rssi_event.rssi= rssi;
        event->gattc_remote_rssi_event.bda= bda;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_advertise_cb(int status, int client_if)
{
    ALOGD(LOGTAG "(%s): status (%d) client_if (%d)\n",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_ADVERTISE_EVENT;
    event->gattc_advertise_event.status= status;
    event->gattc_advertise_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_configure_mtu_cb(int conn_id, int status, int mtu)
{
    ALOGD(LOGTAG "(%s) status (%d) conn_id (%d)",__FUNCTION__,status, conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_CONFIGURE_MTU_EVENT;
    event->gattc_configure_mtu_event.status= status;
    event->gattc_configure_mtu_event.conn_id= conn_id;
    event->gattc_configure_mtu_event.mtu= mtu;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_scan_filter_cfg_cb(int action, int client_if, int status, int filt_type, int avbl_space)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_SCAN_FILTER_CFG_EVENT;
    event->gattc_scan_filter_cfg_event.status= status;
    event->gattc_scan_filter_cfg_event.action= action;
    event->gattc_scan_filter_cfg_event.client_if= client_if;
    event->gattc_scan_filter_cfg_event.filt_type=filt_type;
    event->gattc_scan_filter_cfg_event.avbl_space=avbl_space;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_scan_filter_param_cb(int action, int client_if, int status, int avbl_space)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_SCAN_FILTER_PARAM_EVENT;
    event->gattc_scan_filter_param_event.status= status;
    event->gattc_scan_filter_param_event.action= action;
    event->gattc_scan_filter_param_event.client_if= client_if;
    event->gattc_scan_filter_param_event.avbl_space=avbl_space;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_scan_filter_status_cb(int action, int client_if, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_SCAN_FILTER_STATUS_EVENT;
    event->gattc_scan_filter_status_event.status= status;
    event->gattc_scan_filter_status_event.action= action;
    event->gattc_scan_filter_status_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_multiadv_enable_cb(int client_if, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_MULTIADV_ENABLE_EVENT;
    event->gattc_multiadv_enable_event.status= status;
    event->gattc_multiadv_enable_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_multiadv_update_cb(int client_if, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_MULTIADV_UPDATE_EVENT;
    event->gattc_multiadv_update_event.status= status;
    event->gattc_multiadv_update_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_multiadv_setadv_data_cb(int client_if, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_MULTIADV_SETADV_DATA_EVENT;
    event->gattc_multiadv_setadv_data_event.status= status;
    event->gattc_multiadv_setadv_data_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_multiadv_disable_cb(int client_if, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_MULTIADV_DISABLE_EVENT;
    event->gattc_multiadv_disable_event.status= status;
    event->gattc_multiadv_disable_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_congestion_cb(int conn_id, bool congested)
{

 ALOGD(LOGTAG "(%s) conn_id (%d) congested (%d)",__FUNCTION__,conn_id, congested);

 BtEvent *event = new BtEvent;
 CHECK_PARAM_VOID(event)

 event->event_id = BTGATTC_CONGESTION_EVENT;
 event->gattc_congestion_event.conn_id= conn_id;
 event->gattc_congestion_event.congested= congested;

 PostMessage(THREAD_ID_GATT, event);

}

void btgattc_batchscan_cfg_storage_cb(int client_if, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_BATCHSCAN_CFG_STORAGE_EVENT;
    event->gattc_batchscan_cfg_storage_event.status= status;
    event->gattc_batchscan_cfg_storage_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);
}
void btgattc_batchscan_startstop_cb(int startstop_action, int client_if, int status)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_BATCHSCAN_STARTSTOP_EVENT;
    event->gattc_batchscan_startstop_event.status= status;
    event->gattc_batchscan_startstop_event.startstop_action= startstop_action;
    event->gattc_batchscan_startstop_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);


}

void btgattc_batchscan_reports_cb(int client_if, int status, int report_format,
                                            int num_records, int data_len, uint8_t *p_rep_data)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_BATCHSCAN_REPORTS_EVENT;
    event->gattc_batchscan_reports_event.status= status;
    event->gattc_batchscan_reports_event.client_if= client_if;
    event->gattc_batchscan_reports_event.report_format = report_format;
    event->gattc_batchscan_reports_event.num_records= num_records;
    event->gattc_batchscan_reports_event.data_len= data_len;
    event->gattc_batchscan_reports_event.p_rep_data= p_rep_data;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_batchscan_threshold_cb(int client_if)
{
    ALOGD(LOGTAG "(%s) client_if (%d)",__FUNCTION__,client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_BATCHSCAN_THRESHOLD_EVENT;
    event->gattc_batchscan_threshold_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_track_adv_event_cb(btgatt_track_adv_info_t *p_adv_track_info)
{
    ALOGD(LOGTAG "(%s) ",__FUNCTION__);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_TRACK_ADV_EVENT_EVENT;
    event->gattc_track_adv_event_event.p_adv_track_info= p_adv_track_info;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_scan_parameter_setup_completed_cb(int client_if, btgattc_error_t status)
{
    ALOGD(LOGTAG "(%s) status (%d) client_if (%d)",__FUNCTION__,status, client_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_SCAN_PARAMETER_SETUP_COMPLETED_EVENT;
    event->gattc_scan_parameter_setup_completed_event.status= status;
    event->gattc_scan_parameter_setup_completed_event.client_if= client_if;

    PostMessage(THREAD_ID_GATT, event);

}

void btgattc_get_gatt_db_cb(int conn_id, btgatt_db_element_t *db, int count)
{
    ALOGD(LOGTAG "(%s) conn_id (%d)",__FUNCTION__,conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTC_GET_GATT_DB_EVENT;
    event->gattc_get_gatt_db_event.conn_id = conn_id;
    event->gattc_get_gatt_db_event.db = db;
    event->gattc_get_gatt_db_event.count = count;

    PostMessage(THREAD_ID_GATT, event);

}
static const btgatt_client_callbacks_t sGattClientCallbacks = {
    btgattc_register_app_cb,
    btgattc_scan_result_cb,
    btgattc_open_cb,
    btgattc_close_cb,
    btgattc_search_complete_cb,
    btgattc_register_for_notification_cb,
    btgattc_notify_cb,
    btgattc_read_characteristic_cb,
    btgattc_write_characteristic_cb,
    btgattc_read_descriptor_cb,
    btgattc_write_descriptor_cb,
    btgattc_execute_write_cb,
    btgattc_remote_rssi_cb,
    btgattc_advertise_cb,
    btgattc_configure_mtu_cb,
    btgattc_scan_filter_cfg_cb,
    btgattc_scan_filter_param_cb,
    btgattc_scan_filter_status_cb,
    btgattc_multiadv_enable_cb,
    btgattc_multiadv_update_cb,
    btgattc_multiadv_setadv_data_cb,
    btgattc_multiadv_disable_cb,
    btgattc_congestion_cb,
    btgattc_batchscan_cfg_storage_cb,
    btgattc_batchscan_startstop_cb,
    btgattc_batchscan_reports_cb,
    btgattc_batchscan_threshold_cb,
    btgattc_track_adv_event_cb,
    btgattc_scan_parameter_setup_completed_cb,
    btgattc_get_gatt_db_cb,
    NULL, /* services_removed_cb */
    NULL /* services_added_cb */
};

/**
 * GATT Server Callback Implementation
 */
void btgatts_register_app_cb(int status, int server_if, bt_uuid_t *uuid)
{
    ALOGD(LOGTAG "\n (%s) status (%d) server_if (%d) \n",__FUNCTION__, status, server_if);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)
    event->event_id = BTGATTS_REGISTER_APP_EVENT;
    event->gatts_register_app_event.status = status;
    event->gatts_register_app_event.server_if = server_if;
    event->gatts_register_app_event.uuid = (bt_uuid_t *) osi_malloc (sizeof(bt_uuid_t));
    memcpy(event->gatts_register_app_event.uuid, uuid, sizeof(bt_uuid_t));
    
    PostMessage(THREAD_ID_GATT, event);
    ALOGD(LOGTAG "exiting btgatts_register_app_cb \n");
}

void btgatts_connection_cb(int conn_id, int server_if, int connected, bt_bdaddr_t *bda)
{

    char c_address[32];
    snprintf(c_address,sizeof(c_address), "%02X:%02X:%02X:%02X:%02X:%02X",
            bda->address[0], bda->address[1], bda->address[2],
            bda->address[3], bda->address[4], bda->address[5]);

    ALOGD(LOGTAG "(%s) connid (%d) server_if (%d) status (%d) bda (%s)\n",__FUNCTION__, conn_id,
            server_if, connected, c_address);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_CONNECTION_EVENT;
    event->gatts_connection_event.conn_id = conn_id;
    event->gatts_connection_event.server_if = server_if;
    event->gatts_connection_event.connected = connected;
    event->gatts_connection_event.bda = bda;
    PostMessage(THREAD_ID_GATT, event);

}

void btgatts_service_added_cb(int status, int server_if,
                              btgatt_srvc_id_t *srvc_id, int srvc_handle)
{
    ALOGD(LOGTAG "(%s) status (%d) server_if (%d), srvc_handle(%d)) \n",__FUNCTION__,
                status, server_if, srvc_handle);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_SERVICE_ADDED_EVENT;
    event->gatts_service_added_event.status = status;
    event->gatts_service_added_event.server_if = server_if;
    event->gatts_service_added_event.srvc_id = srvc_id ;
    event->gatts_service_added_event.srvc_handle = srvc_handle ;

    PostMessage(THREAD_ID_GATT, event);

}

void btgatts_included_service_added_cb(int status, int server_if, int srvc_handle,
        int incl_srvc_handle)
{
    ALOGD(LOGTAG "(%s) Status:(%d) \n",__FUNCTION__, status);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_INCLUDED_SERVICE_ADDED_EVENT;
    event->gatts_included_service_added_event.status = status;
    event->gatts_included_service_added_event.server_if = server_if;
    event->gatts_included_service_added_event.srvc_handle = srvc_handle ;
    event->gatts_included_service_added_event.incl_srvc_handle = incl_srvc_handle ;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_characteristic_added_cb(int status, int server_if, bt_uuid_t *char_id,
                                                int srvc_handle, int char_handle)
{
    ALOGD(LOGTAG "(%s) Status:(%d) \n",__FUNCTION__, status);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_CHARACTERISTIC_ADDED_EVENT;
    event->gatts_characteristic_added_event.status = status;
    event->gatts_characteristic_added_event.server_if = server_if;
    event->gatts_characteristic_added_event.char_id = (bt_uuid_t *) osi_malloc (sizeof(bt_uuid_t));;
    memcpy(event->gatts_characteristic_added_event.char_id, char_id, sizeof(bt_uuid_t));
    event->gatts_characteristic_added_event.srvc_handle = srvc_handle ;
    event->gatts_characteristic_added_event.char_handle = char_handle ;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_descriptor_added_cb(int status, int server_if, bt_uuid_t *descr_id,
                                            int srvc_handle, int descr_handle)
{
    ALOGD(LOGTAG "(%s) Status:(%d) \n",__FUNCTION__, status);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_DESCRIPTOR_ADDED_EVENT;
    event->gatts_descriptor_added_event.status = status;
    event->gatts_descriptor_added_event.server_if = server_if;
    event->gatts_descriptor_added_event.descr_id = (bt_uuid_t *) osi_malloc (sizeof(bt_uuid_t));;
    memcpy(event->gatts_descriptor_added_event.descr_id, descr_id, sizeof(bt_uuid_t));
    event->gatts_descriptor_added_event.srvc_handle = srvc_handle ;
    event->gatts_descriptor_added_event.descr_handle = descr_handle ;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_service_started_cb(int status, int server_if, int srvc_handle)
{
    ALOGD(LOGTAG "(%s) Status:(%d) \n",__FUNCTION__, status);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_SERVICE_STARTED_EVENT;
    event->gatts_service_started_event.status = status;
    event->gatts_service_started_event.server_if = server_if;
    event->gatts_service_started_event.srvc_handle = srvc_handle ;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_service_stopped_cb(int status, int server_if, int srvc_handle)
{
    ALOGD(LOGTAG "(%s) Status:(%d)",__FUNCTION__, status);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_SERVICE_STOPPED_EVENT;
    event->gatts_service_stopped_event.status = status;
    event->gatts_service_stopped_event.server_if = server_if;
    event->gatts_service_stopped_event.srvc_handle = srvc_handle ;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_service_deleted_cb(int status, int server_if, int srvc_handle)
{
    ALOGD(LOGTAG "(%s) Status:(%d)",__FUNCTION__, status);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_SERVICE_DELETED_EVENT;
    event->gatts_service_deleted_event.status = status;
    event->gatts_service_deleted_event.server_if = server_if;
    event->gatts_service_deleted_event.srvc_handle = srvc_handle ;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_request_read_cb(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle,
                                        int offset, bool is_long)
{
    char c_address[32];
    snprintf(c_address,sizeof(c_address), "%02X:%02X:%02X:%02X:%02X:%02X",
            bda->address[0], bda->address[1], bda->address[2],
            bda->address[3], bda->address[4], bda->address[5]);

    ALOGD(LOGTAG "(%s) connid:(%d) bda (%s)",__FUNCTION__, conn_id, c_address);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_REQUEST_READ_EVENT;
    event->gatts_request_read_event.conn_id = conn_id;
    event->gatts_request_read_event.trans_id = trans_id;
    event->gatts_request_read_event.bda = bda;
    event->gatts_request_read_event.attr_handle = attr_handle;
    event->gatts_request_read_event.offset = offset;
    event->gatts_request_read_event.is_long = is_long ;

    PostMessage(THREAD_ID_GATT, event);

}

void btgatts_request_write_cb(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle,
                                        int offset, int length, bool need_rsp, bool is_prep,
                                        uint8_t* value)
{
    char c_address[32];
    uint8_t *mem = NULL;
    snprintf(c_address,sizeof(c_address), "%02X:%02X:%02X:%02X:%02X:%02X",
            bda->address[0], bda->address[1], bda->address[2],
           bda->address[3], bda->address[4], bda->address[5]);

    ALOGD(LOGTAG "(%s) connid:(%d) bdaddr:(%s) value (%s) need_rsp (%d)\n",__FUNCTION__, conn_id,
            c_address, value, need_rsp);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_REQUEST_WRITE_EVENT;
    event->gatts_request_write_event.conn_id = conn_id;
    event->gatts_request_write_event.trans_id = trans_id;
    event->gatts_request_write_event.bda = bda;
    event->gatts_request_write_event.attr_handle = attr_handle;
    event->gatts_request_write_event.offset = offset;
    event->gatts_request_write_event.length = length;
    event->gatts_request_write_event.need_rsp = need_rsp;
    event->gatts_request_write_event.is_prep = is_prep;
    if(length > 0 && value != NULL)
    {
        mem = (uint8_t *)osi_malloc(length);
        if(mem != NULL)
            memcpy(mem,value,length);
    }
    event->gatts_request_write_event.value = mem;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_request_exec_write_cb(int conn_id, int trans_id,
                                                bt_bdaddr_t *bda, int exec_write)
{

    char c_address[32];
    snprintf(c_address,sizeof(c_address), "%02X:%02X:%02X:%02X:%02X:%02X",
            bda->address[0], bda->address[1], bda->address[2],
           bda->address[3], bda->address[4], bda->address[5]);

    ALOGD(LOGTAG "(%s) connid:(%d) bda:(%s)",__FUNCTION__, conn_id, c_address);
    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_REQUEST_EXEC_WRITE_EVENT;
    event->gatts_request_exec_write_event.conn_id = conn_id;
    event->gatts_request_exec_write_event.trans_id = trans_id;
    event->gatts_request_exec_write_event.bda = bda;
    event->gatts_request_exec_write_event.exec_write = exec_write;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_response_confirmation_cb(int status, int handle)
{
    ALOGD(LOGTAG "(%s) status:(%d) handle:(%d)",__FUNCTION__, status, handle);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_RESPONSE_CONFIRMATION_EVENT;
    event->gatts_response_confirmation_event.status = status;
    event->gatts_response_confirmation_event.handle = handle ;

    PostMessage(THREAD_ID_GATT, event);

}

void btgatts_indication_sent_cb(int conn_id, int status)
{
    ALOGD(LOGTAG "(%s) conn_id (%d) status:(%d)",__FUNCTION__, conn_id, status);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_INDICATION_SENT_EVENT;
    event->gatts_indication_sent_event.status = status;
    event->gatts_indication_sent_event.conn_id = conn_id ;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_congestion_cb(int conn_id, bool congested)
{
    ALOGD(LOGTAG "(%s) contested (%d) conn_id:(%d)",__FUNCTION__, congested,conn_id);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_CONGESTION_EVENT;
    event->gatts_congestion_event.congested = congested;
    event->gatts_congestion_event.conn_id = conn_id ;

    PostMessage(THREAD_ID_GATT, event);
}

void btgatts_mtu_changed_cb(int conn_id, int mtu)
{
    ALOGD(LOGTAG "(%s) conn_id:(%d) Mtu (%d)",__FUNCTION__, conn_id, mtu);

    BtEvent *event = new BtEvent;
    CHECK_PARAM_VOID(event)

    event->event_id = BTGATTS_MTU_CHANGED_EVENT;
    event->gatts_mtu_changed_event.conn_id = conn_id;
    event->gatts_mtu_changed_event.mtu = mtu ;

    PostMessage(THREAD_ID_GATT, event);

}

static const btgatt_server_callbacks_t sGattServerCallbacks = {
    btgatts_register_app_cb,
    btgatts_connection_cb,
    btgatts_service_added_cb,
    btgatts_included_service_added_cb,
    btgatts_characteristic_added_cb,
    btgatts_descriptor_added_cb,
    btgatts_service_started_cb,
    btgatts_service_stopped_cb,
    btgatts_service_deleted_cb,
    btgatts_request_read_cb,
    btgatts_request_write_cb,
    btgatts_request_exec_write_cb,
    btgatts_response_confirmation_cb,
    btgatts_indication_sent_cb,
    btgatts_congestion_cb,
    btgatts_mtu_changed_cb
};
/**
 * GATT callbacks
 */
static btgatt_callbacks_t sGattCallbacks = {
    sizeof(btgatt_callbacks_t),
    &sGattClientCallbacks,
    &sGattServerCallbacks
};

void BtGattMsgHandler(void *msg) {
    BtEvent* event = NULL;
    bool status = false;
    if(!msg)
    {
        ALOGD(LOGTAG "(%s) Msg is null, return.\n",__FUNCTION__);
        return;
    }
    event = ( BtEvent *) msg;
    ALOGD(LOGTAG "(%s) event id (%d) \n",__FUNCTION__, (int) event->event_id);
    switch(event->event_id) {
        case PROFILE_API_START:
            if (g_gatt) {
                status = g_gatt->HandleEnableGatt(); /*g_gatt->HandleEnableGatt();*/
                BtEvent *start_event = new BtEvent;
                CHECK_PARAM_VOID (start_event)

                ALOGD(LOGTAG "(%s) posting profile start done\n",__FUNCTION__);

                start_event->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
                start_event->profile_start_event.profile_id = PROFILE_ID_GATT;
                start_event->profile_start_event.status = status;
                PostMessage(THREAD_ID_GAP, start_event);
            }
            break;
        case PROFILE_API_STOP:
            if (g_gatt) {
                status = g_gatt->HandleDisableGatt();
                BtEvent *stop_event = new BtEvent;
                CHECK_PARAM_VOID (stop_event)

                stop_event->profile_start_event.event_id = PROFILE_EVENT_STOP_DONE;
                stop_event->profile_start_event.profile_id = PROFILE_ID_GATT;
                stop_event->profile_start_event.status = status;
                PostMessage(THREAD_ID_GAP, stop_event);
            }
            break;
        default:

            if(g_gatt) {
                ALOGD(LOGTAG "(%s) Received Message %d",__FUNCTION__, (int) event->event_id);
                g_gatt->ProcessEvent(( BtEvent *) msg);
            }
            break;
    }
    delete event;
}
#ifdef __cplusplus
}
#endif

void Gatt::HandleGattIpcMsg(BtIpcMsg *ipc_msg)
{

    CHECK_PARAM_VOID(ipc_msg)
}

void Gatt::HandleGattsRegisterAppEvent(GattsRegisterAppEvent *event)
{
    ALOGD(LOGTAG  "(%s) server_if =%d status =%d uuid =%x \n ",__FUNCTION__, event->server_if,
            event->status, event->uuid->uu);

    int itr;
    std::map<uint8_t *,BluetoothGattServerCallback *> ::iterator it;

    for (it = serverCbUuidMap.begin(); it != serverCbUuidMap.end(); ++it) {
        if (it->first &&  event->uuid->uu) {
           ALOGD(LOGTAG "checking \n");
           itr = 0;
           for (itr = 0; itr < 16; itr++) {
               ALOGD(LOGTAG " it->first is %d, uuid is %d \n",it->first[itr], event->uuid->uu[itr]);
               if ( it->first[itr] == event->uuid->uu[itr]) {
                  continue;
               } else {
                 break;
               }
           }
           if ( itr == 16) {
              fprintf (stdout,"App found with this UUID \n");
              serverCbSifMap.insert(std::make_pair(event->server_if, it->second));
              break;
           } else {
              fprintf (stdout,"No app found with this UUUD\n");
           }
        }
     }

     if (it->second) {
        it->second->gattServerRegisterAppCb(event->status,event->server_if,event->uuid);
     } else {
        ALOGD(LOGTAG "Callback is null \n");
     }
}

void Gatt::HandleGattsConnectionEvent(GattsConnectionEvent *event)
{
    ALOGD(LOGTAG "(%s) conn_id (%d) server_if (%d) connected (%d)\n",__FUNCTION__, event->conn_id,
            event->server_if, event->connected);

    ConnidServerifMap.insert(std::make_pair(event->conn_id, event->server_if));

    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(event->server_if);
    if (it != serverCbSifMap.end()) {
       it->second->btgatts_connection_cb(event->conn_id,event->server_if,event->connected, event->bda);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}

void Gatt::HandleGattsServiceAddedEvent(GattsServiceAddedEvent *event)
{
    ALOGD(LOGTAG  "(%s) event_id =%d status =%d server_if =%d,service_handle =%d \n",__FUNCTION__,
           event->event_id, event->status, event->server_if,event->srvc_handle);

    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(event->server_if);
    if (it != serverCbSifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgatts_service_added_cb(event->status,event->server_if,event->srvc_id, event->srvc_handle);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}

void Gatt::HandleGattsCharacteristicAddedEvent(GattsCharacteristicAddedEvent *event)
{
    ALOGD(LOGTAG  "(%s) char_handle =%d char_id =%d status =%d server_if =%d,service_handle =%d \n",
            __FUNCTION__, event->char_handle, event->char_id, event->status,
            event->server_if, event->srvc_handle);


    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(event->server_if);
    if(it != serverCbSifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgatts_characteristic_added_cb(event->status,event->server_if,event->char_id, event->srvc_handle,event->char_handle);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}

void Gatt::HandleGattsDescriptorAddedEvent(GattsDescriptorAddedEvent *event)
{
    ALOGD(LOGTAG  "(%s) desc_handle =%d desc_id =%d status =%d server_if =%d, srvc_handle=%x,"
            "service_handle =%d \n ",__FUNCTION__, event->descr_handle, event->descr_id,
            event->status, event->server_if,event->srvc_handle);

    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(event->server_if);
    if(it != serverCbSifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgatts_descriptor_added_cb(event->status,event->server_if,event->descr_id, event->srvc_handle, event->descr_handle);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}
void Gatt::HandleGattsServiceStartedEvent(GattsServiceStartedEvent *event)
{
    ALOGD(LOGTAG "(%s) status =%d server_if =%d,service_handle =%d\n",__FUNCTION__,
            event->status, event->server_if, event->srvc_handle);

    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(event->server_if);
    if (it != serverCbSifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgatts_service_started_cb(event->status,event->server_if,event->srvc_handle);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}

void Gatt::HandleGattsServiceStoppedEvent(GattsServiceStoppedEvent *event)
{
    ALOGD(LOGTAG  "Handler :(%s) status(%d) server_if(%d) srvc_handle(%d)",__FUNCTION__,
            event->status, event->server_if, event->srvc_handle);

    std::map<int,BluetoothGattServerCallback *> ::iterator it;
    it = serverCbSifMap.find(event->server_if);
    if (it != serverCbSifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgatts_service_stopped_cb(event->status,event->server_if,event->srvc_handle);
    }  else {
       ALOGD(LOGTAG "Not found \n");
    }
}

void Gatt::HandleGattsServiceDeletedEvent(GattsServiceDeletedEvent *event)
{
    ALOGD(LOGTAG  "Handler :(%s) status(%d) server_if(%d) srvc_handle(%d)",__FUNCTION__,
            event->status, event->server_if, event->srvc_handle);

    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(event->server_if);
    if (it != serverCbSifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgatts_service_deleted_cb(event->status,event->server_if,event->srvc_handle);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}

// here we have to take decision based on connection id
void Gatt::HandleGattsRequestWriteEvent(GattsRequestWriteEvent *event)
{
    char c_address[32];
    snprintf(c_address,sizeof(c_address), "%02X:%02X:%02X:%02X:%02X:%02X",
                event->bda->address[0], event->bda->address[1], event->bda->address[2],
                event->bda->address[3], event->bda->address[4], event->bda->address[5]);

    ALOGD(LOGTAG "server if is %d \n", ConnidServerifMap[event->conn_id]);
    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(ConnidServerifMap[event->conn_id]);
    if (it != serverCbSifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgatts_request_write_cb(event->conn_id,event->trans_id, event->bda, event->attr_handle,
                                    event->offset, event->length, event->need_rsp, event->is_prep,
                                    event->value);
    } else {
        ALOGD(LOGTAG "Not found \n");
    }
}


void Gatt::HandleGattsRequestReadEvent(GattsRequestReadEvent *event)
{

    ALOGD(LOGTAG  "Handler :(%s) conn_id(%d) event_id(%d) attr_handle(%d)",__FUNCTION__,
            event->conn_id, event->event_id, event->attr_handle);

    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(ConnidServerifMap[event->conn_id]);
    if (it != serverCbSifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgatts_request_read_cb(event->conn_id, event->trans_id, event->bda, event->attr_handle,
                                       event->offset, event->is_long);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}

void Gatt::HandleGattsIncludeServiceAddedEvent(GattsIncludedServiceAddedEvent *event)
{

    ALOGD(LOGTAG  "(%s) event_id =%d server_if =%d,service_handle =%d \n",__FUNCTION__,
            event->event_id, event->server_if,event->srvc_handle);

    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(event->server_if);
    if (it != serverCbSifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgatts_included_service_added_cb(event->status,event->server_if,event->srvc_handle, event->incl_srvc_handle);
    } else {
        ALOGD(LOGTAG "Not found \n");
    }
}


void Gatt::HandleGattsRequestExecWriteEvent(GattsRequestExecWriteEvent *event)
{

    ALOGD(LOGTAG  "Handler :(%s) conn_id(%d) event_id(%d) exec_write(%d)",__FUNCTION__,
            event->conn_id, event->event_id, event->exec_write);

    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(ConnidServerifMap[event->conn_id]);
    if (it != serverCbSifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgatts_request_exec_write_cb(event->conn_id, event->trans_id,
                                               event->bda, event->exec_write);
    } else {
        ALOGD(LOGTAG "Not found \n");
    }
}


void Gatt::HandleGattsResponseConfirmationEvent(GattsResponseConfirmationEvent *event)
{
    UNUSED
}

void Gatt::HandleGattsIndicationSentEvent(GattsIndicationSentEvent *event)
{

    ALOGD(LOGTAG  "Handler :(%s) conn_id(%d) event_id(%d) status(%d)",__FUNCTION__,
                event->conn_id, event->event_id, event->status);

    std::map<int,BluetoothGattServerCallback *> ::iterator it;

    it = serverCbSifMap.find(ConnidServerifMap[event->conn_id]);
    if (it != serverCbSifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgatts_indication_sent_cb(event->conn_id, event->status);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}


void Gatt::HandleGattsCongetionEvent(GattsCongestionEvent *event)
{
        ALOGD(LOGTAG  "Handler :(%s) conn_id(%d) event_id(%d) congested(%d)",__FUNCTION__,
                        event->conn_id, event->event_id, event->congested);

        std::map<int,BluetoothGattServerCallback *> ::iterator it;

        it = serverCbSifMap.find(ConnidServerifMap[event->conn_id]);
        if(it != serverCbSifMap.end()) {
         ALOGD(LOGTAG "found \n");
         it->second->btgatts_congestion_cb(event->conn_id, event->congested);
        }  else {
         ALOGD(LOGTAG "Not found \n");
        }
}

void Gatt::HandleGattsMtuChangedEvent(GattsMTUchangedEvent *event)
{
        ALOGD(LOGTAG  "Handler :(%s) conn_id(%d) event_id(%d) mtu(%d)",__FUNCTION__,
                        event->conn_id, event->event_id, event->mtu);

        std::map<int,BluetoothGattServerCallback *> ::iterator it;

        it = serverCbSifMap.find(ConnidServerifMap[event->conn_id]);
        if(it != serverCbSifMap.end()) {
         ALOGD(LOGTAG "found \n");
         it->second->btgatts_mtu_changed_cb(event->conn_id, event->mtu);
        } else {
         ALOGD(LOGTAG "Not found \n");
        }

}

void Gatt::HandleGattcRegisterAppEvent(GattcRegisterAppEvent *event)
{
    int itr;
    std::map<uint8_t *,BluetoothGattClientCallback *> ::iterator it;

    for (it = clientCbUuidMap.begin(); it != clientCbUuidMap.end(); ++it) {

         if (it->first &&  event->app_uuid->uu) {
            ALOGD(LOGTAG "checking \n");
            itr = 0;
            for (itr = 0; itr < 16; itr++) {
                ALOGD(LOGTAG " it->first is %d, uuid is %d \n",it->first[itr], event->app_uuid->uu[itr]);
                if ( it->first[itr] == event->app_uuid->uu[itr]) {
                   continue;
                } else {
                    break;
                }
            }
            if ( itr == 16) {
               fprintf (stdout,"found \n");
               clientCbCifMap.insert(std::make_pair(event->clientIf, it->second));
               break;
            } else {
               fprintf (stdout,"not found \n");
            }
        }
    }

    if (it->second) {
       it->second->btgattc_client_register_app_cb(event->status,event->clientIf,event->app_uuid);
    } else {
       ALOGD(LOGTAG "Callback is null \n");
    }

}


void Gatt::HandleGattcScanResultEvent (GattcScanResultEvent *event)
{
         std::map<int,int> ::iterator it;
        for ( it = clientIfScanStatusMap.begin(); it != clientIfScanStatusMap.end();it++) {
            if ( it->second == true) {
                 std::map<int,BluetoothGattClientCallback *> ::iterator it2;
                 it2 = clientCbCifMap.find(it->first);
                 if(it2 != clientCbCifMap.end()) {
                   ALOGD(LOGTAG "found \n");
                    if ( it2->second) {
                        it2->second->btgattc_scan_result_cb(event->bda,event->rssi,event->adv_data);
                    } else {
                        ALOGD(LOGTAG "Not found \n");
                    }
                 }
             }

        }
}

void Gatt::HandleGattcOpenEvent(GattcOpenEvent *event) {

    ALOGD(LOGTAG "(%s) conn_id (%d) clientif (%d) status (%d)\n",__FUNCTION__, event->conn_id,
            event->clientIf, event->status);

    ConnidClientifMap.insert(std::make_pair(event->conn_id, event->clientIf));

    std::map<int,BluetoothGattClientCallback *> ::iterator it;

    it = clientCbCifMap.find(event->clientIf);
    if(it != clientCbCifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgattc_open_cb(event->conn_id, event->status, event->clientIf, event->bda);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}

void Gatt::HandleGattcCloseEvent(GattcCloseEvent *event)  {

    ALOGD(LOGTAG "(%s) conn_id (%d) clientif (%d) status (%d)\n",__FUNCTION__, event->conn_id,
            event->clientIf, event->status);

    std::map<int,BluetoothGattClientCallback *> ::iterator it;

    it = clientCbCifMap.find(event->clientIf);
    if(it != clientCbCifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgattc_close_cb(event->conn_id, event->status, event->clientIf, event->bda);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}

void Gatt::HandleGattcSearchCompleteEvent(GattcSearchCompleteEvent *event)  {

    ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->conn_id,
            event->event_id, event->status);


    std::map<int,BluetoothGattClientCallback *> ::iterator it;

    it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
    if(it != clientCbCifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgattc_search_complete_cb(event->conn_id, event->status);
    } else {
       ALOGD(LOGTAG "Not found \n");
    }
}

void Gatt::HandleGattcRegisterForNotificationEvent(GattcRegisterForNotificationEvent *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->conn_id,
                    event->event_id, event->status);

     std::map<int,BluetoothGattClientCallback *> ::iterator it;

      it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
       if(it != clientCbCifMap.end()) {
          ALOGD(LOGTAG "found \n");
           it->second->btgattc_register_for_notification_cb(event->conn_id, event->registered,
                                      event->status, event->handle);
        } else {
           ALOGD(LOGTAG "Not found \n");
        }
}

void Gatt::HandleGattcNotifyEvent(GattcNotifyEvent *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d)\n",__FUNCTION__, event->conn_id,
                                event->event_id);

      std::map<int,BluetoothGattClientCallback *> ::iterator it;

      it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
      if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_notify_cb(event->conn_id, event->p_data);
      }  else {
        ALOGD(LOGTAG "Not found \n");
      }
}

void Gatt::HandleGattcReadCharacteristicEvent(GattcReadCharacteristicEvent *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->conn_id,
                                event->event_id, event->status);

     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_read_characteristic_cb(event->conn_id, event->status,
                                                    event->p_data);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcWriteCharacteristicEvent(GattcWriteCharacteristicEvent *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->conn_id,
                 event->event_id, event->status);

     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_write_characteristic_cb(event->conn_id, event->status,
                     event->handle);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }

}

void Gatt::HandleGattcReadDescriptorEvent(GattcReadDescriptorEvent *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->conn_id,
                    event->event_id, event->status);

     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_read_descriptor_cb(event->conn_id, event->status, event->p_data);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcWriteDescriptorEvent(GattcWriteDescriptorEvent  *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->conn_id,
                     event->event_id, event->status);

     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_write_descriptor_cb(event->conn_id, event->status, event->handle);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcExecuteWriteEvent(GattcExecuteWriteEvent *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->conn_id,
                                event->event_id, event->status);

      std::map<int,BluetoothGattClientCallback *> ::iterator it;

      it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
      if(it != clientCbCifMap.end()) {
         ALOGD(LOGTAG "found \n");
         it->second->btgattc_execute_write_cb(event->conn_id, event->status);
      } else {
         ALOGD(LOGTAG "Not found \n");
      }
}

void Gatt::HandleGattcRemoteRssiEvent(GattcRemoteRssiEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) rssi (%d)\n",__FUNCTION__, event->client_if,
                     event->event_id, event->rssi);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_remote_rssi_cb(event->client_if,event->bda, event->rssi, event->status);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcAdvertiseEvent(GattcAdvertiseEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                     event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgattc_advertise_cb(event->status, event->client_if);
     } else {
       ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcConfigureMtuEvent(GattcConfigureMtuEvent *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->conn_id,
                     event->event_id, event->status);

     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
     if(it != clientCbCifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgattc_configure_mtu_cb(event->conn_id, event->status, event->mtu);
     } else {
       ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcScanFilterCfgEvent(GattcScanFilterCfgEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                     event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_scan_filter_cfg_cb(event->action, event->client_if, event->status,
                    event->filt_type, event->avbl_space);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcScanFilterParamEvent(GattcScanFilterParamEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                    event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_scan_filter_param_cb(event->action, event->client_if, event->status, event->avbl_space);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcScanFilterStatusEvent(GattcScanFilterStatusEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                    event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_scan_filter_status_cb(event->action, event->client_if, event->status);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcMultiadvEnableEvent(GattcMultiadvEnableEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                     event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgattc_multiadv_enable_cb(event->client_if, event->status);
     } else {
       ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcMultiadvUpdateEvent(GattcMultiadvUpdateEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                    event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_multiadv_update_cb(event->client_if, event->status);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcMultiadvSetadvDataEvent(GattcMultiadvSetadvDataEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                     event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

      it = clientCbCifMap.find(event->client_if);
      if(it != clientCbCifMap.end()) {
         ALOGD(LOGTAG "found \n");
         it->second->btgattc_multiadv_setadv_data_cb(event->client_if, event->status);
      } else {
         ALOGD(LOGTAG "Not found \n");
      }
}

void Gatt::HandleGattcMultiadvDisableEvent(GattcMultiadvDisableEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                     event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_multiadv_disable_cb(event->client_if, event->status);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcCongestionEvent(GattcCongestionEvent *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) congested (%d)\n",__FUNCTION__, event->conn_id,
                                event->event_id, event->congested);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_congestion_cb(event->conn_id, event->congested);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcBatchscanCfgStorageEvent(GattcBatchscanCfgStorageEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                     event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgattc_batchscan_cfg_storage_cb(event->client_if, event->status);
     } else {
       ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcBatchscanStartstopEvent(GattcBatchscanStartstopEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                      event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;
     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_batchscan_startstop_cb(event->startstop_action, event->client_if, event->status);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcBatchscanReportsEvent(GattcBatchscanReportsEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                                event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if(it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_batchscan_reports_cb(event->client_if, event->status, event->report_format,
                               event->num_records, event->data_len, event->p_rep_data);
        } else {
        ALOGD(LOGTAG "Not found \n");
        }
}

void Gatt::HandleGattcBatchscanThresholdEvent(GattcBatchscanThresholdEvent *event)  {

      ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) \n",__FUNCTION__, event->client_if,
                      event->event_id);
      std::map<int,BluetoothGattClientCallback *> ::iterator it;

      it = clientCbCifMap.find(event->client_if);
      if(it != clientCbCifMap.end()) {
         ALOGD(LOGTAG "found \n");
         it->second->btgattc_batchscan_threshold_cb(event->client_if);
        } else {
         ALOGD(LOGTAG "Not found \n");
        }
}

void Gatt::HandleGattcTrackAdvEventEvent(GattcTrackAdvEventEvent *event)  {
     UNUSED
}

void Gatt::HandleGattcScanParameterSetupCompletedEvent(GattcScanParameterSetupCompletedEvent *event)  {
     ALOGD(LOGTAG "(%s) client_if (%d) event_id (%d) status (%d)\n",__FUNCTION__, event->client_if,
                    event->event_id, event->status);
     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(event->client_if);
     if (it != clientCbCifMap.end()) {
        ALOGD(LOGTAG "found \n");
        it->second->btgattc_scan_parameter_setup_completed_cb(event->client_if, event->status);
     } else {
        ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::HandleGattcGetGattDbEvent(GattcGetGattDbEvent *event)  {
     ALOGD(LOGTAG "(%s) conn_id (%d) event_id (%d) count (%d)\n",__FUNCTION__, event->conn_id,
                    event->event_id, event->count);

     std::map<int,BluetoothGattClientCallback *> ::iterator it;

     it = clientCbCifMap.find(ConnidClientifMap[event->conn_id]);
     if(it != clientCbCifMap.end()) {
       ALOGD(LOGTAG "found \n");
       it->second->btgattc_get_gatt_db_cb(event->conn_id, event->db, event->count);
     } else {
       ALOGD(LOGTAG "Not found \n");
     }
}

void Gatt::ProcessEvent(BtEvent* event)
{
    CHECK_PARAM_VOID(event)
    ALOGD(LOGTAG "(%s) Processing event %d \n ",__FUNCTION__, event->event_id);

    switch(event->event_id) {
        case BTGATTS_REGISTER_APP_EVENT:
            HandleGattsRegisterAppEvent((GattsRegisterAppEvent *)event);
            break;
        case BTGATTS_CONNECTION_EVENT:
            HandleGattsConnectionEvent((GattsConnectionEvent *)event);
            break;
        case BTGATTS_SERVICE_ADDED_EVENT:
            HandleGattsServiceAddedEvent((GattsServiceAddedEvent *)event);
            break;
        case BTGATTS_CHARACTERISTIC_ADDED_EVENT:
            HandleGattsCharacteristicAddedEvent((GattsCharacteristicAddedEvent *)event);
            break;
        case BTGATTS_DESCRIPTOR_ADDED_EVENT:
            HandleGattsDescriptorAddedEvent((GattsDescriptorAddedEvent *)event);
            break;
        case BTGATTS_SERVICE_STARTED_EVENT:
            HandleGattsServiceStartedEvent((GattsServiceStartedEvent *)event);
            break;
        case BTGATTS_SERVICE_STOPPED_EVENT:
            HandleGattsServiceStoppedEvent((GattsServiceStoppedEvent *)event);
            break;
        case BTGATTS_SERVICE_DELETED_EVENT:
            HandleGattsServiceDeletedEvent((GattsServiceDeletedEvent *)event);
            break;
        case BTGATTS_REQUEST_WRITE_EVENT:
            HandleGattsRequestWriteEvent((GattsRequestWriteEvent *)event);
            break;
        case BTGATTS_REQUEST_READ_EVENT:
            HandleGattsRequestReadEvent((GattsRequestReadEvent *) event);
            break;
        case BTGATTS_INCLUDED_SERVICE_ADDED_EVENT:
            HandleGattsIncludeServiceAddedEvent((GattsIncludedServiceAddedEvent *) event);
            break;
        case BTGATTS_REQUEST_EXEC_WRITE_EVENT:
            HandleGattsRequestExecWriteEvent((GattsRequestExecWriteEvent *) event);
            break;
        case BTGATTS_RESPONSE_CONFIRMATION_EVENT:
             HandleGattsResponseConfirmationEvent((GattsResponseConfirmationEvent*)event);
             break;
        case BTGATTS_INDICATION_SENT_EVENT:
             HandleGattsIndicationSentEvent((GattsIndicationSentEvent*)event);
             break;
        case BTGATTS_CONGESTION_EVENT:
             HandleGattsCongetionEvent((GattsCongestionEvent*)event);
             break;
        case BTGATTS_MTU_CHANGED_EVENT:
             HandleGattsMtuChangedEvent((GattsMTUchangedEvent*)event);
             break;
        case BTGATTC_REGISTER_APP_EVENT:
             HandleGattcRegisterAppEvent((GattcRegisterAppEvent *) event);
             break;
        case BTGATTC_SCAN_RESULT_EVENT:
             HandleGattcScanResultEvent((GattcScanResultEvent *) event);
             break;
        case BTGATTC_OPEN_EVENT:
             HandleGattcOpenEvent((GattcOpenEvent*) event);
             break;
        case BTGATTC_CLOSE_EVENT:
             HandleGattcCloseEvent((GattcCloseEvent*) event);
             break;
        case BTGATTC_SEARCH_COMPLETE_EVENT:
             HandleGattcSearchCompleteEvent((GattcSearchCompleteEvent*) event);
             break;
        case BTGATTC_REGISTER_FOR_NOTIFICATION_EVENT:
             HandleGattcRegisterForNotificationEvent((GattcRegisterForNotificationEvent*) event);
             break;
        case BTGATTC_NOTIFY_EVENT:
             HandleGattcNotifyEvent((GattcNotifyEvent*) event);
             break;
        case BTGATTC_READ_CHARACTERISTIC_EVENT:
             HandleGattcReadCharacteristicEvent((GattcReadCharacteristicEvent*) event);
             break;
        case BTGATTC_WRITE_CHARACTERISTIC_EVENT:
             HandleGattcWriteCharacteristicEvent((GattcWriteCharacteristicEvent*) event);
             break;
        case BTGATTC_READ_DESCRIPTOR_EVENT:
             HandleGattcReadDescriptorEvent((GattcReadDescriptorEvent*) event);
             break;
        case BTGATTC_WRITE_DESCRIPTOR_EVENT:
             HandleGattcWriteDescriptorEvent((GattcWriteDescriptorEvent *) event);
             break;
        case BTGATTC_EXECUTE_WRITE_EVENT:
             HandleGattcExecuteWriteEvent((GattcExecuteWriteEvent*) event);
             break;
        case BTGATTC_REMOTE_RSSI_EVENT:
             HandleGattcRemoteRssiEvent((GattcRemoteRssiEvent*) event);
             break;
        case BTGATTC_ADVERTISE_EVENT:
             HandleGattcAdvertiseEvent((GattcAdvertiseEvent*) event);
             break;
        case BTGATTC_CONFIGURE_MTU_EVENT:
             HandleGattcConfigureMtuEvent((GattcConfigureMtuEvent*) event);
             break;
        case BTGATTC_SCAN_FILTER_CFG_EVENT:
             HandleGattcScanFilterCfgEvent((GattcScanFilterCfgEvent*) event);
             break;
        case BTGATTC_SCAN_FILTER_PARAM_EVENT:
             HandleGattcScanFilterParamEvent((GattcScanFilterParamEvent*) event);
             break;
        case BTGATTC_SCAN_FILTER_STATUS_EVENT:
             HandleGattcScanFilterStatusEvent((GattcScanFilterStatusEvent*) event);
             break;
        case BTGATTC_MULTIADV_ENABLE_EVENT:
             HandleGattcMultiadvEnableEvent((GattcMultiadvEnableEvent*) event);
             break;
        case BTGATTC_MULTIADV_UPDATE_EVENT:
             HandleGattcMultiadvUpdateEvent((GattcMultiadvUpdateEvent*) event);
             break;
        case BTGATTC_MULTIADV_SETADV_DATA_EVENT:
             HandleGattcMultiadvSetadvDataEvent((GattcMultiadvSetadvDataEvent*) event);
             break;
        case BTGATTC_MULTIADV_DISABLE_EVENT:
             HandleGattcMultiadvDisableEvent((GattcMultiadvDisableEvent*) event);
             break;
        case BTGATTC_CONGESTION_EVENT:
             HandleGattcCongestionEvent((GattcCongestionEvent*) event);
             break;
        case BTGATTC_BATCHSCAN_CFG_STORAGE_EVENT:
             HandleGattcBatchscanCfgStorageEvent((GattcBatchscanCfgStorageEvent*) event);
             break;
        case BTGATTC_BATCHSCAN_STARTSTOP_EVENT:
             HandleGattcBatchscanStartstopEvent((GattcBatchscanStartstopEvent*) event);
             break;
        case BTGATTC_BATCHSCAN_REPORTS_EVENT:
             HandleGattcBatchscanReportsEvent((GattcBatchscanReportsEvent*) event);
             break;
        case BTGATTC_BATCHSCAN_THRESHOLD_EVENT:
             HandleGattcBatchscanThresholdEvent((GattcBatchscanThresholdEvent*) event);
             break;
        case BTGATTC_TRACK_ADV_EVENT_EVENT:
             HandleGattcTrackAdvEventEvent((GattcTrackAdvEventEvent*) event);
             break;
        case BTGATTC_SCAN_PARAMETER_SETUP_COMPLETED_EVENT:
             HandleGattcScanParameterSetupCompletedEvent((GattcScanParameterSetupCompletedEvent*) event);
             break;
        case BTGATTC_GET_GATT_DB_EVENT:
             HandleGattcGetGattDbEvent((GattcGetGattDbEvent*) event);
             break;
        default: //All fall-through, enable as needed
            ALOGD(LOGTAG  "(%s) Unhandled Event(%d)",__FUNCTION__, event->event_id);
            break;
    }
}

bool Gatt::GattInterfaceInit(const bt_interface_t *bt_interface)
{
    gatt_interface = (btgatt_interface_t*) bt_interface->get_profile_interface(BT_PROFILE_GATT_ID);
    if (gatt_interface == NULL)
    {
        ALOGD(LOGTAG "(%s) Failed to init gatt profile \n",__FUNCTION__);
        return false;
    }
    bt_status_t status = gatt_interface->init(&sGattCallbacks);
    return (status != BT_STATUS_SUCCESS ? false : true);
}

bt_status_t Gatt::scan( bool start, int client_if )
{
    clientIfScanStatusMap[client_if] = start;
    if (gatt_interface) {
       return gatt_interface->client->scan( start );
    }
}

btgatt_interface_t * Gatt::GetGattInterface()
{
    if (gatt_interface)
       return gatt_interface;
    else
       ALOGD(LOGTAG "(%s) gatt interface is null",__FUNCTION__);
}


void Gatt::RegisterServerCallback(BluetoothGattServerCallback * rspServerCb,bt_uuid_t *server_uuid)
{
     ALOGD(LOGTAG " RegisterCallback entry \n");
     std::map<uint8_t *,BluetoothGattServerCallback *> ::iterator it;

     serverCbUuidMap.insert(std::make_pair(server_uuid->uu, rspServerCb));
     ALOGD(LOGTAG " RegisterCallback exit \n");
}
void Gatt::UnRegisterServerCallback( int serverif)
{
     ALOGD(LOGTAG "UnRegisterServerCallback \n");
     std::map<uint8_t *,BluetoothGattServerCallback *> ::iterator it;
     std::map<int ,BluetoothGattServerCallback *> ::iterator it2;
     std::map<int ,int> ::iterator it3;

     for (it = serverCbUuidMap.begin(); it != serverCbUuidMap.end(); ++it) {

         if (it->second == serverCbSifMap[serverif]) {
            serverCbUuidMap.erase (it);
            break;
         }
     }
     it2 = serverCbSifMap.find(serverif);
     if (it2 != serverCbSifMap.end())
     serverCbSifMap.erase (it2);
     it3 = ConnidServerifMap.find(serverif);
     if (it3 != ConnidServerifMap.end())
     ConnidServerifMap.erase(it3);
}

void Gatt::RegisterClientCallback(BluetoothGattClientCallback * clientCb,bt_uuid_t *client_uuid)
{
     ALOGD(LOGTAG " RegisterCallback entry\n");
     clientCbUuidMap.insert(std::make_pair(client_uuid->uu, clientCb));
     ALOGD(LOGTAG " RegisterclientCallback exit \n");
}
void Gatt::UnRegisterClientCallback( int clientif)
{
     std::map<uint8_t *,BluetoothGattClientCallback *> ::iterator it;
     std::map<int ,BluetoothGattClientCallback *> ::iterator it2;
     std::map<int ,int> ::iterator it3;
     for (it = clientCbUuidMap.begin(); it != clientCbUuidMap.end(); ++it) {
          if (it->second == clientCbCifMap[clientif]) {
             clientCbUuidMap.erase (it);
             break;
          }
     }
     it2 = clientCbCifMap.find(clientif);
     if (it2 != clientCbCifMap.end())
     clientCbCifMap.erase (it2);
     it3 = ConnidClientifMap.find(clientif);
     if (it3 != ConnidClientifMap.end())
     ConnidClientifMap.erase(it3);
}

void Gatt::GattInterfaceCleanup()
{
    if (gatt_interface) {
        gatt_interface->cleanup();
        gatt_interface = NULL;
    }
}

Gatt::Gatt(const bt_interface_t *bt_interface, config_t *config)
{
    ALOGD(LOGTAG "(%s) Starting Up Gatt Instance",__FUNCTION__);
    this->gatt_interface = NULL;
    this->bluetooth_interface = bt_interface;
    this->config = config;
}
Gatt::~Gatt()
{
    ALOGD(LOGTAG  "(%s) Cleaning up GATT Interface",__FUNCTION__);
    GattInterfaceCleanup();
}

bool Gatt::HandleEnableGatt()
{
     ALOGD(LOGTAG "(%s) HandleEnableGatt \n",__FUNCTION__);

     if (GattInterfaceInit(bluetooth_interface)!= true) {
             ALOGD(LOGTAG "(%s) Gatt Initialization Failed \n ",__FUNCTION__);
             return false;
     } else {
             return true;
     }
}
bool Gatt::HandleDisableGatt()
{
    bool status = true;
    ALOGD(LOGTAG  "(%s) Closing Gatt Instance",__FUNCTION__);
    GattInterfaceCleanup();
    return status;
}


bt_status_t Gatt::register_client( bt_uuid_t *client_uuid ) {

            if (gatt_interface) {
                return gatt_interface->client->register_client(client_uuid);
            }
}


bt_status_t Gatt::unregister_client(int client_if ) {
            if (gatt_interface) {
                return gatt_interface->client->unregister_client(client_if);
            }
}


bt_status_t Gatt::clientConnect( int client_if, const bt_bdaddr_t *bd_addr,
                                         bool is_direct, int transport ) {

            if (gatt_interface) {
               return gatt_interface->client->connect( client_if, bd_addr, is_direct,
                                      transport );
            }
}


bt_status_t Gatt::clientDisconnect( int client_if, const bt_bdaddr_t *bd_addr,
                                int conn_id) {
            if (gatt_interface) {
                return gatt_interface->client->disconnect( client_if, bd_addr, conn_id);
            }
}


bt_status_t Gatt::listen(int client_if, bool start) {
            if (gatt_interface) {
                return gatt_interface->client->listen(client_if, start);
            }
}


bt_status_t Gatt::refresh( int client_if, const bt_bdaddr_t *bd_addr ) {
            if (gatt_interface) {
                return gatt_interface->client->refresh( client_if, bd_addr );
            }
}


bt_status_t Gatt::search_service(int conn_id, bt_uuid_t *filter_uuid ) {
            if (gatt_interface) {
                return gatt_interface->client->search_service(conn_id, filter_uuid );
            }
}

bt_status_t Gatt::read_characteristic( int conn_id, uint16_t handle,
                                int auth_req ) {
        if (gatt_interface) {
            return gatt_interface->client->read_characteristic(conn_id,
                                   handle, auth_req );
        }

}

bt_status_t Gatt::write_characteristic(int conn_id, uint16_t handle,
                                int write_type, int len, int auth_req,
                                char* p_value) {
        if (gatt_interface) {
            return gatt_interface->client->write_characteristic(conn_id,
                 handle, write_type, len, auth_req, p_value);
        }

}


bt_status_t Gatt::read_descriptor(int conn_id, uint16_t handle,
                                int auth_req) {
        if (gatt_interface) {
            return gatt_interface->client->read_descriptor(conn_id,
                handle, auth_req);
        }

}


bt_status_t Gatt::write_descriptor(int conn_id, uint16_t handle,
                                   int write_type, int len,
                                   int auth_req, char* p_value) {
        if (gatt_interface) {
            return gatt_interface->client->write_descriptor( conn_id, handle,
                   write_type, len, auth_req,p_value);
        }

}


bt_status_t Gatt::execute_write(int conn_id, int execute) {
        if (gatt_interface) {
            return gatt_interface->client->execute_write(conn_id, execute);
        }

}


bt_status_t Gatt::register_for_notification( int client_if,
                                const bt_bdaddr_t *bd_addr, uint16_t handle) {
        if (gatt_interface) {
            return gatt_interface->client->register_for_notification(client_if,
                                bd_addr, handle);
        }

}


bt_status_t Gatt::deregister_for_notification( int client_if,
                                const bt_bdaddr_t *bd_addr, uint16_t handle) {
        if (gatt_interface) {
            return gatt_interface->client->deregister_for_notification(client_if,
                                bd_addr, handle);
        }

}


bt_status_t Gatt::read_remote_rssi( int client_if, const bt_bdaddr_t *bd_addr) {
        if (gatt_interface) {
            return gatt_interface->client->read_remote_rssi( client_if, bd_addr);
        }

}


bt_status_t Gatt::scan_filter_param_setup(btgatt_filt_param_setup_t filt_param) {
        if (gatt_interface) {
            return gatt_interface->client->scan_filter_param_setup(filt_param);
        }

}



bt_status_t Gatt::scan_filter_add_remove(int client_if, int action, int filt_type,
                                         int filt_index, int company_id,
                                         int company_id_mask, const bt_uuid_t *p_uuid,
                                         const bt_uuid_t *p_uuid_mask, const bt_bdaddr_t *bd_addr,
                                         char addr_type, int data_len, char* p_data, int mask_len,
                                         char* p_mask) {
        if (gatt_interface) {
            return gatt_interface->client->scan_filter_add_remove(client_if, action, filt_type,
                   filt_index, company_id,company_id_mask, p_uuid,p_uuid_mask, bd_addr,
                    addr_type, data_len, p_data, mask_len, p_mask);
        }

}


bt_status_t Gatt::scan_filter_clear(int client_if, int filt_index) {
        if (gatt_interface) {
            return gatt_interface->client->scan_filter_clear(client_if, filt_index);
        }

}


bt_status_t Gatt::scan_filter_enable(int client_if, bool enable) {
        if (gatt_interface) {
            return gatt_interface->client->scan_filter_enable(client_if, enable);
        }

}


int Gatt::get_device_type( const bt_bdaddr_t *bd_addr ) {
        if (gatt_interface) {
            return gatt_interface->client->get_device_type( bd_addr );
        }

}


bt_status_t Gatt::set_adv_data(int client_if, bool set_scan_rsp, bool include_name,
                                bool include_txpower, int min_interval, int max_interval, int appearance,
                                uint16_t manufacturer_len, char* manufacturer_data,
                                uint16_t service_data_len, char* service_data,
                                uint16_t service_uuid_len, char* service_uuid) {
        if (gatt_interface) {
            return gatt_interface->client->set_adv_data(client_if, set_scan_rsp,
                                   include_name, include_txpower, min_interval,
                                   max_interval, appearance,manufacturer_len, manufacturer_data,
                                   service_data_len, service_data, service_uuid_len,service_uuid);
        }

}


bt_status_t Gatt::configure_mtu(int conn_id, int mtu) {
        if (gatt_interface) {
            return gatt_interface->client->configure_mtu(conn_id, mtu);
        }

}


bt_status_t Gatt::conn_parameter_update(const bt_bdaddr_t *bd_addr, int min_interval,
                                int max_interval, int latency, int timeout) {
        if (gatt_interface) {
            return gatt_interface->client->conn_parameter_update(bd_addr, min_interval,
                                max_interval, latency, timeout) ;
        }

}


bt_status_t Gatt::set_scan_parameters(int client_if, int scan_interval, int scan_window) {
        if (gatt_interface) {
            return gatt_interface->client->set_scan_parameters(client_if, scan_interval, scan_window);
        }

}


bt_status_t Gatt::multi_adv_enable(int client_if, int min_interval,int max_interval,int adv_type,
                         int chnl_map, int tx_power, int timeout_s) {
        if (gatt_interface) {
            return gatt_interface->client->multi_adv_enable(client_if, min_interval,max_interval,adv_type,
                         chnl_map, tx_power, timeout_s);
        }

}


bt_status_t Gatt::multi_adv_update(int client_if, int min_interval,int max_interval,int adv_type,
                         int chnl_map, int tx_power, int timeout_s) {
        if (gatt_interface) {
            return gatt_interface->client->multi_adv_update(client_if, min_interval,max_interval,adv_type,
                            chnl_map, tx_power, timeout_s);
        }

}


bt_status_t Gatt::multi_adv_set_inst_data(int client_if, bool set_scan_rsp, bool include_name,
                                bool incl_txpower, int appearance, int manufacturer_len,
                                char* manufacturer_data, int service_data_len,
                                char* service_data, int service_uuid_len, char* service_uuid) {
        if (gatt_interface) {
            return gatt_interface->client->multi_adv_set_inst_data(client_if, set_scan_rsp, include_name,
                                incl_txpower, appearance, manufacturer_len, manufacturer_data, service_data_len,
                                service_data, service_uuid_len, service_uuid);
        }

}


bt_status_t Gatt::multi_adv_disable(int client_if) {
        if (gatt_interface) {
            return gatt_interface->client->multi_adv_disable(client_if);
        }

}


bt_status_t Gatt::batchscan_cfg_storage(int client_if, int batch_scan_full_max,
        int batch_scan_trunc_max, int batch_scan_notify_threshold) {
        if (gatt_interface) {
            return gatt_interface->client->batchscan_cfg_storage(client_if, batch_scan_full_max,
                                   batch_scan_trunc_max, batch_scan_notify_threshold);
        }

}


bt_status_t Gatt::batchscan_enb_batch_scan(int client_if, int scan_mode,
        int scan_interval, int scan_window, int addr_type, int discard_rule) {
        if (gatt_interface) {
            return gatt_interface->client->batchscan_enb_batch_scan(client_if, scan_mode,
                                   scan_interval, scan_window, addr_type, discard_rule);
        }

}


bt_status_t Gatt::batchscan_dis_batch_scan(int client_if) {
        if (gatt_interface) {
            return gatt_interface->client->batchscan_dis_batch_scan(client_if);
        }

}



bt_status_t Gatt::batchscan_read_reports(int client_if, int scan_mode) {
        if (gatt_interface) {
            return gatt_interface->client->batchscan_read_reports(client_if, scan_mode);
        }

}


bt_status_t Gatt::test_command( int command, btgatt_test_params_t* params) {
        if (gatt_interface) {
            return gatt_interface->client->test_command( command, params);
        }

}

bt_status_t Gatt::get_gatt_db(int conn_id) {
        if (gatt_interface) {
            return gatt_interface->client->get_gatt_db(conn_id);
        }

}

bt_status_t Gatt:: register_server( bt_uuid_t *uuid ) {
   if (gatt_interface) {
       return gatt_interface->server->register_server(uuid);
   }
}

bt_status_t Gatt:: unregister_server(int server_if ) {
        if (gatt_interface) {
            return gatt_interface->server->unregister_server(server_if);
        }

}


bt_status_t Gatt:: serverConnect(int server_if, const bt_bdaddr_t *bd_addr,
                                                bool is_direct, int transport) {
        if (gatt_interface) {
            return gatt_interface->server->connect(server_if, bd_addr,
                                                is_direct, transport);
        }

}


bt_status_t Gatt:: serverDisconnect(int server_if, const bt_bdaddr_t *bd_addr,
                                int conn_id ) {
        if (gatt_interface) {
            return gatt_interface->server->disconnect(server_if, bd_addr, conn_id);
        }

}


bt_status_t Gatt:: add_service( int server_if, btgatt_srvc_id_t *srvc_id, int num_handles) {
        if (gatt_interface) {
            return gatt_interface->server->add_service(server_if, srvc_id, num_handles);
        }

}


bt_status_t Gatt:: add_included_service( int server_if, int service_handle, int included_handle) {
        if (gatt_interface) {
            return gatt_interface->server->add_included_service( server_if,service_handle, included_handle);
        }

}

bt_status_t Gatt:: add_characteristic( int server_if,
                                int service_handle, bt_uuid_t *uuid,
                                int properties, int permissions) {
        if (gatt_interface) {
            return gatt_interface->server->add_characteristic(server_if, service_handle, uuid,
                                                            properties, permissions);
        }

}


bt_status_t Gatt:: add_descriptor(int server_if, int service_handle,
                                                          bt_uuid_t *uuid, int permissions) {
        if (gatt_interface) {
            return gatt_interface->server->add_descriptor(server_if,
                                                        service_handle, uuid,
                                                        permissions);
        }

}


bt_status_t Gatt:: start_service(int server_if, int service_handle,
                                                         int transport) {
        if (gatt_interface) {
            return gatt_interface->server->start_service(server_if,
                                                        service_handle, transport);
        }

}


bt_status_t Gatt:: stop_service(int server_if, int service_handle) {
        if (gatt_interface) {
            return gatt_interface->server->stop_service(server_if,
                                                        service_handle);
        }

}


bt_status_t Gatt:: delete_service(int server_if, int service_handle) {
        if (gatt_interface) {
            return gatt_interface->server->delete_service(server_if,service_handle);
        }
}


bt_status_t Gatt:: send_indication(int server_if, int attribute_handle,
                                                           int conn_id, int len, int confirm,
                                                           char* p_value) {
        if (gatt_interface) {
            return gatt_interface->server->send_indication(server_if, attribute_handle,
                                           conn_id, len, confirm, p_value);
        }

}


bt_status_t Gatt:: send_response(int conn_id, int trans_id,
                                                         int status, btgatt_response_t *response) {
        if (gatt_interface) {
            return gatt_interface->server->send_response(conn_id, trans_id,
                                                         status, response);
        }

}


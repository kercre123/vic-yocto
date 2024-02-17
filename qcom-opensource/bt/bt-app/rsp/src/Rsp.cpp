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

#include "Gatt.hpp"
#include "Rsp.hpp"

#define LOGTAG "RSP "
#define UNUSED

Rsp *rsp = NULL;
int serverif, clientif;

class clientCallback : public BluetoothGattClientCallback
{
   public:
   void btgattc_client_register_app_cb(int status,int client_if,bt_uuid_t *uuid) {

        fprintf(stdout,"gattClinetRegisterAppCb\n ");

        GattcRegisterAppEvent event;
        event.event_id = RSP_ENABLE_EVENT;
        event.status = status;
        event.clientIf = client_if;
        rsp->SetRSPClientAppData(&event);

        rsp->ClientSetAdvData("Remote Start Profile");
        rsp->StartAdvertisement();
   }

   void btgattc_scan_result_cb(bt_bdaddr_t* bda, int rssi, uint8_t* adv_data) {
        UNUSED
   }

   void btgattc_open_cb(int conn_id, int status, int clientIf, bt_bdaddr_t* bda)
   {
        UNUSED
   }

   void btgattc_close_cb(int conn_id, int status, int clientIf, bt_bdaddr_t* bda)
   {
        UNUSED
   }

   void btgattc_search_complete_cb(int conn_id, int status)
   {
        UNUSED
   }

   void btgattc_search_result_cb(int conn_id, btgatt_srvc_id_t *srvc_id)
   {
        UNUSED
   }

   void btgattc_get_characteristic_cb(int conn_id, int status,
                                     btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *char_id,
                                     int char_prop)
   {
        UNUSED
   }

   void btgattc_get_descriptor_cb(int conn_id, int status,
                                 btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *char_id,
                                 btgatt_gatt_id_t *descr_id)
   {
        UNUSED
   }

   void btgattc_get_included_service_cb(int conn_id, int status,
                                       btgatt_srvc_id_t *srvc_id, btgatt_srvc_id_t *incl_srvc_id)
   {
        UNUSED
   }

   void btgattc_register_for_notification_cb(int conn_id, int registered,
                                                int status, btgatt_srvc_id_t *srvc_id,
                                                btgatt_gatt_id_t *char_id)
   {
        UNUSED
   }

   void btgattc_notify_cb(int conn_id, btgatt_notify_params_t *p_data)
   {
        UNUSED
   }

   void btgattc_read_characteristic_cb(int conn_id, int status,
                                          btgatt_read_params_t *p_data)
   {
        UNUSED
   }

   void btgattc_write_characteristic_cb(int conn_id, int status,
                                           btgatt_write_params_t *p_data)
   {
        UNUSED
   }

   void btgattc_read_descriptor_cb(int conn_id, int status, btgatt_read_params_t *p_data)
   {
        UNUSED
   }

    void btgattc_write_descriptor_cb(int conn_id, int status, btgatt_write_params_t *p_data)
    {
        UNUSED
    }

   void btgattc_execute_write_cb(int conn_id, int status)
   {
        UNUSED
   }

   void btgattc_remote_rssi_cb(int client_if,bt_bdaddr_t* bda, int rssi, int status)
   {
       UNUSED
   }

   void btgattc_advertise_cb(int status, int client_if)
   {
        UNUSED

   }

   void btgattc_configure_mtu_cb(int conn_id, int status, int mtu)
   {
        UNUSED
   }

   void btgattc_scan_filter_cfg_cb(int action, int client_if, int status, int filt_type, int avbl_space)
   {
        UNUSED
   }

   void btgattc_scan_filter_param_cb(int action, int client_if, int status, int avbl_space)
   {
        UNUSED
   }

   void btgattc_scan_filter_status_cb(int action, int client_if, int status)
   {
        UNUSED
   }

   void btgattc_multiadv_enable_cb(int client_if, int status)
   {
        UNUSED
   }

   void btgattc_multiadv_update_cb(int client_if, int status)
   {
        UNUSED
   }

    void btgattc_multiadv_setadv_data_cb(int client_if, int status)
   {
        UNUSED
   }

   void btgattc_multiadv_disable_cb(int client_if, int status)
   {
        UNUSED
   }

   void btgattc_congestion_cb(int conn_id, bool congested)
   {
        UNUSED
   }

   void btgattc_batchscan_cfg_storage_cb(int client_if, int status)
   {
        UNUSED
   }

   void btgattc_batchscan_startstop_cb(int startstop_action, int client_if, int status)
   {
        UNUSED

   }

   void btgattc_batchscan_reports_cb(int client_if, int status, int report_format,
        int num_records, int data_len, uint8_t *p_rep_data)
   {
        UNUSED
   }

   void btgattc_batchscan_threshold_cb(int client_if)
   {
        UNUSED
   }

   void btgattc_track_adv_event_cb(btgatt_track_adv_info_t *p_adv_track_info)
   {
        UNUSED
   }

   void btgattc_scan_parameter_setup_completed_cb(int client_if, btgattc_error_t status)
   {
        UNUSED
   }

};

class serverCallback :public BluetoothGattServerCallback
{

      public:

      void gattServerRegisterAppCb(int status, int server_if, bt_uuid_t *uuid) {

           fprintf(stdout,"gattServerRegisterAppCb status is %d, serverif is %d \n ",
                   status, server_if);

           if (status == BT_STATUS_SUCCESS)
           {
              GattsRegisterAppEvent rev;
              rev.event_id = RSP_ENABLE_EVENT;
              rev.server_if = server_if;
              rev.uuid = uuid;
              rev.status = status;
              fprintf(stdout," set rsp data \n");
              rsp->SetRSPAppData(&rev);
              rsp->AddService();
           } else {
              fprintf (stdout,"(%s) Failed to registerApp, %d \n",__FUNCTION__, server_if);
           }
      }

      void btgatts_connection_cb(int conn_id, int server_if, int connected, bt_bdaddr_t *bda)
      {

           fprintf(stdout,"btgatts_connection_cb  rsp \n ");

           GattsConnectionEvent event;
           event.event_id = RSP_ENABLE_EVENT;
           event.conn_id = conn_id;
           event.server_if = server_if;
           event.connected = connected;
           event.bda = bda;

           if (rsp) {
               rsp->SetRSPConnectionData(&event);
               if (connected)
               {
                   rsp->StopAdvertisement();
               }
           }
      }

      void btgatts_service_added_cb(int status, int server_if,
                                    btgatt_srvc_id_t *srvc_id, int srvc_handle)
      {
           fprintf(stdout,"btgatts_service_added_cb \n");
           if (status == BT_STATUS_SUCCESS) {
              GattsServiceAddedEvent event;
               event.event_id =RSP_ENABLE_EVENT;
               event.server_if = server_if;
               event.srvc_id = srvc_id;
               event.srvc_handle = srvc_handle;
               rsp->SetRSPSrvcData(&event);
               rsp->AddCharacteristics();
           } else {
               fprintf(stdout, "(%s) Failed to Add_Service %d ",__FUNCTION__, server_if);
           }
      }

      void btgatts_included_service_added_cb(int status, int server_if, int srvc_handle,
                                                   int incl_srvc_handle)
      {
            UNUSED;
      }

      void btgatts_characteristic_added_cb(int status, int server_if, bt_uuid_t *char_id,
                                                      int srvc_handle, int char_handle)
      {
           fprintf(stdout,"btgatts_characteristic_added_cb \n");
           if (status == BT_STATUS_SUCCESS) {
               GattsCharacteristicAddedEvent event;
               event.event_id =RSP_ENABLE_EVENT;
               event.server_if = server_if;
               event.char_id = char_id;
               event.srvc_handle = srvc_handle;
               event.char_handle = char_handle;
               rsp->SetRSPCharacteristicData(&event);
               rsp->AddDescriptor();
           } else {
               fprintf(stdout, "(%s) Failed to Add Characteristics %d ",__FUNCTION__, server_if);
           }
      }

      void btgatts_descriptor_added_cb(int status, int server_if, bt_uuid_t *descr_id,
                                                  int srvc_handle, int descr_handle)
      {
           fprintf(stdout,"btgatts_descriptor_added_cb \n");
           if (status == BT_STATUS_SUCCESS) {
               GattsDescriptorAddedEvent event;
               event.event_id =RSP_ENABLE_EVENT;
               event.server_if = server_if;
               event.descr_id= descr_id;
               event.srvc_handle = srvc_handle;
               event.descr_handle= descr_handle;
               rsp->SetRSPDescriptorData(&event);
               rsp->StartService();
            } else {
               fprintf(stdout, "(%s) Failed to add descriptor %d \n",__FUNCTION__, server_if);
            }
      }

      void btgatts_service_started_cb(int status, int server_if, int srvc_handle)
      {
           fprintf(stdout,"btgatts_service_started_cb \n");
           rsp->RegisterClient();
      }

      void btgatts_service_stopped_cb(int status, int server_if, int srvc_handle)
      {
           fprintf(stdout,"btgatts_service_stopped_cb \n");

          if (rsp) {
              if (!status)
                  rsp->DeleteService();
          }
          fprintf(stdout,  "RSP Service stopped successfully, deleting the service");
      }

      void btgatts_service_deleted_cb(int status, int server_if, int srvc_handle)
      {
         fprintf(stdout,"btgatts_service_deleted_cb \n");

          if (rsp) {
              if (!status) {
                  rsp->CleanUp(server_if);
                  delete rsp;
                  rsp = NULL;
              }
          }
          fprintf(stdout,"RSP Service stopped & Unregistered successfully\n");
      }

      void btgatts_request_read_cb(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle,
                                              int offset, bool is_long)
      {
           UNUSED;
      }

      void btgatts_request_write_cb(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle,
                                              int offset, int length, bool need_rsp, bool is_prep,
                                              uint8_t* value)
      {
           fprintf(stdout,"onCharacteristicWriteRequest \n");
           GattsRequestWriteEvent event;
           event.event_id = RSP_ENABLE_EVENT;
           event.conn_id = conn_id;
           event.trans_id = trans_id;
           event.bda = bda;
           event.attr_handle = attr_handle;
           event.offset = offset;
           event.length = length;
           event.need_rsp = need_rsp;
           event.is_prep = is_prep;
           event.value = value;
           rsp->SendResponse(&event);
      }

      void btgatts_request_exec_write_cb(int conn_id, int trans_id,
                                                      bt_bdaddr_t *bda, int exec_write)
      {
           UNUSED;
      }

      void btgatts_response_confirmation_cb(int status, int handle)
      {
           UNUSED;
      }

      void btgatts_indication_sent_cb(int conn_id, int status)
      {
           UNUSED;
      }

      void btgatts_congestion_cb(int conn_id, bool congested)
      {
           UNUSED;
      }

      void btgatts_mtu_changed_cb(int conn_id, int mtu)
      {
           UNUSED;
      }
};

serverCallback *serverCb = NULL;
clientCallback *clientCb = NULL;



Rsp::Rsp(btgatt_interface_t *gatt_itf, Gatt* gatt)
{

    fprintf(stdout,"rsp instantiated ");
    gatt_interface = gatt_itf;
    app_gatt = gatt;

    GattsRegisterAppEvent* p_app_if = GetRSPAppData();
    memset(p_app_if, 0, sizeof(GattsRegisterAppEvent));

    GattsCharacteristicAddedEvent* p_char_data = GetRSPCharacteristicData();
    memset(p_char_data, 0, sizeof(GattsCharacteristicAddedEvent));

    GattsDescriptorAddedEvent* p_desc_data = GetRSPDescriptorData();
    memset(p_desc_data, 0, sizeof(GattsDescriptorAddedEvent));
}


Rsp::~Rsp()
{
    fprintf(stdout, "(%s) RSP DeInitialized",__FUNCTION__);
    SetDeviceState(WLAN_INACTIVE);

    GattsRegisterAppEvent* p_app_if = GetRSPAppData();
    if(p_app_if->uuid != NULL)
        osi_free(p_app_if->uuid);

    GattsCharacteristicAddedEvent* p_char_data = GetRSPCharacteristicData();
    if(p_char_data->char_id != NULL)
        osi_free(p_char_data->char_id);

    GattsDescriptorAddedEvent* p_desc_data = GetRSPDescriptorData();
    if(p_desc_data->descr_id != NULL)
        osi_free(p_desc_data->descr_id);
}

bool Rsp::CopyUUID(bt_uuid_t *uuid)
{
    CHECK_PARAM(uuid)
    for (int i = 0; i < 16; i++) {
        uuid->uu[i] = 0x30;
    }
    return true;
}
bool Rsp::CopyCharacteristicsUUID(bt_uuid_t *uuid)
{
    CHECK_PARAM(uuid);
    uuid->uu[0] = 0xfb;
    uuid->uu[1] = 0x34;
    uuid->uu[2] = 0x9b;
    uuid->uu[3] = 0x5f;
    uuid->uu[4] = 0x80;
    uuid->uu[5] = 0x00;
    uuid->uu[6] = 0x00;
    uuid->uu[7] = 0x80;
    uuid->uu[8] = 0x00;
    uuid->uu[9] = 0x10;
    uuid->uu[10] = 0x00;
    uuid->uu[11] = 0x00;

    uuid->uu[12] = 0x02;
    uuid->uu[13] = 0xbb;

    uuid->uu[14] = 0x00;
    uuid->uu[15] = 0x00;

    return true;
}
bool Rsp::CopyDescriptorUUID(bt_uuid_t *uuid)
{
    CHECK_PARAM(uuid);
    uuid->uu[0] = 0xfb;
    uuid->uu[1] = 0x34;
    uuid->uu[2] = 0x9b;
    uuid->uu[3] = 0x5f;
    uuid->uu[4] = 0x80;
    uuid->uu[5] = 0x00;
    uuid->uu[6] = 0x00;
    uuid->uu[7] = 0x80;
    uuid->uu[8] = 0x00;
    uuid->uu[9] = 0x10;
    uuid->uu[10] = 0x00;
    uuid->uu[11] = 0x00;

    uuid->uu[12] = 0x03;
    uuid->uu[13] = 0xcc;

    uuid->uu[14] = 0x00;
    uuid->uu[15] = 0x00;

    return true;
}
bool Rsp::CopyServerUUID(bt_uuid_t *uuid)
{
    CHECK_PARAM(uuid);
    uuid->uu[0] = 0xfb;
    uuid->uu[1] = 0x34;
    uuid->uu[2] = 0x9b;
    uuid->uu[3] = 0x5f;
    uuid->uu[4] = 0x80;
    uuid->uu[5] = 0x00;
    uuid->uu[6] = 0x00;
    uuid->uu[7] = 0x80;
    uuid->uu[8] = 0x00;
    uuid->uu[9] = 0x10;
    uuid->uu[10] = 0x00;
    uuid->uu[11] = 0x00;

    uuid->uu[12] = 0x06;
    uuid->uu[13] = 0x00;

    uuid->uu[14] = 0x00;
    uuid->uu[15] = 0x00;

    return true;
}
bool Rsp::CopyServiceUUID(bt_uuid_t *uuid)
{
    CHECK_PARAM(uuid);
    uuid->uu[0] = 0xfb;
    uuid->uu[1] = 0x34;
    uuid->uu[2] = 0x9b;
    uuid->uu[3] = 0x5f;
    uuid->uu[4] = 0x80;
    uuid->uu[5] = 0x00;
    uuid->uu[6] = 0x00;
    uuid->uu[7] = 0x80;
    uuid->uu[8] = 0x00;
    uuid->uu[9] = 0x10;
    uuid->uu[10] = 0x00;
    uuid->uu[11] = 0x00;

    uuid->uu[12] = 0x01;
    uuid->uu[13] = 0xaa;

    uuid->uu[14] = 0x00;
    uuid->uu[15] = 0x00;
    return true;
}
bool Rsp::CopyClientUUID(bt_uuid_t *uuid)
{
    CHECK_PARAM(uuid)
    uuid->uu[0] = 0xff;
    uuid->uu[1] = 0x34;
    uuid->uu[2] = 0x9b;
    uuid->uu[3] = 0x5f;
    uuid->uu[4] = 0x80;
    uuid->uu[5] = 0x00;
    uuid->uu[6] = 0x00;
    uuid->uu[7] = 0x80;
    uuid->uu[8] = 0x00;
    uuid->uu[9] = 0x10;
    uuid->uu[10] = 0x00;
    uuid->uu[11] = 0x00;

    uuid->uu[12] = 0x05;
    uuid->uu[13] = 0x00;

    uuid->uu[14] = 0x00;
    uuid->uu[15] = 0x00;
    return true;
}

bool Rsp::CopyParams(bt_uuid_t *uuid_dest, bt_uuid_t *uuid_src)
{
    CHECK_PARAM(uuid_dest)
    CHECK_PARAM(uuid_src)

    for (int i = 0; i < 16; i++) {
        uuid_dest->uu[i] = uuid_src->uu[i];
    }
    return true;
}

bool Rsp::MatchParams(bt_uuid_t *uuid_dest, bt_uuid_t *uuid_src)
{
    CHECK_PARAM(uuid_dest)
    CHECK_PARAM(uuid_src)

    for (int i = 0; i < 16; i++) {
        if(uuid_dest->uu[i] != uuid_src->uu[i])
            return false;
    }
    fprintf(stdout, "(%s) UUID Matches",__FUNCTION__);
    return true;
}

bool Rsp::EnableRSP()
{
    fprintf(stdout, "(%s) Enable RSP Initiated \n",__FUNCTION__);

    RspEnableEvent rev;
    rev.event_id = RSP_ENABLE_EVENT;// change it later
    CopyCharacteristicsUUID(&rev.characteristics_uuid);
    CopyDescriptorUUID(&rev.descriptor_uuid);
    CopyServerUUID(&rev.server_uuid);
    CopyClientUUID(&rev.client_uuid);
    CopyServiceUUID(&rev.service_uuid);

    fprintf(stdout," set rsp data \n");
    SetRSPAttrData(&rev);
    RegisterApp();
}

bool Rsp::DisableRSP()
{
    fprintf(stdout, "(%s) Disable RSP Initiated",__FUNCTION__);
    StopService();
}

bool Rsp::RegisterApp()
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }
    serverCb = new serverCallback;
    bt_uuid_t server_uuid = GetRSPAttrData()->server_uuid;
    fprintf(stdout,"reg app addr is %d \n", GetRSPAttrData()->server_uuid);
    app_gatt->RegisterServerCallback(serverCb,&GetRSPAttrData()->server_uuid);
    return app_gatt->register_server(&server_uuid) == BT_STATUS_SUCCESS;
}

bool Rsp::RegisterClient()
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }
    clientCb = new clientCallback;
    bt_uuid_t client_uuid = GetRSPAttrData()->client_uuid;
    app_gatt->RegisterClientCallback(clientCb,&GetRSPAttrData()->client_uuid);
    return app_gatt->register_client(&client_uuid) == BT_STATUS_SUCCESS;
}

bool Rsp::UnregisterClient(int client_if)
{
    if (GetGattInterface() == NULL) {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }
    if(clientCb != NULL)
        delete clientCb;
    app_gatt->UnRegisterClientCallback(client_if);
    return app_gatt->unregister_client(client_if) == BT_STATUS_SUCCESS;
}

bool Rsp::ClientSetAdvData(char *str)
{
    bt_status_t        Ret;
    bool              SetScanRsp        = false;
    bool              IncludeName       = true;
    bool              IncludeTxPower    = false;
    int               min_conn_interval = RSP_MIN_CI;
    int               max_conn_interval = RSP_MAX_CI;

    app_gatt->set_adv_data(GetRSPClientAppData()->clientIf, SetScanRsp,
                                                IncludeName, IncludeTxPower, min_conn_interval,
                                                max_conn_interval, 0,strlen(str), str,
                                                strlen(str), str, 0,NULL);
}

void Rsp::CleanUp(int server_if)
{
    UnregisterServer(server_if);
    UnregisterClient(GetRSPClientAppData()->clientIf);
}

bool Rsp::UnregisterServer(int server_if)
{
    if (GetGattInterface() == NULL) {
        ALOGE(LOGTAG  "Gatt Interface Not present");
        return false;
    }
    app_gatt->UnRegisterServerCallback(server_if);
    if(serverCb != NULL)
        delete serverCb;
    return app_gatt->unregister_server(server_if) == BT_STATUS_SUCCESS;
}

bool Rsp::StartAdvertisement()
{
    if (GetGattInterface() == NULL) {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }
    fprintf(stdout,  "(%s) Listening on the interface (%d) ",__FUNCTION__,
            GetRSPAppData()->server_if);
    SetDeviceState(WLAN_INACTIVE);
    return app_gatt->listen(GetRSPClientAppData()->clientIf, true);
}

bool Rsp::SendResponse(GattsRequestWriteEvent *event)
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present \n",__FUNCTION__);
        return false;
    }
    CHECK_PARAM(event)
    btgatt_response_t att_resp;
    int response = -1;
    memset(att_resp.attr_value.value,0,BTGATT_MAX_ATTR_LEN);
    memcpy(att_resp.attr_value.value, event->value, event->length);
    att_resp.attr_value.handle = event->attr_handle;
    att_resp.attr_value.offset = event->offset;
    att_resp.attr_value.len = event->length;
    att_resp.attr_value.auth_req = 0;

    if(event->value != NULL && !strncasecmp((const char *)(event->value), "on", 2)) {
        if (GetDeviceState() == WLAN_INACTIVE)
        {
            fprintf(stdout, "(%s) Turn ON WLAN\n", __FUNCTION__);
            HandleWlanOn();
            SetDeviceState(WLAN_TRANSACTION_PENDING);
        }
        response = 0;
    } else {
        response = -1;
    }

    if (event->value != NULL) {
        fprintf(stdout, "(%s) Sending RSP response to write (%d) value (%s) "
            "State (%d)",__FUNCTION__, GetRSPAppData()->server_if, event->value,
            GetDeviceState());
        osi_free(event->value);
    }
    rsp->SetDeviceState(WLAN_ACTIVE);;
    return app_gatt->send_response(event->conn_id, event->trans_id,
                                                         response, &att_resp);
}

bool Rsp::HandleWlanOn()
{
    BtEvent *event = new BtEvent;
    CHECK_PARAM(event);
    event->event_id = SKT_API_IPC_MSG_WRITE;
    event->bt_ipc_msg_event.ipc_msg.type = BT_IPC_REMOTE_START_WLAN;
    event->bt_ipc_msg_event.ipc_msg.status = INITIATED;
    StopAdvertisement();
    fprintf(stdout, "(%s) Posting wlan start to main thread \n",__FUNCTION__);
    PostMessage (THREAD_ID_MAIN, event);
    return true;
}

bool Rsp::StopAdvertisement()
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }
    fprintf(stdout, "(%s) Stopping listen on the interface (%d) \n",__FUNCTION__,
            GetRSPClientAppData()->clientIf);
    return app_gatt->listen(GetRSPClientAppData()->clientIf, false);
}

bool Rsp::AddService()
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }
    btgatt_srvc_id_t srvc_id;
    srvc_id.id.inst_id = 0;   // 1 instance
    srvc_id.is_primary = 1;   // Primary addition
    srvc_id.id.uuid = GetRSPAttrData()->service_uuid;
    return app_gatt->add_service(GetRSPAppData()->server_if, &srvc_id,4)
                                                        ==BT_STATUS_SUCCESS;
}

bool Rsp::DisconnectServer()
{
    int server_if = GetRSPConnectionData()->server_if;
    bt_bdaddr_t * bda = GetRSPConnectionData()->bda;
    int conn_id = GetRSPConnectionData()->conn_id;
    fprintf(stdout,  "(%s) Disconnecting interface (%d), connid (%d) ",__FUNCTION__,
            server_if, conn_id);
    return app_gatt->serverDisconnect(server_if, bda, conn_id) == BT_STATUS_SUCCESS;
}

bool Rsp::DeleteService()
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }
    bool status = false;
    int srvc_handle = GetRspSrvcData()->srvc_handle;
    return app_gatt->delete_service(GetRSPAppData()->server_if,
                                                            srvc_handle) == BT_STATUS_SUCCESS;
}

bool Rsp::AddCharacteristics()
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }
    bt_uuid_t char_uuid;
    char_uuid=GetRSPAttrData()->characteristics_uuid;
    int srvc_handle = GetRspSrvcData()->srvc_handle;
    int server_if = GetRspSrvcData()->server_if;
    fprintf(stdout,  "(%s) Adding Characteristics server_if (%d), srvc_handle (%d) \n",
            __FUNCTION__, server_if,srvc_handle);
    return app_gatt->add_characteristic(server_if, srvc_handle, &char_uuid,
                                                            GATT_PROP_WRITE, GATT_PERM_WRITE)
                                                            ==BT_STATUS_SUCCESS;
}

bool Rsp::AddDescriptor(void)
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }

    bt_uuid_t desc_uuid;
    desc_uuid = GetRSPAttrData()->descriptor_uuid;
    int srvc_handle = GetRspSrvcData()->srvc_handle;
    return app_gatt->add_descriptor(GetRSPAppData()->server_if,
                                                        srvc_handle, &desc_uuid,
                                                        GATT_PERM_READ) == BT_STATUS_SUCCESS;
}

bool Rsp::StartService()
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }

    int srvc_handle = GetRspSrvcData()->srvc_handle;
    return app_gatt->start_service(GetRSPAppData()->server_if,
                                                        srvc_handle, GATT_TRANSPORT_LE)
                                                        == BT_STATUS_SUCCESS;
}

bool Rsp::StopService()
{
    if (GetGattInterface() == NULL)
    {
        ALOGE(LOGTAG  "(%s) Gatt Interface Not present",__FUNCTION__);
        return false;
    }

    int srvc_handle = GetRspSrvcData()->srvc_handle;
    return app_gatt->stop_service(GetRSPAppData()->server_if,
                                                        srvc_handle) == BT_STATUS_SUCCESS;
}

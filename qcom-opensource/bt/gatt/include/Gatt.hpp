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

#ifndef GATT_APP_H
#define GATT_APP_H

#pragma once
#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_gatt.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <map>


#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "osi/include/semaphore.h"
#include "osi/include/allocator.h"
#include "ipc.h"

#define MAX_GATT_DEVICES    (1)

#define REMOTE_START_PROFILE            (0x01)
#define ALERT_NOTIFICATION_PROFILE      (0x02)
#define CYCLING_SPEED__cADENCE_PROFILE  (0x04)
#define CYCLING_POWER_PROFILE           (0x08)
#define RUNNING_SPEED_CADENCE_PROFILE   (0x10)
#define HUMAN_INTERFACE_DEVICE_PROFILE  (0x20)
#define HEART_RATE_PROFILE              (0x40)

extern const char *BT_GATT_ENABLED;

#define CHECK_PARAM(x)                                                      \
   if (!x) {                                                                \
       ALOGE("'%s' Param is NULL - exiting from function ", __FUNCTION__);  \
       return false;                                                        \
   }

#define CHECK_PARAM_VOID(x)                                                      \
   if (!x) {                                                                     \
       ALOGE("'%s' Void Param is NULL - exiting from function ", __FUNCTION__);  \
       return ;                                                                  \
   }


 /**
   * This virtual class is used to implement {@link BluetoothGatt} callbacks.
   */

class BluetoothGattClientCallback {

   public:

      virtual void btgattc_client_register_app_cb(int status,int client_if,bt_uuid_t *uuid)
      {
      }
      virtual void btgattc_scan_result_cb(bt_bdaddr_t* bda, int rssi, uint8_t* adv_data)
      {
      }

      virtual void btgattc_open_cb(int conn_id, int status, int clientIf, bt_bdaddr_t* bda)
      {
      }

      virtual void btgattc_close_cb(int conn_id, int status, int clientIf, bt_bdaddr_t* bda)
      {
      }

      virtual void btgattc_search_complete_cb(int conn_id, int status)
      {
      }

      virtual void btgattc_register_for_notification_cb(int conn_id, int registered,
                                                        int status, uint16_t handle)
      {
      }

      virtual void btgattc_notify_cb(int conn_id, btgatt_notify_params_t *p_data)
      {
      }

      virtual void btgattc_read_characteristic_cb(int conn_id, int status,
        btgatt_read_params_t *p_data)
      {
      }

      virtual void btgattc_write_characteristic_cb(int conn_id, int status,
        uint16_t handle)
      {
      }

      virtual void btgattc_read_descriptor_cb(int conn_id, int status, btgatt_read_params_t *p_data)
      {
      }

      virtual void btgattc_write_descriptor_cb(int conn_id, int status, uint16_t handle)
      {
      }

      virtual void btgattc_execute_write_cb(int conn_id, int status)
      {
      }

      virtual void btgattc_remote_rssi_cb(int client_if,bt_bdaddr_t* bda, int rssi, int status)
      {
      }

      virtual void btgattc_advertise_cb(int status, int client_if)
      {
      }

      virtual void btgattc_configure_mtu_cb(int conn_id, int status, int mtu)
      {
      }

      virtual void btgattc_scan_filter_cfg_cb(int action, int client_if, int status, int filt_type, int avbl_space)
      {
      }

      virtual void btgattc_scan_filter_param_cb(int action, int client_if, int status, int avbl_space)
      {
      }

      virtual void btgattc_scan_filter_status_cb(int action, int client_if, int status)
      {
      }

      virtual void btgattc_multiadv_enable_cb(int client_if, int status)
      {
      }

      virtual void btgattc_multiadv_update_cb(int client_if, int status)
      {
      }

      virtual void btgattc_multiadv_setadv_data_cb(int client_if, int status)
      {
      }

      virtual void btgattc_multiadv_disable_cb(int client_if, int status)
      {
      }

      virtual void btgattc_congestion_cb(int conn_id, bool congested)
      {
      }

      virtual void btgattc_batchscan_cfg_storage_cb(int client_if, int status)
      {
      }

      virtual void btgattc_batchscan_startstop_cb(int startstop_action, int client_if, int status)
      {
      }

      virtual void btgattc_batchscan_reports_cb(int client_if, int status, int report_format,
                                                int num_records, int data_len, uint8_t *p_rep_data)
      {
      }
      virtual void btgattc_batchscan_threshold_cb(int client_if)
      {
      }

      virtual void btgattc_track_adv_event_cb(btgatt_track_adv_info_t *p_adv_track_info)
      {
      }

      virtual void btgattc_scan_parameter_setup_completed_cb(int client_if, btgattc_error_t status)
      {
      }
      virtual void btgattc_get_gatt_db_cb(int conn_id, btgatt_db_element_t *db, int count)
      {
      }
};

 /**
   * This virtual class is used to implement {@link BluetoothGattServer} callbacks.
   */
class BluetoothGattServerCallback {

   public:

     virtual void gattServerRegisterAppCb(int status, int server_if, bt_uuid_t *uuid)
     {
     }

     virtual void btgatts_connection_cb(int conn_id, int server_if, int connected, bt_bdaddr_t *bda)
     {
     }

     virtual void btgatts_service_added_cb(int status, int server_if,
                                 btgatt_srvc_id_t *srvc_id, int srvc_handle)
     {
     }

     virtual void btgatts_included_service_added_cb(int status, int server_if, int srvc_handle,
           int incl_srvc_handle)
     {
     }

     virtual void btgatts_characteristic_added_cb(int status, int server_if, bt_uuid_t *char_id,
                                                   int srvc_handle, int char_handle)
     {
     }

     virtual void btgatts_descriptor_added_cb(int status, int server_if, bt_uuid_t *descr_id,
                                               int srvc_handle, int descr_handle)
     {
     }

     virtual void btgatts_service_started_cb(int status, int server_if, int srvc_handle)
     {
     }

     virtual void btgatts_service_stopped_cb(int status, int server_if, int srvc_handle)
     {
     }

     virtual void btgatts_service_deleted_cb(int status, int server_if, int srvc_handle)
     {
     }

     virtual void btgatts_request_read_cb(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle,
                                           int offset, bool is_long)
     {
     }

     virtual void btgatts_request_write_cb(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle,
                                           int offset, int length, bool need_rsp, bool is_prep,
                                           uint8_t* value)
     {
     }

     virtual void btgatts_request_exec_write_cb(int conn_id, int trans_id,
                                                   bt_bdaddr_t *bda, int exec_write)
     {
     }

     virtual void btgatts_response_confirmation_cb(int status, int handle)
     {
     }

     virtual void btgatts_indication_sent_cb(int conn_id, int status)
     {
     }

     virtual void btgatts_congestion_cb(int conn_id, bool congested)
     {
     }

     virtual void btgatts_mtu_changed_cb(int conn_id, int mtu)
     {
     }
};

class Gatt {
    private:
        config_t *config;
        const bt_interface_t * bluetooth_interface;
        btgatt_interface_t *gatt_interface;

    public:

        bt_uuid_t rsp_uuid;
        int le_supported_profiles;
        std::map<uint8_t *,BluetoothGattServerCallback *> serverCbUuidMap;
        std::map<int ,BluetoothGattServerCallback *> serverCbSifMap;
        std::map<uint8_t *,BluetoothGattClientCallback *> clientCbUuidMap;
        std::map<int ,BluetoothGattClientCallback *> clientCbCifMap;
        std::map<int,int> ConnidClientifMap;
        std::map<int ,int> ConnidServerifMap;
        std::map<int,int> clientIfScanStatusMap;

        Gatt(const bt_interface_t *bt_interface, config_t *config);
        ~Gatt();
        void ProcessEvent(BtEvent* );
        bool Scan( bool ,int);
        void RegisterServerCallback(BluetoothGattServerCallback * ,bt_uuid_t*);
        void UnRegisterServerCallback(int);
        void RegisterClientCallback(BluetoothGattClientCallback * ,bt_uuid_t*);
        void UnRegisterClientCallback(int);
        bool GattInterfaceInit(const bt_interface_t *);
        btgatt_interface_t * GetGattInterface();
        void GattInterfaceCleanup(void);
        void HandleGattIpcMsg(BtIpcMsg *);
        void HandleGattsRegisterAppEvent(GattsRegisterAppEvent *);
        void HandleGattsConnectionEvent(GattsConnectionEvent *);
        void HandleGattsServiceAddedEvent(GattsServiceAddedEvent *);
        void HandleGattsCharacteristicAddedEvent(GattsCharacteristicAddedEvent *);
        void HandleGattsDescriptorAddedEvent(GattsDescriptorAddedEvent *);
        void HandleGattsServiceStartedEvent(GattsServiceStartedEvent *);
        void HandleGattsServiceStoppedEvent(GattsServiceStoppedEvent *);
        void HandleGattsServiceDeletedEvent(GattsServiceDeletedEvent *);
        void HandleGattsRequestWriteEvent(GattsRequestWriteEvent *);
        void HandleGattsRequestReadEvent(GattsRequestReadEvent *);
        void HandleGattsIncludeServiceAddedEvent(GattsIncludedServiceAddedEvent *);
        void HandleGattsRequestExecWriteEvent(GattsRequestExecWriteEvent *);
        void HandleGattsResponseConfirmationEvent(GattsResponseConfirmationEvent*);
        void HandleGattsIndicationSentEvent(GattsIndicationSentEvent*);
        void HandleGattsCongetionEvent(GattsCongestionEvent*);
        void HandleGattsMtuChangedEvent(GattsMTUchangedEvent*);
        void HandleGattcRegisterAppEvent(GattcRegisterAppEvent *);
        void HandleGattcScanResultEvent (GattcScanResultEvent *);
        void HandleGattcOpenEvent(GattcOpenEvent*) ;
        void HandleGattcCloseEvent(GattcCloseEvent*) ;
        void HandleGattcSearchCompleteEvent(GattcSearchCompleteEvent*) ;
        void HandleGattcRegisterForNotificationEvent(GattcRegisterForNotificationEvent*) ;
        void HandleGattcNotifyEvent(GattcNotifyEvent*) ;
        void HandleGattcReadCharacteristicEvent(GattcReadCharacteristicEvent*) ;
        void HandleGattcWriteCharacteristicEvent(GattcWriteCharacteristicEvent*) ;
        void HandleGattcReadDescriptorEvent(GattcReadDescriptorEvent*) ;
        void HandleGattcWriteDescriptorEvent(GattcWriteDescriptorEvent *) ;
        void HandleGattcExecuteWriteEvent(GattcExecuteWriteEvent*) ;
        void HandleGattcRemoteRssiEvent(GattcRemoteRssiEvent*) ;
        void HandleGattcAdvertiseEvent(GattcAdvertiseEvent*) ;
        void HandleGattcConfigureMtuEvent(GattcConfigureMtuEvent*) ;
        void HandleGattcScanFilterCfgEvent(GattcScanFilterCfgEvent*) ;
        void HandleGattcScanFilterParamEvent(GattcScanFilterParamEvent*) ;
        void HandleGattcScanFilterStatusEvent(GattcScanFilterStatusEvent*) ;
        void HandleGattcMultiadvEnableEvent(GattcMultiadvEnableEvent*) ;
        void HandleGattcMultiadvUpdateEvent(GattcMultiadvUpdateEvent*) ;
        void HandleGattcMultiadvSetadvDataEvent(GattcMultiadvSetadvDataEvent*) ;
        void HandleGattcMultiadvDisableEvent(GattcMultiadvDisableEvent*) ;
        void HandleGattcCongestionEvent(GattcCongestionEvent*) ;
        void HandleGattcBatchscanCfgStorageEvent(GattcBatchscanCfgStorageEvent*) ;
        void HandleGattcBatchscanStartstopEvent(GattcBatchscanStartstopEvent*) ;
        void HandleGattcBatchscanReportsEvent(GattcBatchscanReportsEvent*) ;
        void HandleGattcBatchscanThresholdEvent(GattcBatchscanThresholdEvent*) ;
        void HandleGattcTrackAdvEventEvent(GattcTrackAdvEventEvent*) ;
        void HandleGattcScanParameterSetupCompletedEvent(GattcScanParameterSetupCompletedEvent*) ;
        void HandleGattcGetGattDbEvent(GattcGetGattDbEvent *event);
        void HandleRspEnableEvent(RspEnableEvent *);
        void HandleRspDisableEvent(RspDisableEvent *);
        bool HandleEnableGatt(void);
        bool HandleDisableGatt(void);
        /** Register a client application from the stack */
        bt_status_t register_client( bt_uuid_t *uuid );

        /** Unregister a client application from the stack */
        bt_status_t unregister_client(int client_if );

        /** Start or stop LE device scanning */
        bt_status_t scan( bool start, int client_if );

        /** Create a connection to a remote LE or dual-mode device */
        bt_status_t clientConnect( int client_if, const bt_bdaddr_t *bd_addr,
                             bool is_direct, int transport );

        /** Disconnect a remote device or cancel a pending connection */
        bt_status_t clientDisconnect( int client_if, const bt_bdaddr_t *bd_addr,
                        int conn_id);

        /** Start or stop advertisements to listen for incoming connections */
        bt_status_t listen(int client_if, bool start);

        /** Clear the attribute cache for a given device */
        bt_status_t refresh( int client_if, const bt_bdaddr_t *bd_addr );

        /**
               * Enumerate all GATT services on a connected device.
               * Optionally, the results can be filtered for a given UUID.
               */
        bt_status_t search_service(int conn_id, bt_uuid_t *filter_uuid );

        /**
                * Enumerate included services for a given service.
                * Set start_incl_srvc_id to NULL to get the first included service.
                */
        bt_status_t get_included_service( int conn_id, btgatt_srvc_id_t *srvc_id,
                                             btgatt_srvc_id_t *start_incl_srvc_id);

        /**
                * Enumerate characteristics for a given service.
                * Set start_char_id to NULL to get the first characteristic.
                */
        bt_status_t get_characteristic( int conn_id,
                        btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *start_char_id);

        /**
                * Enumerate descriptors for a given characteristic.
                * Set start_descr_id to NULL to get the first descriptor.
                */
        bt_status_t get_descriptor( int conn_id,
                        btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *char_id,
                        btgatt_gatt_id_t *start_descr_id);

        /** Read a characteristic on a remote device */
        bt_status_t read_characteristic( int conn_id,
                        uint16_t handle, int auth_req );

        /** Write a remote characteristic */
        bt_status_t write_characteristic(int conn_id,
                        uint16_t handle, int write_type, int len, int auth_req,
                        char* p_value);

        /** Read the descriptor for a given characteristic */
        bt_status_t read_descriptor(int conn_id,
                        uint16_t handle, int auth_req);

        /** Write a remote descriptor for a given characteristic */
        bt_status_t write_descriptor( int conn_id,
                        uint16_t handle, int write_type, int len,
                        int auth_req, char* p_value);

        /** Execute a prepared write operation */
        bt_status_t execute_write(int conn_id, int execute);

        /**
                * Register to receive notifications or indications for a given
                * characteristic
                */
        bt_status_t register_for_notification( int client_if,
                        const bt_bdaddr_t *bd_addr, uint16_t handle);

        /** Deregister a previous request for notifications/indications */
        bt_status_t deregister_for_notification( int client_if,
                        const bt_bdaddr_t *bd_addr, uint16_t handle);

        /** Request RSSI for a given remote device */
        bt_status_t read_remote_rssi( int client_if, const bt_bdaddr_t *bd_addr);

        /** Setup scan filter params */
        bt_status_t scan_filter_param_setup(btgatt_filt_param_setup_t filt_param);


        /** Configure a scan filter condition  */
        bt_status_t scan_filter_add_remove(int client_if, int action, int filt_type,
                                       int filt_index, int company_id,
                                       int company_id_mask, const bt_uuid_t *p_uuid,
                                       const bt_uuid_t *p_uuid_mask, const bt_bdaddr_t *bd_addr,
                                       char addr_type, int data_len, char* p_data, int mask_len,
                                       char* p_mask);

        /** Clear all scan filter conditions for specific filter index*/
        bt_status_t scan_filter_clear(int client_if, int filt_index);

        /** Enable / disable scan filter feature*/
        bt_status_t scan_filter_enable(int client_if, bool enable);

        /** Determine the type of the remote device (LE, BR/EDR, Dual-mode) */
        int get_device_type( const bt_bdaddr_t *bd_addr );

        /** Set the advertising data or scan response data */
        bt_status_t set_adv_data(int client_if, bool set_scan_rsp, bool include_name,
                        bool include_txpower, int min_interval, int max_interval, int appearance,
                        uint16_t manufacturer_len, char* manufacturer_data,
                        uint16_t service_data_len, char* service_data,
                        uint16_t service_uuid_len, char* service_uuid);

        /** Configure the MTU for a given connection */
        bt_status_t configure_mtu(int conn_id, int mtu);

        /** Request a connection parameter update */
        bt_status_t conn_parameter_update(const bt_bdaddr_t *bd_addr, int min_interval,
                        int max_interval, int latency, int timeout);

        /** Sets the LE scan interval and window in units of N*0.625 msec */
        bt_status_t set_scan_parameters(int client_if, int scan_interval, int scan_window);

        /* Setup the parameters as per spec, user manual specified values and enable multi ADV */
        bt_status_t multi_adv_enable(int client_if, int min_interval,int max_interval,int adv_type,
                     int chnl_map, int tx_power, int timeout_s);

        /* Update the parameters as per spec, user manual specified values and restart multi ADV */
        bt_status_t multi_adv_update(int client_if, int min_interval,int max_interval,int adv_type,
                     int chnl_map, int tx_power, int timeout_s);

        /* Setup the data for the specified instance */
        bt_status_t multi_adv_set_inst_data(int client_if, bool set_scan_rsp, bool include_name,
                        bool incl_txpower, int appearance, int manufacturer_len,
                        char* manufacturer_data, int service_data_len,
                        char* service_data, int service_uuid_len, char* service_uuid);

        /* Disable the multi adv instance */
        bt_status_t multi_adv_disable(int client_if);

        /* Configure the batchscan storage */
        bt_status_t batchscan_cfg_storage(int client_if, int batch_scan_full_max,
            int batch_scan_trunc_max, int batch_scan_notify_threshold);

        /* Enable batchscan */
        bt_status_t batchscan_enb_batch_scan(int client_if, int scan_mode,
            int scan_interval, int scan_window, int addr_type, int discard_rule);

        /* Disable batchscan */
        bt_status_t batchscan_dis_batch_scan(int client_if);

        /* Read out batchscan reports */
        bt_status_t batchscan_read_reports(int client_if, int scan_mode);

        /** Test mode interface */
        bt_status_t test_command( int command, btgatt_test_params_t* params);

        /** get gatt database */
        bt_status_t get_gatt_db(int conn_id);

        /** Registers a GATT server application with the stack */
        bt_status_t register_server( bt_uuid_t *uuid );

        /** Unregister a server application from the stack */
        bt_status_t unregister_server(int server_if );

        /** Create a connection to a remote peripheral */
        bt_status_t serverConnect(int server_if, const bt_bdaddr_t *bd_addr,
                                bool is_direct, int transport);

        /** Disconnect an established connection or cancel a pending one */
        bt_status_t serverDisconnect(int server_if, const bt_bdaddr_t *bd_addr,
                        int conn_id );

        /** Create a new service */
        bt_status_t add_service( int server_if, btgatt_srvc_id_t *srvc_id, int num_handles);

        /** Assign an included service to it's parent service */
        bt_status_t add_included_service( int server_if, int service_handle, int included_handle);

        /** Add a characteristic to a service */
        bt_status_t add_characteristic( int server_if,
                        int service_handle, bt_uuid_t *uuid,
                        int properties, int permissions);

        /** Add a descriptor to a given service */
        bt_status_t add_descriptor(int server_if, int service_handle,
                                      bt_uuid_t *uuid, int permissions);

        /** Starts a local service */
        bt_status_t start_service(int server_if, int service_handle,
                                     int transport);

        /** Stops a local service */
        bt_status_t stop_service(int server_if, int service_handle);

        /** Delete a local service */
        bt_status_t delete_service(int server_if, int service_handle);

        /** Send value indication to a remote device */
        bt_status_t send_indication(int server_if, int attribute_handle,
                                       int conn_id, int len, int confirm,
                                       char* p_value);

        /** Send a response to a read/write operation */
        bt_status_t send_response(int conn_id, int trans_id,
                                     int status, btgatt_response_t *response);
};
#endif

/*
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
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

#include <list>
#include <map>
#include <string.h>
#include <hardware/bluetooth.h>
#include <hardware/hardware.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "osi/include/log.h"
#include "Gap.hpp"
#include "utils.h"
#ifdef USE_BT_OBEX
#include "oi_obex.h"
#include "oi_obex_lower.h"
#include "oi_wrapper.h"
#include "oi_osinterface.h"
#endif

const char *BT_LOCAL_DEV_NAME = "BtLocalDeviceName";
const char *BT_SCAN_MODE_TYPE = "BtScanMode";
const char *BT_USR_INPUT     = "UserInteractionNeeded";
const char *BT_A2DP_SINK_ENABLED_STRING  = "BtA2dpSinkEnable";
const char *BT_A2DP_SOURCE_ENABLED_STRING  = "BtA2dpSourceEnable";
const char *BT_HFP_CLIENT_ENABLED_STRING  = "BtHfClientEnable";
const char *BT_PAN_ENABLED    = "BtPanEnable";
const char *BT_GATT_ENABLED   = "BtGattEnable";
#ifdef USE_BT_OBEX
const char *BT_OBEX_ENABLED    = "BtObexEnable";
const char *BT_OBEX_LOG_LEVEL    = "BtObexLogLevel";
const char *BT_PBAP_CLIENT_ENABLED   = "BtPbapClientEnable";
const char *BT_OPP_ENABLED   = "BtOppEnable";
#endif
const char *BT_HFP_AG_ENABLED_STRING  = "BtHfpAGEnable";
const char *BT_AVRCP_ENABLED_STRING  = "BtAvrcpEnable";

#define LOGTAG "GAP "

using namespace std;
using std::list;
using std::string;

Gap *g_gap = NULL;


#ifdef __cplusplus
extern "C" {
#endif

static bool SetWakeAlarm(uint64_t delay_millis, bool should_wake, alarm_cb cb,
                                                                    void *data) {
    return BT_STATUS_SUCCESS;
}

static int AcquireWakeLock(const char *lock_name) {
    return BT_STATUS_SUCCESS;
}

static int ReleaseWakeLock(const char *lock_name) {
    return BT_STATUS_SUCCESS;
}

static bt_os_callouts_t callouts = {
    sizeof(bt_os_callouts_t),
    SetWakeAlarm,
    AcquireWakeLock,
    ReleaseWakeLock,
};

static void AdapterStateChangeCallback(bt_state_t state) {
    BtEvent *event = new BtEvent;

    ALOGV (LOGTAG " AdapterStateChangeCallback: state %d",state);

    event->event_id = GAP_EVENT_ADAPTER_STATE;
    event->state_event.status = state;
    PostMessage(THREAD_ID_GAP, event);
}

static void AdapterPropertiesCb(bt_status_t status, int num_properties,
                                            bt_property_t *properties) {
    bt_property_t *props;
    unsigned short index;
    BtEvent *event = new BtEvent;

    ALOGV (LOGTAG " adapter_properties_callback:");

    props = new bt_property_t[num_properties];
    memcpy(props, properties, num_properties * sizeof(bt_property_t));
    for (index = 0; index < num_properties; index++) {
        props[index].val = new char[properties[index].len];
        memcpy(props[index].val, properties[index].val, properties[index].len);
    }
    event->adapater_properties_event.num_properties = num_properties;
    event->adapater_properties_event.properties = props;

    event->event_id = GAP_EVENT_ADAPTER_PROPERTIES;
    PostMessage(THREAD_ID_GAP, event);
}

static void RemoteDevicePropertiesCb(bt_status_t status, bt_bdaddr_t *bd_addr,
                                int num_properties, bt_property_t *properties) {
    bt_property_t *props;
    unsigned short index;
    BtEvent *event = new BtEvent;

    ALOGV (LOGTAG " RemoteDevicePropertiesCb:");
    props = new bt_property_t[num_properties];
    memcpy(props, properties, num_properties * sizeof(bt_property_t));
    for (index = 0; index < num_properties; index++) {
        props[index].val = new char[properties[index].len];
        memcpy(props[index].val, properties[index].val, properties[index].len);
    }
    memcpy(&event->remote_properties_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    event->remote_properties_event.num_properties = num_properties;
    event->remote_properties_event.properties = props;

    event->event_id = GAP_EVENT_REMOTE_DEVICE_PROPERTIES;
    PostMessage(THREAD_ID_GAP, event);
}


static void DeviceFoundCb(int num_properties, bt_property_t *properties) {

    bt_property_t *props;
    unsigned short index;
    BtEvent *event = new BtEvent;

    ALOGV (LOGTAG " DeviceFoundCb:");
    props = new bt_property_t[num_properties];
    memcpy(props, properties, num_properties * sizeof(bt_property_t));
    for (index = 0; index < num_properties; index++) {
        props[index].val = new char[properties[index].len];
        memcpy(props[index].val, properties[index].val, properties[index].len);
    }
    event->device_found_event_int.num_properties = num_properties;
    event->device_found_event_int.properties = props;
    event->event_id = GAP_EVENT_DEVICE_FOUND_INT;
    PostMessage(THREAD_ID_GAP, event);
}


static void BondStateChangedCb(bt_status_t status, bt_bdaddr_t *bd_addr,
                                                bt_bond_state_t state) {

    BtEvent *event = new BtEvent;

    ALOGV (LOGTAG " BondStateChangedCb: %d", state);
    event->event_id = GAP_EVENT_BOND_STATE_INT;
    memcpy(&event->bond_state_event_int.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    event->bond_state_event_int.state = state;
    PostMessage(THREAD_ID_GAP, event);
}

static void AclStateChangedCb(bt_status_t status, bt_bdaddr_t *bd_addr,
                                                    bt_acl_state_t state) {

    BtEvent *event = new BtEvent;
    event->event_id = GAP_EVENT_ACL_STATE_CHANGED;
    memcpy(&event->acl_state_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    event->acl_state_event.status = status;
    event->acl_state_event.state = state;
    PostMessage(THREAD_ID_GAP, event);
    ALOGV (LOGTAG " AclStateChangedCb:");
}

static void DiscoveryStateChangedCb(bt_discovery_state_t state) {

    ALOGV (LOGTAG " DiscoveryStateChangedCb:");

    BtEvent *event = new BtEvent;
    event->event_id = GAP_EVENT_DISCOVERY_STATE_CHANGED;
    event->discovery_state_event.state = state;
    PostMessage(THREAD_ID_GAP, event);
}

static void PinRequestCb(bt_bdaddr_t *bd_addr, bt_bdname_t *bd_name,
                                uint32_t cod, bool min_16_digit) {
    BtEvent *event = new BtEvent;

    ALOGV (LOGTAG " PinRequestCb:");
    event->event_id = GAP_EVENT_PIN_REQUEST;
    memcpy(&event->pin_request_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    memcpy(&event->pin_request_event.bd_name, bd_name, sizeof(bt_bdname_t));

    event->pin_request_event.cod = cod;
    event->pin_request_event.secure = min_16_digit;
    PostMessage(THREAD_ID_GAP, event);
}

static void SspRequestCb(bt_bdaddr_t *bd_addr, bt_bdname_t *bd_name, uint32_t cod,
        bt_ssp_variant_t pairing_variant, uint32_t pass_key) {

    BtEvent *event = new BtEvent;
    memset(event, 0, sizeof(BtEvent));

    ALOGV (LOGTAG " SspRequestCb: name %s ", bd_name);
    event->event_id = GAP_EVENT_SSP_REQUEST;
    memcpy(&event->ssp_request_event.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    memcpy(&event->ssp_request_event.bd_name, bd_name, sizeof(bt_bdname_t));
    event->ssp_request_event.cod = cod;
    event->ssp_request_event.pairing_variant = pairing_variant;
    event->ssp_request_event.pass_key = pass_key;
    PostMessage(THREAD_ID_GAP, event);
}

static void CbThreadEvent(bt_cb_thread_evt event) {
    if (event  == ASSOCIATE_JVM) {
        ALOGV (LOGTAG " Callback thread attached: ");
    } else if (event == DISASSOCIATE_JVM) {
        ALOGE (LOGTAG " Callback: CbThreadEvent is not called on correct thread");
    }
}

static void DutModeRecvCb (uint16_t opcode, uint8_t *buf, uint8_t len) {
    ALOGV (LOGTAG " DutModeRecvCb ");
}

static void LeTestModeRecvCb (bt_status_t status, uint16_t packet_count) {

    ALOGV (LOGTAG " LeTestModeRecvCb: status:%d packet_count:%d ", status,
                                                            packet_count);
}

static void EnergyInfoRecvCb(bt_activity_energy_info *p_energy_info) {
    ALOGV (LOGTAG " EnergyInfoRecvCb: ");
}

//TODO: update the callbacks, made NULL to compile
static bt_callbacks_t sBluetoothCallbacks = {
    sizeof(sBluetoothCallbacks),
    AdapterStateChangeCallback,
    AdapterPropertiesCb,
    RemoteDevicePropertiesCb,
    DeviceFoundCb,
    DiscoveryStateChangedCb,
    PinRequestCb,
    SspRequestCb,
    BondStateChangedCb,
    AclStateChangedCb,
    CbThreadEvent,
    DutModeRecvCb,
    LeTestModeRecvCb,
    NULL,
    NULL,
};

static void SsrCleanupCb() {
    ALOGV (LOGTAG " SsrCleanupCb: ");
    BtEvent *event = new BtEvent;
    event->event_id = GAP_EVENT_SSR_CLEANUP;
    PostMessage(THREAD_ID_GAP, event);
}

static btvendor_callbacks_t sVendorCallbacks = {
    sizeof(sVendorCallbacks),
    NULL,
    SsrCleanupCb,
};

void BtGapMsgHandler(void *msg) {
    BtEvent* event = NULL;
    if (!msg) {
        printf("Msg is null, return.\n");
        return;
    }

    event = ( BtEvent *) msg;

    switch (event->event_id) {
        default:
            if (g_gap) {
                g_gap->ProcessEvent(( BtEvent *) msg);
            }
            delete event;
            break;
    }
}

void profile_startup_timer_expired(void *context) {
    ALOGV(LOGTAG, " profile_startup_timer_expired");

    BtEvent *event = new BtEvent;
    event->event_id = GAP_EVENT_PROFILE_START_TIMEOUT;
    PostMessage(THREAD_ID_GAP, event);
}

void profile_stop_timer_expired(void *context) {
    ALOGV(LOGTAG, " profile_stop_timer_expired");

    BtEvent *event = new BtEvent;
    event->event_id = GAP_EVENT_PROFILE_STOP_TIMEOUT;
    PostMessage(THREAD_ID_GAP, event);
}

void enable_timer_expired(void *context) {
    ALOGV(LOGTAG, " enable_timer_expired");

    BtEvent *event = new BtEvent;
    event->event_id = GAP_EVENT_ENABLE_TIMEOUT;
    PostMessage(THREAD_ID_GAP, event);
}

void disable_timer_expired(void *context) {
    ALOGV(LOGTAG, " disable_timer_expired");

    BtEvent *event = new BtEvent;
    event->event_id = GAP_EVENT_DISABLE_TIMEOUT;
    PostMessage(THREAD_ID_GAP, event);
}
#ifdef __cplusplus
}
#endif

void Gap::HandlePinRequestEvent(PINRequestEvent *event) {

    bt_pin_code_t pincode;
    BtEvent *bt_event;

    memset(&pincode, 0, sizeof(pincode));
    /* go for auto accept incase of user input disabled */
    if (!is_user_input_enabled_) {
        /* For now auto accept with most common pincode "0000" */
        pincode.pin[0] = '0';
        pincode.pin[1] = '0';
        pincode.pin[2] = '0';
        pincode.pin[3] = '0';
        bluetooth_interface_->pin_reply(&event->bd_addr, 1, 4, &pincode);
    } else {
        // pass the same event to Main thread
        bt_event = new BtEvent;
        memcpy(bt_event, event, sizeof(BtEvent));
        bt_event->event_id = MAIN_EVENT_PIN_REQUEST;
        PostMessage(THREAD_ID_MAIN, bt_event);
    }
}

void Gap::HandleSspRequestEvent(SSPRequestEvent *event) {
    DeviceProperties *remote_dev_prop;
    BtEvent *bt_event;
    bdstr_t bd_str;

    bdaddr_to_string(&event->bd_addr, &bd_str[0], sizeof(bd_str));

    string deviceAddress(bd_str);
    remote_dev_prop = remote_devices_obj_->GetDeviceProperties(event->bd_addr);

    if (remote_dev_prop == NULL ) {
        remote_dev_prop = remote_devices_obj_->AddDeviceProperties(event->bd_addr);
    }

    /* go for auto accept incase of user input disabled */
    if (!is_user_input_enabled_) {
        bluetooth_interface_->ssp_reply(&event->bd_addr, event->pairing_variant,
            1, event->pass_key);
    } else {
        // pass the same event to Main thread
        bt_event = new BtEvent;
        memcpy(bt_event, event, sizeof(BtEvent));
        bt_event->event_id = MAIN_EVENT_SSP_REQUEST;
        PostMessage(THREAD_ID_MAIN, bt_event);
    }
}


void Gap::HandleSspReply(SSPReplyEvent *event) {
    bluetooth_interface_->ssp_reply(&event->bd_addr, event->pairing_variant,
            event->accept, event->pass_key);
}

void Gap::HandlePinReply(PINReplyEvent *event) {
    bluetooth_interface_->pin_reply(&event->bd_addr, 1, event->pin_len, &event->pincode);
}

void Gap::HandleBondStateEvent(DeviceBondStateEventInt *event) {
    DeviceProperties *remote_dev_prop;

    remote_dev_prop = remote_devices_obj_->GetDeviceProperties(event->bd_addr);

    if (remote_dev_prop == NULL ) {
        remote_dev_prop = remote_devices_obj_->AddDeviceProperties(event->bd_addr);
    }

    if (remote_dev_prop->bond_state == event->state)
        return;

    adapter_properties_obj_->OnbondStateChanged(event->bd_addr, event->state, true);
}

void Gap::HandleEnable(void) {
    BtEvent  *bt_event  = NULL;
    if (adapter_properties_obj_->GetState() == BT_ADAPTER_STATE_OFF) {
       if(bluetooth_interface_->enable(false) == BT_STATUS_SUCCESS) {
           alarm_set(enable_timer, ENABLE_TIMEOUT_DELAY, enable_timer_expired, NULL);
           adapter_properties_obj_->SetState(BT_ADAPTER_STATE_TURNING_ON);
           return;
       } else {
           goto error;
       }
    }

error:
    //Sending update to the Main thread
    bt_event = new BtEvent;
    bt_event->event_id = MAIN_EVENT_ENABLED;
    bt_event->state_event.status = BT_STATE_OFF;
    PostMessage(THREAD_ID_MAIN, bt_event);
}

void Gap::HandleDisable(void) {
    BtEvent  *bt_event  = NULL;
    if ((adapter_properties_obj_->GetState() == BT_ADAPTER_STATE_ON) &&
       (bluetooth_interface_->disable() == BT_STATUS_SUCCESS)) {
        alarm_set(disable_timer, DISABLE_TIMEOUT_DELAY, disable_timer_expired, NULL);
        adapter_properties_obj_->SetState(BT_ADAPTER_STATE_TURNING_OFF);
    } else {
        //Sending update to the Main thread
        bt_event = new BtEvent;
        bt_event->event_id = MAIN_EVENT_DISABLED;
        bt_event->state_event.status = BT_STATE_ON;
        PostMessage(THREAD_ID_MAIN, bt_event);
    }
}

void Gap::HandleStartDiscovery(void) {
    if ((adapter_properties_obj_->GetState() == BT_ADAPTER_STATE_ON) &&
       (bluetooth_interface_->start_discovery() == BT_STATUS_SUCCESS)) {
    } else {
        //Sending update to the Main thread
        BtEvent *event = new BtEvent;
        event->event_id = GAP_EVENT_DISCOVERY_STATE_CHANGED;
        event->discovery_state_event.state = BT_DISCOVERY_STOPPED;
        PostMessage(THREAD_ID_GAP, event);
    }
}

void Gap::HandleStopDiscovery(void) {
    if ((adapter_properties_obj_->GetState() == BT_ADAPTER_STATE_ON) &&
       (bluetooth_interface_->cancel_discovery() == BT_STATUS_SUCCESS)) {
    } else {
        //Sending update to the Main thread
        BtEvent *event = new BtEvent;
        event->event_id = GAP_EVENT_DISCOVERY_STATE_CHANGED;
        event->discovery_state_event.state = BT_DISCOVERY_STARTED;
        PostMessage(THREAD_ID_GAP, event);
    }
}

bt_bdaddr_t *Gap::GetBtAddress(void) {
    return adapter_properties_obj_->GetBtAddress();
}

bt_bdname_t *Gap::GetBtName(void) {
    return adapter_properties_obj_->GetBtName();
}

int Gap::SetBtName(bt_property_t *prop) {
    return adapter_properties_obj_->SetBtName(prop);
}


bool Gap::IsDeviceBonded(bt_bdaddr_t device) {
    return adapter_properties_obj_->IsDeviceBonded(device);
}
void Gap::ProcessEvent(BtEvent* event) {
    bt_property_t prop;
    bt_scan_mode_t scan_mode;
    bt_bdname_t bd_name;
    BtEvent  *bt_event  = NULL;
    int profile_id, profile_count = 0;

    ALOGD(LOGTAG " Processing event %d", event->event_id);

    switch (event->event_id) {
        case GAP_EVENT_ADAPTER_STATE:
            adapter_properties_obj_->SetState((AdapterState)event->state_event.status);
            if ( event->state_event.status == BT_STATE_ON ) {

                if (enable_timer)
                    alarm_cancel(enable_timer);

                //Scan mode is BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE by default
                scan_mode = (bt_scan_mode_t)config_get_int(config_,
                            CONFIG_DEFAULT_SECTION, BT_SCAN_MODE_TYPE,
                                        BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
                prop.type = BT_PROPERTY_ADAPTER_SCAN_MODE;
                prop.val = &scan_mode;
                prop.len = sizeof(bt_scan_mode_t);
                bluetooth_interface_->set_adapter_property(&prop);

                prop.type = BT_PROPERTY_BDNAME;
                strlcpy((char*)&bd_name.name[0], config_get_string (config_,
                   CONFIG_DEFAULT_SECTION, BT_LOCAL_DEV_NAME, "MDM_Fluoride"), sizeof(bd_name));
                prop.val = &bd_name;
                prop.len = strlen((char*)bd_name.name);
                bluetooth_interface_->set_adapter_property(&prop);

                //Sending update to the Main thread
                bt_event = new BtEvent;
                bt_event->event_id = MAIN_EVENT_ENABLED;
                bt_event->state_event.status = event->state_event.status;
                PostMessage(THREAD_ID_MAIN, bt_event);

            } else if ( event->state_event.status == BT_STATE_OFF) {
                if (disable_timer)
                    alarm_cancel(disable_timer);

                //Sending update to the Main thread
                //TODO to check the right place for this
                scan_mode = BT_SCAN_MODE_NONE;
                prop.type = BT_PROPERTY_ADAPTER_SCAN_MODE;
                prop.val = &scan_mode;
                prop.len = sizeof(bt_scan_mode_t);
                bluetooth_interface_->set_adapter_property(&prop);

                adapter_properties_obj_->FlushBondedDeviceList();
                remote_devices_obj_->FlushDiscoveredDeviceList();
                // cleanup the stack
                bluetooth_interface_->cleanup();
#ifdef USE_BT_OBEX
                if (is_obex_enabled_) {
                    OI_OBEX_LOWER_SetSocketInterface(NULL);
                    OI_OBEX_Deinit();
                    sock_interface_ = NULL;
                }
#endif
                bt_event = new BtEvent;
                bt_event->event_id = MAIN_EVENT_DISABLED;
                bt_event->state_event.status = event->state_event.status;
                PostMessage(THREAD_ID_MAIN, bt_event);

            }
            break;

        case GAP_API_ENABLE:

            if (adapter_properties_obj_->GetState() == BT_ADAPTER_STATE_OFF) {
                bluetooth_interface_->init(&sBluetoothCallbacks);
            }
            else {
                ALOGV (LOGTAG "Ignoring GAP_API_ENABLE command state : %d",
                                adapter_properties_obj_->GetState());
                fprintf(stdout, "Ignoring GAP_API_ENABLE command state : %d\n",
                                adapter_properties_obj_->GetState());

                //Sending update to the Main thread
                bt_event = new BtEvent;
                bt_event->event_id = MAIN_EVENT_ENABLED;
                bt_event->state_event.status = BT_STATE_ON;
                PostMessage(THREAD_ID_MAIN, bt_event);
                break;
            }

            // check if there are profiles enabled
            if(!supported_profiles_count) {
                HandleEnable();
                break;
            }

            ALOGV (LOGTAG "Start QC BT Daemon");
            system("qcbtdaemon &");

#ifdef USE_BT_OBEX
            /* Initialize OBEX if enabled in config */
            if (is_obex_enabled_) {
                if ((sock_interface_ = (btsock_interface_t *)
                    bluetooth_interface_->get_profile_interface(BT_PROFILE_SOCKETS_ID)) == NULL) {
                    ALOGE(LOGTAG "%s: Failed to get Bluetooth socket interface", __FUNCTION__);
                } else {
                    OI_OBEX_Init(50);
                    OI_OBEX_LOWER_SetSocketInterface(sock_interface_);
                    OI_SetLogLevel(obex_logging_level_);
                }
            }
#endif

            // reset start status for all supported profiles
            for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                                profile_id++) {
                if(profile_config[profile_id].is_enabled) {
                    profile_config[profile_id].start_status = false;
                }
            }

             // start the profile start timer
            alarm_set(profile_startup_timer, PROFILE_STARTUP_TIMEOUT_DELAY,
                                profile_startup_timer_expired, NULL);

            for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                                profile_id++) {
                if(profile_config[profile_id].is_enabled) {
                    bt_event = new BtEvent;
                    bt_event->event_id = PROFILE_API_START;
                    ALOGD(LOGTAG " sending start to Profile %d",
                        profile_id);
                    PostMessage(profile_config[profile_id].thread_id, bt_event);
                }
            }
            break;

        case PROFILE_EVENT_START_DONE:

            // set the start status for the given profile
            for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                                profile_id++) {
                if((profile_config[profile_id].is_enabled)  &&
                   ((profile_config[profile_id].profile_id ==
                            event->profile_start_event.profile_id))) {
                    ALOGD(LOGTAG " Profile %d started with status %d",
                        profile_id, event->profile_stop_event.status);
                    profile_config[profile_id].start_status =
                    event->profile_start_event.status;
                }
            }

            // check if all profiles started
            for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                                profile_id++) {
                if((profile_config[profile_id].is_enabled)  &&
                    (!profile_config[profile_id].start_status)) {
                    return;
                }
            }

            ALOGD(LOGTAG " All profiles started");
            //stoping profile_startup_timer
            alarm_cancel(profile_startup_timer);
            HandleEnable();

            break;
        case PROFILE_EVENT_STOP_DONE:

            // set the stop status for the given profile
            for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                                profile_id++) {
                if((profile_config[profile_id].is_enabled)  &&
                   ((profile_config[profile_id].profile_id ==
                        event->profile_stop_event.profile_id))) {
                    ALOGD(LOGTAG " Profile %d stopped with status %d",
                        profile_id, event->profile_stop_event.status);
                    profile_config[profile_id].stop_status =
                    event->profile_stop_event.status;
                }
            }

            // check if all profiles stopped
            for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                                profile_id++) {
                if((profile_config[profile_id].is_enabled)  &&
                    (!profile_config[profile_id].stop_status)) {
                    return;
                }
            }

            ALOGD(LOGTAG " All profiles stopped");
            //stoping profile_stop_timer
            alarm_cancel(profile_stop_timer);
            HandleDisable();
            break;

        case GAP_EVENT_PROFILE_START_TIMEOUT:
        case GAP_EVENT_PROFILE_STOP_TIMEOUT:
        case GAP_EVENT_DISABLE_TIMEOUT:
        case GAP_EVENT_ENABLE_TIMEOUT:
            ALOGD(LOGTAG " Killing the proces due to timeout %d", event->event_id);
            fprintf(stderr, " Killing the proces due to timeout %d\n", event->event_id);
            kill(getpid(), SIGKILL);
            break;
        case GAP_API_DISABLE:
            if (adapter_properties_obj_->GetState() != BT_ADAPTER_STATE_ON) {
                ALOGV (LOGTAG "Ignoring GAP_API_DISABLE command state : %d",
                                            adapter_properties_obj_->GetState());
                fprintf(stdout, "Ignoring GAP_API_DISABLE command state : %d\n",
                                            adapter_properties_obj_->GetState());

                //Sending update to the Main thread
                bt_event = new BtEvent;
                bt_event->event_id = MAIN_EVENT_DISABLED;
                bt_event->state_event.status = BT_STATE_OFF;
                PostMessage(THREAD_ID_MAIN, bt_event);
                break;

            }
            if (profile_config[PROFILE_ID_A2DP_SINK].is_enabled)
            {
                bt_event = new BtEvent;
                bt_event->event_id = A2DP_SINK_CLEANUP_REQ;
                PostMessage(THREAD_ID_A2DP_SINK, bt_event);
                break;
            }

            /*Fall through*/
        case A2DP_SINK_CLEANUP_DONE:
            // check if there are profiles enabled
            if(!supported_profiles_count) {
                HandleDisable();
                ALOGV (LOGTAG "Stop QC BT Daemon");
                system("killall -s SIGTERM qcbtdaemon");
                break;
            }

            ALOGV (LOGTAG "Stop QC BT Daemon");
            system("killall -s SIGTERM qcbtdaemon");

            // reset stop status for all supported profiles
            for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                                profile_id++) {
                if(profile_config[profile_id].is_enabled) {
                    profile_config[profile_id].stop_status = false;
                }
            }

            // start the profile stop timer
            alarm_set(profile_stop_timer, PROFILE_STOP_TIMEOUT_DELAY,
                            profile_stop_timer_expired, NULL);

            for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                            profile_id++) {
                if(profile_config[profile_id].is_enabled &&
                    profile_config[profile_id].start_status) {
                    bt_event = new BtEvent;
                    bt_event->event_id = PROFILE_API_STOP;
                    PostMessage(profile_config[profile_id].thread_id, bt_event);
                }
            }
            break;

        case GAP_API_SET_BDNAME:
            SetBtName(&event->set_device_name_event.prop);
            config_set_string(config_,CONFIG_DEFAULT_SECTION,BT_LOCAL_DEV_NAME,(char *)event->set_device_name_event.prop.val);
            break;

        case GAP_EVENT_DEVICE_FOUND_INT:
            remote_devices_obj_->DeviceFound(&event->device_found_event_int);
            break;

        case GAP_EVENT_REMOTE_DEVICE_PROPERTIES:
            remote_devices_obj_->RemoteDeviceProperties(
                                            &event->remote_properties_event);
            break;

        case GAP_EVENT_ADAPTER_PROPERTIES:
            adapter_properties_obj_->AdapterPropertiesUpdate(
                                            &event->adapater_properties_event);
            break;

        case GAP_EVENT_PIN_REQUEST:
            HandlePinRequestEvent(&event->pin_request_event);
            break;

        case GAP_EVENT_SSP_REQUEST:
            HandleSspRequestEvent(&event->ssp_request_event);
            break;

        case GAP_EVENT_BOND_STATE_INT:
            HandleBondStateEvent(&event->bond_state_event_int);
            break;

        case GAP_EVENT_ACL_STATE_CHANGED:
            remote_devices_obj_->HandleAclStateChange(event->acl_state_event.status,
                event->acl_state_event.bd_addr, event->acl_state_event.state);
            break;

        case GAP_EVENT_DISCOVERY_STATE_CHANGED:
            adapter_properties_obj_->HandleDiscoveryStateChange(
                                            event->discovery_state_event.state);
            break;
        case GAP_API_START_INQUIRY:
            HandleStartDiscovery();
            break;

        case GAP_API_STOP_INQUIRY:
            HandleStopDiscovery();
            break;

        case GAP_API_CREATE_BOND:
            // Calling the cancel_discovery before create_bond
            bluetooth_interface_->cancel_discovery();
            bluetooth_interface_->create_bond(&event->bond_device.bd_addr, 1);
            break;

        case GAP_API_SSP_REPLY:
            HandleSspReply(&event->ssp_reply_event);
            break;

        case GAP_API_PIN_REPLY:
            HandlePinReply(&event->pin_reply_event);
            break;

        case GAP_EVENT_SSR_CLEANUP:
            /* Audio related cleanup can be done here.*/
            ALOGD(LOGTAG " Killing the proces after SSR_CLEANUP %d", event->event_id);
            kill(getpid(), SIGKILL);
            break;

        default:
            ALOGD(LOGTAG " Unhandled event %d", event->event_id);
            break;
    }
}

Gap :: Gap(const bt_interface_t *bt_interface, config_t *config) {

    int profile_id;
    this->bluetooth_interface_ = bt_interface;
    this->config_ = config;

    profile_startup_timer = NULL;
    profile_stop_timer = NULL;
    enable_timer = NULL;
    disable_timer = NULL;
    supported_profiles_count = 0;

    //checking for user input
    is_user_input_enabled_ = config_get_bool (config, CONFIG_DEFAULT_SECTION,
                                    BT_USR_INPUT, false);

#ifdef USE_BT_OBEX
    is_obex_enabled_ = config_get_bool (config,
                     CONFIG_DEFAULT_SECTION, BT_OBEX_ENABLED, false);

    obex_logging_level_ = config_get_int (config,
                     CONFIG_DEFAULT_SECTION, BT_OBEX_LOG_LEVEL, OI_MSG_CODE_TRACE);
#endif

    if ((bluetooth_interface_->init(&sBluetoothCallbacks) == BT_STATUS_SUCCESS))
        bt_interface->set_os_callouts(&callouts);

    this->remote_devices_obj_ = new RemoteDevices(bluetooth_interface_);
    this->adapter_properties_obj_ = new AdapterProperties(bluetooth_interface_,
                                                        remote_devices_obj_);

    for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                        profile_id++) {

        this->profile_config[profile_id].profile_id =  (ProfileIdType) profile_id;
        this->profile_config[profile_id].is_enabled = false;
        this->profile_config[profile_id].start_status = false;
        this->profile_config[profile_id].stop_status = false;

        if(profile_id == PROFILE_ID_BT_AM)
            this->profile_config[profile_id].thread_id = THREAD_ID_BT_AM;
        else if(profile_id == PROFILE_ID_A2DP_SINK)
            this->profile_config[profile_id].thread_id = THREAD_ID_A2DP_SINK;
        else if(profile_id == PROFILE_ID_A2DP_SOURCE)
            this->profile_config[profile_id].thread_id = THREAD_ID_A2DP_SOURCE;
        else if(profile_id == PROFILE_ID_HFP_CLIENT)
            this->profile_config[profile_id].thread_id = THREAD_ID_HFP_CLIENT;
        else if (profile_id == PROFILE_ID_PAN)
            this->profile_config[profile_id].thread_id = THREAD_ID_PAN;
        else if (profile_id == PROFILE_ID_GATT)
            this->profile_config[profile_id].thread_id = THREAD_ID_GATT;
        else if (profile_id == PROFILE_ID_SDP_CLIENT)
            this->profile_config[profile_id].thread_id = THREAD_ID_SDP_CLIENT;
#ifdef USE_BT_OBEX
        else if (profile_id == PROFILE_ID_PBAP_CLIENT)
            this->profile_config[profile_id].thread_id = THREAD_ID_PBAP_CLIENT;
        else if (profile_id == PROFILE_ID_OPP)
            this->profile_config[profile_id].thread_id = THREAD_ID_OPP;
#endif
        else if(profile_id == PROFILE_ID_HFP_AG)
            this->profile_config[profile_id].thread_id = THREAD_ID_HFP_AG;
        else if(profile_id == PROFILE_ID_AVRCP)
            this->profile_config[profile_id].thread_id = THREAD_ID_AVRCP;

    }

    this->profile_config[PROFILE_ID_A2DP_SINK].is_enabled = config_get_bool (config,
                     CONFIG_DEFAULT_SECTION, BT_A2DP_SINK_ENABLED_STRING, false);

    this->profile_config[PROFILE_ID_A2DP_SOURCE].is_enabled = config_get_bool (config,
                     CONFIG_DEFAULT_SECTION, BT_A2DP_SOURCE_ENABLED_STRING, false);

    this->profile_config[PROFILE_ID_HFP_CLIENT].is_enabled = config_get_bool (config,
                     CONFIG_DEFAULT_SECTION, BT_HFP_CLIENT_ENABLED_STRING, false);

    this->profile_config[PROFILE_ID_HFP_AG].is_enabled = config_get_bool (config,
                     CONFIG_DEFAULT_SECTION, BT_HFP_AG_ENABLED_STRING, false);

    this->profile_config[PROFILE_ID_AVRCP].is_enabled = config_get_bool (config,
                     CONFIG_DEFAULT_SECTION, BT_AVRCP_ENABLED_STRING, false);

    if ((this->profile_config[PROFILE_ID_A2DP_SINK].is_enabled) ||
        (this->profile_config[PROFILE_ID_HFP_CLIENT].is_enabled)) {
        this->profile_config[PROFILE_ID_BT_AM].is_enabled = true;
    }

    this->profile_config[PROFILE_ID_PAN].is_enabled = config_get_bool (config,
                     CONFIG_DEFAULT_SECTION, BT_PAN_ENABLED, false);

    this->profile_config[PROFILE_ID_GATT].is_enabled = config_get_bool (config,
                     CONFIG_DEFAULT_SECTION, BT_GATT_ENABLED, false);

    // SDP Client should be enabled and is not configurable to be disabled
    this->profile_config[PROFILE_ID_SDP_CLIENT].is_enabled = true;

#ifdef USE_BT_OBEX
    this->profile_config[PROFILE_ID_PBAP_CLIENT].is_enabled = config_get_bool (config,
                     CONFIG_DEFAULT_SECTION, BT_PBAP_CLIENT_ENABLED, false);

    this->profile_config[PROFILE_ID_OPP].is_enabled = config_get_bool (config,
                 CONFIG_DEFAULT_SECTION, BT_OPP_ENABLED, false);
#endif

    for(profile_id = PROFILE_ID_A2DP_SINK; profile_id < PROFILE_ID_MAX;
                                                            profile_id++) {
        if(this->profile_config[profile_id].is_enabled) {
            this->supported_profiles_count++;
        }
    }
    // Vendor interface
    sBtVendorInterface = (btvendor_interface_t *)bluetooth_interface_->
                            get_profile_interface(BT_PROFILE_VENDOR_ID);

    if (sBtVendorInterface != NULL) {
        sBtVendorInterface->init(&sVendorCallbacks);
    }

    if( !(profile_startup_timer = alarm_new())) {
        ALOGE(LOGTAG, " unable to create profile_startup_timer timer.");
        return;
    }

    if( !(profile_stop_timer = alarm_new())) {
        ALOGE(LOGTAG, " unable to create profile_stop_timer timer.");
        return;
    }

    if( !(enable_timer = alarm_new())) {
        ALOGE(LOGTAG, " unable to create enable_timer timer.");
        return;
    }

    if( !(disable_timer = alarm_new())) {
        ALOGE(LOGTAG, " unable to create disable_timer timer.");
        return;
    }
}

Gap :: ~Gap() {
    delete adapter_properties_obj_;
    delete remote_devices_obj_;

    alarm_free(profile_startup_timer);
    profile_startup_timer = NULL;

    alarm_free(profile_stop_timer);
    profile_stop_timer = NULL;

    alarm_free(enable_timer);
    enable_timer = NULL;

    alarm_free(disable_timer);
    disable_timer = NULL;

    if (sBtVendorInterface != NULL) {
        sBtVendorInterface->cleanup();
        sBtVendorInterface = NULL;
    }
}

int Gap:: GetState() {
    if (adapter_properties_obj_ != NULL)
        return adapter_properties_obj_->GetState();
    return  BT_STATE_OFF;
}

bool Gap:: IsDiscovering() {
    return adapter_properties_obj_->IsDiscovering();
}

bool Gap:: IsEnabled() {
    return (adapter_properties_obj_->GetState() == BT_STATE_ON);
}

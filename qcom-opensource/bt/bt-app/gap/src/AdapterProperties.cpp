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

#include <list>
#include <map>
#include <iostream>
#include <algorithm>
#include <string.h>
#include <hardware/bluetooth.h>
#include <hardware/hardware.h>
#include <algorithm>

#include "Gap.hpp"
#include "AdapterProperties.hpp"
#include "osi/include/log.h"

#include "utils.h"

#define LOGTAG "AdapterProperties"


void AdapterProperties:: SetState(AdapterState state) {
    pthread_mutex_lock(&lock_);
    state_ = state;
    pthread_mutex_unlock(&lock_);
}


AdapterProperties :: AdapterProperties(const bt_interface_t *bt_interface,
                                RemoteDevices  *remote_devices_obj) {
    remote_devices_obj_ = remote_devices_obj;
    bluetooth_interface_ = bt_interface;
    state_ = BT_ADAPTER_STATE_OFF;
    pthread_mutex_init(&lock_, NULL);
}


AdapterProperties :: ~AdapterProperties() {
    pthread_mutex_destroy(&lock_);
    bonded_devices.clear();
}


void AdapterProperties::FlushBondedDeviceList() {
    bonded_devices.clear();
}


int AdapterProperties:: GetState() {
    return state_;
}

bt_bdaddr_t *AdapterProperties:: GetBtAddress() {
    return &bt_device_info.bd_addr;
}

bt_bdname_t *AdapterProperties:: GetBtName() {
    return &bt_device_info.bd_name;
}

int AdapterProperties:: SetBtName(bt_property_t *prop) {
    return bluetooth_interface_->set_adapter_property(prop);
}

bool AdapterProperties::IsDeviceBonded(bt_bdaddr_t bd_addr) {
    bdstr_t bd_str;
    bdaddr_to_string(&bd_addr, &bd_str[0], sizeof(bd_str));
    std::string deviceAddress(bd_str);
    std::list<std::string>::iterator bdstring;

    bdstring = std::find(bonded_devices.begin(), bonded_devices.end(), deviceAddress);

    if (bdstring == bonded_devices.end())
        return false;
    else
        return true;
}
bool AdapterProperties:: IsDiscovering() {
    bool discovering = false;
    pthread_mutex_lock(&lock_);
    discovering = discovering_;
    pthread_mutex_unlock(&lock_);
    return discovering;
}

void AdapterProperties::GetBondedDevicesFromPropertyList(int num_properties,
        bt_property_t *properties, bt_property_type_t type,
        bt_bdaddr_t *bd_addr, int *num_bonded_devices) {

    int index;

    for (index = 0; index < num_properties; index++) {
        if (properties[index].type == type) {

            int toCopy = (properties[index].len <
               (*num_bonded_devices *sizeof(bt_bdaddr_t)))
               ? properties[index].len :(*num_bonded_devices * sizeof(bt_bdaddr_t));

            *num_bonded_devices = toCopy/sizeof(bt_bdaddr_t);
            memcpy(bd_addr, properties[index].val, toCopy);
            return;
        }
    }
    *num_bonded_devices = 0;
}

void AdapterProperties::GetCorePropertyList(int num_properties,
        bt_property_t *properties) {

    int index;
    for (index = 0; index < num_properties; index++) {
        if (properties[index].type == BT_PROPERTY_BDADDR) {
            memcpy((bt_bdaddr_t *)&bt_device_info.bd_addr, properties[index].val,
                                properties[index].len);
        } else if ( properties[index].type == BT_PROPERTY_BDNAME) {
            memset (&bt_device_info.bd_name, '\0', sizeof (bt_device_info.bd_name));
            memcpy((bt_bdname_t *)&bt_device_info.bd_name, properties[index].val,
                                properties[index].len);
        }
    }
}

void AdapterProperties :: OnbondStateChanged( bt_bdaddr_t bd_addr,
        bt_bond_state_t new_state, bool notify) {
    bdstr_t bd_str;
    bdaddr_to_string(&bd_addr, &bd_str[0], sizeof(bd_str));
    std::string deviceAddress(bd_str);
    DeviceProperties *remote_device_prop = NULL;
    std::list<std::string>::iterator bdstring;
    BtEvent *bt_event;

    remote_device_prop = remote_devices_obj_->GetDeviceProperties(bd_addr);

    if (!remote_device_prop)
        remote_device_prop = remote_devices_obj_->AddDeviceProperties(bd_addr);

    remote_device_prop->bond_state = new_state;

    bdstring = std::find(bonded_devices.begin(), bonded_devices.end(), deviceAddress);

    switch (new_state) {
        case BT_BOND_STATE_BONDED:
            if (bdstring == bonded_devices.end())
                bonded_devices.push_back (deviceAddress);
            break;
        case BT_BOND_STATE_NONE:
            if (bdstring != bonded_devices.end())
                bonded_devices.remove(deviceAddress);
            break;
    }

    if(notify) {
        bt_event = new BtEvent;
        bt_event->event_id = MAIN_EVENT_BOND_STATE;
        bt_event->bond_state_event.state = new_state;
        memcpy(&bt_event->bond_state_event.bd_addr, &bd_addr,
                                        sizeof(bt_bdaddr_t));
        memcpy(&bt_event->bond_state_event.bd_name, &remote_device_prop->name,
                                       sizeof(bt_bdname_t));
        PostMessage(THREAD_ID_MAIN, bt_event);
    } else {
        // set the broadcast flag to notify once
        // the remote name is available
        remote_device_prop->broadcast = true;
    }
}


void AdapterProperties :: HandleDiscoveryStateChange(bt_discovery_state_t state) {
    if ((state == BT_DISCOVERY_STOPPED) && discovering_) {
        discovering_ = false;
    } else if (state == BT_DISCOVERY_STARTED) {
        discovering_ = true;
    }

    ALOGI (LOGTAG "discovery state changed state %d", state);

    //Sending upadte to the MAIN thread
    BtEvent *bt_event = new BtEvent;
    bt_event->event_id = MAIN_EVENT_INQUIRY_STATUS;
    bt_event->discovery_state_event.state = state;
    PostMessage(THREAD_ID_MAIN, bt_event);
}


void AdapterProperties :: AdapterPropertiesUpdate(AdapterPropertiesEvent *event) {
    bt_bdaddr_t rmt_devices[MAX_BONDED_DEVICES];
    int num_bonded_devices = MAX_BONDED_DEVICES, index;

    GetBondedDevicesFromPropertyList(event->num_properties, event->properties,
            BT_PROPERTY_ADAPTER_BONDED_DEVICES, rmt_devices, &num_bonded_devices);

    if (num_bonded_devices) {
        ALOGI (LOGTAG "Found %d bonded devices", num_bonded_devices);
        /* Fetch the properties of bonded devices */
        for (index = 0; index < num_bonded_devices; index++)
            OnbondStateChanged(rmt_devices[index], BT_BOND_STATE_BONDED, false);
    }

    GetCorePropertyList(event->num_properties, event->properties);
}

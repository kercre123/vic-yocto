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
#include <string.h>
#include "osi/include/log.h"
#include <hardware/bluetooth.h>
#include <hardware/hardware.h>
#include "Gap.hpp"
#include "ipc.h"
#include "utils.h"
#include "RemoteDevices.hpp"

#define LOGTAG "RemoteDevices"


DeviceProperties *RemoteDevices :: AddDeviceProperties(bt_bdaddr_t bd_addr) {
    bdstr_t bd_str;
    bdaddr_to_string(&bd_addr, &bd_str[0], sizeof(bd_str));
    std::string deviceAddress(bd_str);
    DeviceProperties *rem_dev_prop;

    ALOGI (LOGTAG "RemoteDevices: AddDeviceProperties %s", bd_str);
    pthread_mutex_lock(&lock_);
    std::map<std::string, DeviceProperties*>::iterator it;
    it = remote_device_prop.find(deviceAddress);
    if (it != remote_device_prop.end()) {
        pthread_mutex_unlock(&lock_);
        return (rem_dev_prop = it->second);
    } else {
        rem_dev_prop = new DeviceProperties;
        memset(rem_dev_prop, '\0', sizeof(DeviceProperties));
        memcpy(&rem_dev_prop->address, &bd_addr, sizeof(bt_bdaddr_t));
        remote_device_prop[deviceAddress] = rem_dev_prop;
        pthread_mutex_unlock(&lock_);
        return rem_dev_prop;
    }
}


DeviceProperties *RemoteDevices:: GetDeviceProperties(bt_bdaddr_t bd_addr) {
    bdstr_t bd_str;
    bdaddr_to_string(&bd_addr, &bd_str[0], sizeof(bd_str));
    std::string deviceAddress(bd_str);
    DeviceProperties *rem_dev_prop;

    ALOGI (LOGTAG "RemoteDevices: GetDeviceProperties %s", bd_str);

    pthread_mutex_lock(&lock_);
    std::map<std::string, DeviceProperties*>::iterator it;
    it = remote_device_prop.find(deviceAddress);
    if (it != remote_device_prop.end()) {
        pthread_mutex_unlock(&lock_);
        return (rem_dev_prop = it->second);
    } else {
        pthread_mutex_unlock(&lock_);
        return NULL;
    }
}

void RemoteDevices :: HandleAclStateChange(int status, bt_bdaddr_t bd_addr,
        int newState) {
}

bool RemoteDevices::GetValueFromPropertyList(int num_properties,
        bt_property_t *properties, bt_property_type_t type, void* dest) {
    int index;

    for (index = 0; index < num_properties; index++) {
        if (properties[index].type == type) {
            memcpy(dest, properties[index].val, properties[index].len);
            return true;
        }
    }
    return false;
}

void RemoteDevices::ClearPropertyList(int num_properties,
        bt_property_t *properties) {
    int index;

    for (index = 0; index < num_properties; index++) {
        delete properties[index].val;
    }
    delete properties;
}


void RemoteDevices::FlushDiscoveredDeviceList() {
    std::map<std::string, DeviceProperties*>::iterator it;
    DeviceProperties *rem_dev_prop;

    pthread_mutex_lock(&lock_);
    for (it = remote_device_prop.begin(); it != remote_device_prop.end(); ++it) {
        rem_dev_prop = it->second;
        delete rem_dev_prop;
    }
    remote_device_prop.clear();
    pthread_mutex_unlock(&lock_);
}

RemoteDevices:: RemoteDevices(const bt_interface_t *bt_interface) {
    this->bluetooth_interface_ = bt_interface;
    pthread_mutex_init(&this->lock_, NULL);
}

RemoteDevices:: ~RemoteDevices() {
    std::map<std::string, DeviceProperties*>::iterator it;
    DeviceProperties *rem_dev_prop;

    for (it = remote_device_prop.begin(); it != remote_device_prop.end(); ++it) {
        rem_dev_prop = it->second;
        delete rem_dev_prop;
    }
    remote_device_prop.clear();

    pthread_mutex_destroy(&lock_);
}


void RemoteDevices::DeviceFound(DeviceFoundEventInt *dev_found) {
    bt_bdaddr_t bd_addr;
    DeviceProperties *rem_dev_prop;
    BtEvent *bt_event;
    bool ret;

    ret = GetValueFromPropertyList(dev_found->num_properties, dev_found->properties,
            BT_PROPERTY_BDADDR, &bd_addr);
    if (ret == false) {
        ALOGE (LOGTAG "Error finding device address");
        return;
    }

    rem_dev_prop = GetDeviceProperties(bd_addr);
    if (!rem_dev_prop) {
        rem_dev_prop = AddDeviceProperties(bd_addr);
    }

    memset(rem_dev_prop->name, 0, sizeof(rem_dev_prop->name));
    GetValueFromPropertyList(dev_found->num_properties, dev_found->properties,
            BT_PROPERTY_BDADDR, &rem_dev_prop->address);

    GetValueFromPropertyList(dev_found->num_properties, dev_found->properties,
            BT_PROPERTY_CLASS_OF_DEVICE, &rem_dev_prop->bluetooth_class);

    GetValueFromPropertyList(dev_found->num_properties, dev_found->properties,
            BT_PROPERTY_BDNAME, rem_dev_prop->name);

    GetValueFromPropertyList(dev_found->num_properties, dev_found->properties,
            BT_PROPERTY_REMOTE_RSSI, &rem_dev_prop->rssi);

    /* Free the memory used for properties */
    ClearPropertyList(dev_found->num_properties, dev_found->properties);

    /* Update only if remote device Name is available for Now */
    if (strlen(rem_dev_prop->name) > 0) {
        bt_event = new BtEvent;
        bt_event->event_id = MAIN_EVENT_DEVICE_FOUND;
        memcpy(&bt_event->device_found_event.remoteDevice, rem_dev_prop,
                sizeof(DeviceProperties));

        PostMessage(THREAD_ID_MAIN, bt_event);
    }

}

void RemoteDevices::RemoteDeviceProperties(RemotePropertiesEvent *event) {
    DeviceProperties *rem_dev_prop = NULL;
    BtEvent *bt_event;

    rem_dev_prop = GetDeviceProperties (event->bd_addr);
    if (!rem_dev_prop) {
        rem_dev_prop = AddDeviceProperties(event->bd_addr);
    }

    memset(rem_dev_prop->name, 0, sizeof(rem_dev_prop->name));
    GetValueFromPropertyList(event->num_properties, event->properties,
            BT_PROPERTY_BDADDR, &rem_dev_prop->address);

    GetValueFromPropertyList(event->num_properties, event->properties,
            BT_PROPERTY_BDNAME, rem_dev_prop->name);

    if( (rem_dev_prop->name[0] != '\0') &&
            (rem_dev_prop->bond_state == BT_BOND_STATE_BONDED) &&
            (rem_dev_prop->broadcast == true)) {
        bt_event = new BtEvent;
        bt_event->event_id = MAIN_EVENT_BOND_STATE;
        bt_event->bond_state_event.state = BT_BOND_STATE_BONDED;
        memcpy(&bt_event->bond_state_event.bd_addr, &rem_dev_prop->address,
                                    sizeof(bt_bdaddr_t));
        memcpy(&bt_event->bond_state_event.bd_name, &rem_dev_prop->name,
                                   sizeof(bt_bdname_t));
        PostMessage(THREAD_ID_MAIN, bt_event);
        rem_dev_prop->broadcast = false;
    }

    GetValueFromPropertyList(event->num_properties, event->properties,
            BT_PROPERTY_CLASS_OF_DEVICE, &rem_dev_prop->bluetooth_class);

    GetValueFromPropertyList(event->num_properties, event->properties,
            BT_PROPERTY_REMOTE_RSSI, &rem_dev_prop->rssi);

    /* Free the memory used for properties */
    ClearPropertyList(event->num_properties, event->properties);
}

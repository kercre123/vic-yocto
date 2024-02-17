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

#ifndef REMOTE_DEV_HPP
#define REMOTE_DEV_HPP

#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "ipc.h"
#include <pthread.h>

/**
 * @file RemoteDevices.hpp
 * @brief Remote Device header file
 */

/**
 * @class RemoteDevices
 *
 * @brief RemoteDevices Class
 */
class RemoteDevices {
    private:
        /**
         *  structure object for standard Bluetooth DM interface
         */
        const bt_interface_t *bluetooth_interface_;
        /**
         * Used to for syncronization accces
         */
        pthread_mutex_t lock_;
        bool GetValueFromPropertyList(int num_properties, bt_property_t *properties,
                     bt_property_type_t type, void* dest);
        void ClearPropertyList(int num_properties, bt_property_t *properties);

    public:
        RemoteDevices(const bt_interface_t *bt_interface);
        ~RemoteDevices();
        /**
         * @brief map to store Device Properties
         *
         * Maintains a list for inquiry results
         */
        std::map <std::string, DeviceProperties*> remote_device_prop;
        /**
         * @brief AddDeviceProperties
         *
         * It will allocates a new structure for the given RemoteDevice if not allocated
         *
         * @param  bt_bdaddr_t bd_addr has remote device bd adrress
         * @return DeviceProperties
         */
        DeviceProperties *AddDeviceProperties(bt_bdaddr_t bd_addr);
        /**
         * @brief GetDeviceProperties
         *
         * It will search for properties for provided remote device adrress in
         * @ref remote_device_prop list. If properties are present for the given bd_addr
         * it will returns @ref DeviceProperties strucre else it will return NULL
         *
         * @param  bt_bdaddr_t bd_addr
         * @return DeviceProperties*
         */
         DeviceProperties *GetDeviceProperties(bt_bdaddr_t bd_addr);

        /**
         * @brief FlushDiscoveredDeviceList
         *
         * It will remove the inquiry results from the @ref remote_device_prop
         *
         * @return none
         */
        void FlushDiscoveredDeviceList(void);
        /**
         * @brief DeviceFound
         *
         * It is used to notify the discovery result to the main application.
         * and sends a @ref MAIN_EVENT_DEVICE_FOUND event to @ref THREAD_ID_MAIN main thread
         *
         * @param  DeviceFoundEventInt dev_found
         * @return none
         */
        void DeviceFound(DeviceFoundEventInt *dev_found);
        /**
         * @brief RemoteDeviceProperties
         *
         * It is used to notify the remote device properties to the main application.
         * this funtion is executed when we got a GAP_EVENT_REMOTE_DEVICE_PROPERTIES
         * event in @ref THREAD_ID_GAP gap thread.
         * Sends a @ref MAIN_EVENT_BOND_STATE event to @ref THREAD_ID_MAIN main thread
         *
         * @param  RemotePropertiesEvent
         * @return none
         */
        void RemoteDeviceProperties(RemotePropertiesEvent *event);
        /**
         * @brief HandleAclStateChange
         *
         * It is used to notify ACL connection state change to the main application
         *
         * @param int status
         * @param bt_bdaddr_t bd_addr
         * @param int new_state
         * @return none
         */
        void HandleAclStateChange(int status, bt_bdaddr_t bd_addr, int new_state);
};

#endif

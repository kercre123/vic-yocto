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

#ifndef A2DP_SINK_APP_H
#define A2DP_SINK_APP_H

#include <map>
#include <list>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_av.h>
#include <pthread.h>

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "osi/include/allocator.h"
#include "osi/include/alarm.h"
#include "ipc.h"
#include "utils.h"
#include "hardware/bt_av_vendor.h"
#include "A2dp_Sink_Streaming.hpp"

using namespace std;
using std::list;
using std::string;

typedef enum {
    DEVICE_STATE_DISCONNECTED = 0,
    DEVICE_STATE_PENDING,
    DEVICE_STATE_CONNECTED,
}A2dpSinkDeviceState;

typedef enum {
    SINK_STATE_NOT_STARTED = 0,
    SINK_STATE_STARTED,
}A2dpSinkState;

typedef struct {
    uint32_t sample_rate;
    uint8_t channel_count;
}A2dpSinkConfig_t;

class A2dp_Device {

  public:
    config_t *config;
    bt_bdaddr_t mDevice;
    bt_bdaddr_t mConnectingDevice;
    bt_bdaddr_t mConnectedDevice;
    A2dpSinkDeviceState mSinkDeviceState;
    A2dpSinkConfig_t av_config;
    btav_codec_config_t dev_codec_config;
    uint16_t dev_codec_type;
    pthread_mutex_t lock;
    bool mAvrcpConnected;
    int mNotificationLabel;
    bool mAbsVolNotificationRequested;
  public:
    A2dp_Device(config_t *config, bt_bdaddr_t dev);
    ~A2dp_Device();
};

class A2dp_Sink {

  private:
    config_t *config;
    const bt_interface_t * bluetooth_interface;
    const btav_interface_t *sBtA2dpSinkInterface;
    A2dpSinkState mSinkState;
    const btav_sink_vendor_interface_t *sBtA2dpSinkVendorInterface;

  public:
    A2dp_Sink(const bt_interface_t *bt_interface, config_t *config);
    ~A2dp_Sink();
    void ProcessEvent(BtEvent* pEvent, list<A2dp_Device>::iterator iter);
    void state_disconnected_handler(BtEvent* pEvent, list<A2dp_Device>::iterator iter);
    void state_pending_handler(BtEvent* pEvent, list<A2dp_Device>::iterator iter);
    void state_connected_handler(BtEvent* pEvent, list<A2dp_Device>::iterator iter);
    void change_state(list<A2dp_Device>::iterator iter, A2dpSinkDeviceState mState);
    char* dump_message(BluetoothEventId event_id);
    pthread_mutex_t lock;
    uint32_t max_a2dp_conn;
    void HandleEnableSink();
    void HandleDisableSink();
    void HandleSinkStreamingDisableDone();
    void ConnectionManager(BtEvent* pEvent, bt_bdaddr_t dev);
    void EventManager(BtEvent* pEvent, bt_bdaddr_t dev);
    bool isConnectionEvent(BluetoothEventId event_id);
    void UpdateSupportedCodecs(uint8_t num_codecs);
    list<A2dp_Device> pA2dpDeviceList;
};

#endif

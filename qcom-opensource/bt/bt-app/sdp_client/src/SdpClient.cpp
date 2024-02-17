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
#include "SdpClient.hpp"
#include "utils.h"

#define LOGTAG "Sdp "

using namespace std;

SdpClient *g_sdpClient = NULL;;
SdpSearchCb mSearchCb;
bool mSearchOngoing;

#ifdef __cplusplus
extern "C"
{
#endif

void BtSdpClientMsgHandler(void *msg)
{
    BtEvent* event = NULL;
    bool status = false;
    if(!msg) {
        ALOGE(LOGTAG "%s: Msg is null, return", __FUNCTION__);
        return;
    }

    event = ( BtEvent *) msg;

    if (event == NULL)
    {
        ALOGE(LOGTAG "%s: event is null", __FUNCTION__);
        return;
    }

    ALOGD(LOGTAG "BtSdpClientMsgHandler event = %d", event->event_id);
    switch(event->event_id) {
        case PROFILE_API_START:
            if (g_sdpClient) {
                status = g_sdpClient->HandleEnableSdpClient();

                BtEvent *start_event = new BtEvent;
                start_event->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
                start_event->profile_start_event.profile_id = PROFILE_ID_SDP_CLIENT;
                start_event->profile_start_event.status = status;
                PostMessage(THREAD_ID_GAP, start_event);
            }
            break;

        case PROFILE_API_STOP:
            if (g_sdpClient) {
                status = g_sdpClient->HandleDisableSdpClient();

                BtEvent *stop_event = new BtEvent;
                stop_event->profile_start_event.event_id = PROFILE_EVENT_STOP_DONE;
                stop_event->profile_start_event.profile_id = PROFILE_ID_SDP_CLIENT;
                stop_event->profile_start_event.status = status;
                PostMessage(THREAD_ID_GAP, stop_event);
            }
            break;

        default:
            if(g_sdpClient) {
               g_sdpClient->ProcessEvent(( BtEvent *) msg);
            }
            break;
    }
    delete event;
}

void sdp_client_search_callback(bt_status_t status, bt_bdaddr_t *addr, uint8_t* uuid,
            int num_records, bluetooth_sdp_record *records)
{
    int i = 0;
    bluetooth_sdp_record* record;

    ALOGD(LOGTAG "%s: Status is: %d, Record count: %d", __FUNCTION__, status, num_records);

    //stoping sdp_search_timer
    if(g_sdpClient) {
        alarm_cancel(g_sdpClient->sdp_search_timer);
    }
    // Ensure we run the loop at least once, to also signal errors if they occur
    for(i = 0; i < num_records || i==0; i++) {
        bool more_results = (i < (num_records - 1 ))? true : false;
        record = &records[i];
        if (record->hdr.service_name_length > 0) {
            ALOGD("%s, ServiceName:  %s", __FUNCTION__, record->hdr.service_name);
        }
        if (mSearchCb)
            mSearchCb(status, addr, uuid, record, more_results);
    }
    mSearchOngoing = false;
    mSearchCb = NULL;
}

static btsdp_callbacks_t sBluetoothSdpClientCallback = {
    sizeof(sBluetoothSdpClientCallback),
    sdp_client_search_callback,
};

#ifdef __cplusplus
}
#endif

SdpClient :: SdpClient(const bt_interface_t *bt_interface, config_t *config)
{
    this->bluetooth_interface = bt_interface;
    this->config = config;
    sdp_search_timer = NULL;
    if( !(sdp_search_timer = alarm_new())) {
        ALOGE(LOGTAG, " unable to create sdp_search_timer");
        return;
    }

}

SdpClient :: ~SdpClient()
{
    alarm_free(g_sdpClient->sdp_search_timer);
    g_sdpClient->sdp_search_timer = NULL;
}

void sdp_search_timer_expired(void *context) {
    ALOGD(LOGTAG, " sdp_search_timer_expired");

    BtEvent *event = new BtEvent;
    event->event_id = SDP_CLIENT_SEARCH_TIMEOUT;
    memcpy(&event->sdp_client_event.bd_addr, (bt_bdaddr_t *)context, sizeof(bt_bdaddr_t));
    PostMessage(THREAD_ID_SDP_CLIENT, event);
}

void SdpClient::ProcessEvent(BtEvent* pEvent)
{
    char str[18];
    ALOGD(LOGTAG "%s: Processing event %d", __FUNCTION__, pEvent->event_id);

    switch(pEvent->event_id) {

        case SDP_CLIENT_SEARCH:
            memcpy(&mDevice, &pEvent->sdp_client_event.bd_addr, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&mDevice, str, 18);
            ALOGD(LOGTAG "initiating SDP search with device %s", str);
            Search(&mDevice, pEvent->sdp_client_event.uuid, pEvent->sdp_client_event.searchCb);
            break;

        case SDP_CLIENT_ADD_RECORD:
            AddRecord(&pEvent->sdp_client_event.record, pEvent->sdp_client_event.addRecordCb);
            break;

        case SDP_CLIENT_REMOVE_RECORD:
            RemoveRecord(pEvent->sdp_client_event.rec_handle,
                pEvent->sdp_client_event.removeRecordCb);
            break;

        case SDP_CLIENT_SEARCH_TIMEOUT:
            if (mSearchCb)
                mSearchCb(BT_STATUS_FAIL, &pEvent->sdp_client_event.bd_addr,
                NULL, NULL, false);
            mSearchOngoing = false;
            mSearchCb = NULL;
            break;

        default:
            ALOGW(LOGTAG "%s: unhandled event: %d", __FUNCTION__, pEvent->event_id);
            break;
    }
}

bool SdpClient :: Search(bt_bdaddr_t *addr, uint8_t *uuid, SdpSearchCb cb)
{
    if (mSearchOngoing) {
        ALOGW(LOGTAG "%s: active sdp search, returning false", __FUNCTION__);
        if (cb)
            cb(BT_STATUS_FAIL, addr, uuid, NULL, false);
        return false;
    }
    mSearchOngoing = true;
    mSearchCb = cb;
     // start the sdp search timer
    alarm_set(sdp_search_timer, SDP_SEARCH_TIMEOUT_DELAY,
                        sdp_search_timer_expired, addr);
    if (sdp_client_interface)
        sdp_client_interface->sdp_search(addr, uuid);

    /* sdp_search always returns BT_STATUS_SUCCESS, so return true */
    return true;
}

void SdpClient :: AddRecord(bluetooth_sdp_record *record, SdpAddRecordCb cb)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    int record_handle;
    if (sdp_client_interface) {
        status = sdp_client_interface->create_sdp_record(record, &record_handle);
        if (status != BT_STATUS_SUCCESS) {
            ALOGE(LOGTAG "%s create_sdp_record failed %d", __FUNCTION__, status);
        } else {
            ALOGV(LOGTAG "%s create_sdp_record succeded, handle %d",
                __FUNCTION__, record_handle);
        }
        if (cb)
            cb(status, record_handle);
    }
}

void SdpClient :: RemoveRecord(int record_handle, SdpRemoveRecordCb cb)
{
    bt_status_t status = BT_STATUS_SUCCESS;
    if (sdp_client_interface) {
        status = sdp_client_interface->remove_sdp_record(record_handle);
        if (status != BT_STATUS_SUCCESS) {
            ALOGE(LOGTAG "%s remove_sdp_record failed %d", __FUNCTION__, status);
        }
        if (cb)
            cb(status);
    }
}

bool SdpClient :: HandleEnableSdpClient() {
    ALOGV(LOGTAG "%s", __FUNCTION__);

    if ((sdp_client_interface = (btsdp_interface_t *)
            bluetooth_interface->get_profile_interface(BT_PROFILE_SDP_CLIENT_ID)) == NULL) {
        ALOGE(LOGTAG "%s: Failed to get Bluetooth Sdp Client Interface", __FUNCTION__);
        return false;
    }

    bt_status_t status;
    if ((status = sdp_client_interface->init(&sBluetoothSdpClientCallback)) != BT_STATUS_SUCCESS) {
        ALOGE(LOGTAG "%s: Failed to initialize Bluetooth SDP Client, status: %d",
            __FUNCTION__, status);
        sdp_client_interface = NULL;
        return false;
    }
    return true;
}

bool SdpClient :: HandleDisableSdpClient()
{
    ALOGV(LOGTAG "%s", __FUNCTION__);

    bt_status_t status;
    if (sdp_client_interface != NULL) {
        ALOGE(LOGTAG "%s: Cleaning up Bluetooth Sdp Interface...", __FUNCTION__);
        if ((status = sdp_client_interface->deinit()) != BT_STATUS_SUCCESS) {
            ALOGE(LOGTAG "%s: Failed to de-initialize Bluetooth SDP Client, status: %d",
                __FUNCTION__, status);
            sdp_client_interface = NULL;
            return false;
        }
        sdp_client_interface = NULL;
    }
    return true;
}



 /*
  * Copyright (c) 2016, The Linux Foundation. All rights reserved.
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

#ifndef A2DP_SOURCE_APP_H
#define A2DP_SOURCE_APP_H

#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_av.h>
#include <hardware/bt_rc.h>
#include <pthread.h>
#include "hardware/bt_av_vendor.h"
#include "hardware/bt_rc_vendor.h"

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "osi/include/allocator.h"
#include "osi/include/alarm.h"
#include "ipc.h"
#include "utils.h"
#include <list>

using std::list;
using std::string;

#define A2DP_SOURCE_SET_ABS_VOL_TIMER_DURATION         4000

typedef enum {
    AVRC_RSP_NOT_IMPL = 8,
    AVRC_RSP_ACCEPT,
    AVRC_RSP_REJ,
    AVRC_RSP_IN_TRANS,
    AVRC_RSP_IMPL_STBL,
    AVRC_RSP_CHANGED,
    AVRC_RSP_INTERIM = 15,
}AvrcRspType;

typedef enum {
    AVRC_KEY_DOWN = 0,
    AVRC_KEY_UP,
}AvrcKeyDir;

typedef enum {
    ATTR_TRACK_NUM = 0,
    ATTR_TITLE,
    ATTR_ARTIST_NAME,
    ATTR_ALBUM_NAME,
    ATTR_MEDIA_NUMBER,
    ATTR_MEDIA_TOTAL_NUMBER,
    ATTR_GENRE,
    ATTR_PLAYING_TIME_MS,
}AttrType;

typedef enum {
    STATE_A2DP_SOURCE_NOT_STARTED = 0,
    STATE_A2DP_SOURCE_DISCONNECTED,
    STATE_A2DP_SOURCE_PENDING,
    STATE_A2DP_SOURCE_CONNECTED,
}A2dpSourceState;


class MediaPlayerInfo {
  public:
    short mPlayerId;
    char mMajorPlayerType;
    int mPlayerSubType;
    char mPlayState;
    short mCharsetId;
    short mDisplayableNameLength;
    char* mDisplayableName;
    char* mPlayerPackageName;
    bool mIsAvailable;
    bool mIsFocussed;
    char mItemType;
    bool mIsRemoteAddressable;
    short mItemLength;
    short mEntryLength;
    char mFeatureMask[16];


  public:
    MediaPlayerInfo(short playerId, char majorPlayerType, int playerSubType, char playState,
                        short charsetId, short displayableNameLength, char* displayableName,
                        char* playerPackageName, bool isAvailable, bool isFocussed, char itemType,
                        bool isRemoteAddressable, short itemLength, short entryLength,
                        char featureMask[]);
    int RetrievePlayerEntryLength();
    char* RetrievePlayerItemEntry();
    ~MediaPlayerInfo();
};

typedef struct  {
    btrc_media_attr_t *p_attr;
    long mUid;
    int mSize;
}ItemAttr;

typedef struct  {
    uint32_t mStart;
    uint32_t mEnd;
    uint32_t mSize;
    uint8_t mNumAttr;
    uint32_t p_attr[BTRC_MAX_ELEM_ATTR_SIZE];
}FolderListEntries;

class A2dp_Source {

  private:
    config_t *config;
    const bt_interface_t * bluetooth_interface;
    const btav_interface_t *sBtA2dpSourceInterface;
    const btrc_interface_t *sBtAvrcpTargetInterface;
    A2dpSourceState mSourceState;
    bool mAvrcpConnected;
    const btav_vendor_interface_t *sBtA2dpSourceVendorInterface;
    const btrc_vendor_interface_t *sBtAvrcpTargetVendorInterface;

  public:
    A2dp_Source(const bt_interface_t *bt_interface, config_t *config);
    ~A2dp_Source();
    void ProcessEvent(BtEvent* pEvent);
    void state_disconnected_handler(BtEvent* pEvent);
    void state_pending_handler(BtEvent* pEvent);
    void state_connected_handler(BtEvent* pEvent);
    void change_state(A2dpSourceState mState);
    A2dpSourceState get_state(void);
    bool get_codec_cfg(uint8_t* info, uint8_t* type);
    char* dump_message(BluetoothEventId event_id);
    pthread_mutex_t lock;
    bool enable_delay_report;
    bt_bdaddr_t mConnectingDevice;
    bt_bdaddr_t mConnectedDevice;
    bt_bdaddr_t mConnectedAvrcpDevice;
    bool mVolCmdSetInProgress;
    bool mVolCmdAdjustInProgress;
    bool mAbsVolRemoteSupported;
    int mInitialRemoteVolume;
    int mLastRemoteVolume;
    int mRemoteVolume;
    int mLastLocalVolume;
    int mLocalVolume;
    alarm_t *set_play_postion_timer;
    alarm_t *set_abs_volume_timer;
    bool abs_vol_timer;
    bool play_pos_timer;
    uint32_t play_position_interval;
    uint16_t mPreviousAddrPlayerId;
    uint16_t mCurrentAddrPlayerId;
    uint32_t get_a2dp_sbc_sampling_rate(uint8_t frequency);
    char * get_a2dp_sbc_channel_mode(uint8_t channel_count);
    uint8_t get_a2dp_sbc_block_len(uint8_t blocklen);
    uint8_t get_a2dp_sbc_sub_band(uint8_t subband);
    char * get_a2dp_sbc_allocation_mth(uint8_t allocation);
    uint32_t get_a2dp_aptx_sampling_rate(uint8_t frequency);
    char * get_a2dp_aptx_channel_mode(uint8_t channel_count);
    void HandleAvrcpEvents(BtEvent* pEvent);
    void HandleEnableSource();
    void HandleDisableSource();
    void StartSetAbsVolTimer();
    void StopSetAbsVolTimer();
    void StartPlayPostionTimer();
    void StopPlayPostionTimer();
    void SendAppSettingChange();
    void updateResetNotification(btrc_event_id_t noti);
    void UpdateSupportedCodecs(uint8_t num_codecs);
    list<MediaPlayerInfo> pMediaPlayerList;
};

#endif

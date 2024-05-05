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

#include <iostream>
#include <string.h>
#include <hardware/bluetooth.h>
#if (defined(BT_AUDIO_HAL_INTEGRATION))
#include <hardware/hardware.h>
#include <hardware/audio.h>
#endif
#include <hardware/bt_av.h>
#include <hardware/bt_rc.h>
#include <list>
#include <map>
#include <dlfcn.h>
#include "Avrcp.hpp"
#include "A2dp_Src.hpp"
#include "Gap.hpp"
#include "hardware/bt_av_vendor.h"
#include "hardware/bt_rc_vendor.h"
#include <math.h>
#include <algorithm>
#include <cutils/properties.h>
#include "osi/include/list.h"
#include "osi/include/allocator.h"
#include "oi_utils.h"

#define LOGTAG_A2DP "A2DP_SRC "
#define LOGTAG_AVRCP "AVRCP_TG "

using namespace std;
using std::list;
using std::string;

extern Avrcp *pAvrcp;
A2dp_Source *pA2dpSource = NULL;
static pthread_t playback_thread = NULL;
AttrType mAttrType;
bool media_playing = false;
bool use_bigger_metadata = false;
btrc_play_status_t playStatus = BTRC_PLAYSTATE_ERROR;
btrc_notification_type_t mPlayStatusNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
btrc_notification_type_t mTrackChangeNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
btrc_notification_type_t mAddrPlayerChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
btrc_notification_type_t mAvailPlayerChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
btrc_notification_type_t mPlayPosChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
btrc_notification_type_t mAppSettingChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;


static int ATTRIBUTE_NOTSUPPORTED = -1;

static int ATTRIBUTE_EQUALIZER = 1;
static int ATTRIBUTE_REPEATMODE = 2;
static int ATTRIBUTE_SHUFFLEMODE = 3;
static int ATTRIBUTE_SCANMODE = 4;
static int NUMPLAYER_ATTRIBUTE = 4;

uint8_t default_eq_value = BTRC_PLAYER_VAL_OFF_EQUALIZER;
uint8_t default_repeat_value = BTRC_PLAYER_VAL_OFF_REPEAT;
uint8_t default_shuffle_value = BTRC_PLAYER_VAL_OFF_SHUFFLE;
uint8_t default_scan_value = BTRC_PLAYER_VAL_OFF_SCAN;

uint32_t a2dp_play_position = 10;
static uint32_t a2dp_playstatus = A2DP_SOURCE_AUDIO_STOPPED;
long NO_TRACK_SELECTED = -1L;
long TRACK_IS_SELECTED = 0L;
long mCurrentTrackID = NO_TRACK_SELECTED;

int mCurrentEqualizer = default_eq_value;
int mCurrentRepeat = default_repeat_value;
int mCurrentShuffle = default_shuffle_value;
int mCurrentScan = default_scan_value;


#define AVRCP_MAX_VOL 127
int mAudioStreamMax = 15;
bool is_sink_relay_enabled = false;


#if (defined(BT_AUDIO_HAL_INTEGRATION))
audio_hw_device_t *a2dp_device = NULL;
struct audio_stream_out *output_stream = NULL;
static pthread_mutex_t a2dp_hal_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

typedef enum
{
    SRC_STREAMING,
    SRC_NO_STREAMING,
}SrcStreamStatus;

#define AUDIO_STREAM_OUTPUT_BUFFER_SZ      (20*512)
#define INVALID_CODEC    -1
#define NON_A2DP_MEDIA_CT    0xFF
#define DEBUGPRINTBIT
#ifdef DEBUGPRINTBIT
#define PRINTBIT(s,num)   do{ ALOGD("IN Function %s The content of %s:",__func__,#s);\
                              for(int i=0;i<num;i++) ALOGD(" %hhu",*((uint8_t*)(s)+i));}while(0)
#else
#define PRINTBIT(s,num)
#endif

typedef struct
{
    uint16_t codec_type;
    uint16_t len;
    uint16_t offset;
} t_SINK_RELAY_DATA;
list_t *a2dp_sink_relay_data_list;
static pthread_mutex_t a2dp_sink_relay_mutex = PTHREAD_MUTEX_INITIALIZER;
extern bool GetCodecInfoByAddr(bt_bdaddr_t* bd_addr, uint16_t *dev_codec_type, btav_codec_config_t* codec_config);
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ARRAYSIZE
#define _ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

/**
 * Maximum argument length
 */
#define COMMAND_ARG_SIZE     200

static btav_codec_configuration_t a2dpSrcCodecList[MAX_NUM_CODEC_CONFIGS];
#define SBC_MIN_BITPOOL      2
#define SBC_MAX_BITPOOL      250
#define SBC_PARAM_LEN 7
#define APTX_PARAM_LEN 2

static const char * valid_codecs[] = {
    "sbc",
    "aptx"
};

static uint8_t valid_codec_values[] = {
    A2DP_SOURCE_AUDIO_CODEC_SBC,
    A2DP_SOURCE_AUDIO_CODEC_APTX,
};

static const char * valid_sbc_freq[] = {
    "16",
    "32",
    "44.1",
    "48",
};

static uint8_t valid_sbc_freq_values[] = {
    SBC_SAMP_FREQ_16,
    SBC_SAMP_FREQ_32,
    SBC_SAMP_FREQ_44,
    SBC_SAMP_FREQ_48,
};

static const char * valid_sbc_channels[] = {
    "mono",
    "dual",
    "stereo",
    "joint",
};

static uint8_t valid_sbc_channels_values[] = {
    SBC_CH_MONO,
    SBC_CH_DUAL,
    SBC_CH_STEREO,
    SBC_CH_JOINT,
};

static const char * valid_sbc_blocks[] = {
    "4",
    "8",
    "12",
    "16",
};

static uint8_t valid_sbc_blocks_values[] = {
    SBC_BLOCKS_4,
    SBC_BLOCKS_8,
    SBC_BLOCKS_12,
    SBC_BLOCKS_16,
};

static const char * valid_sbc_subbands[] = {
    "4",
    "8",
};

static uint8_t valid_sbc_subbands_values[] = {
    SBC_SUBBAND_4,
    SBC_SUBBAND_8,
};

static const char * valid_sbc_allocation[] = {
    "snr",
    "loud",
};

static uint8_t valid_sbc_allocation_values[] = {
    SBC_ALLOC_SNR,
    SBC_ALLOC_LOUDNESS,
};

static const char * valid_aptx_freq[] = {
    "44.1",
    "48",
};

static uint8_t valid_aptx_freq_values[] = {
    APTX_SAMPLERATE_44100,
    APTX_SAMPLERATE_48000,
};

static const char * valid_aptx_channels[] = {
    "mono",
    "stereo",
};

static uint8_t valid_aptx_channels_values[] = {
    APTX_CHANNELS_MONO,
    APTX_CHANNELS_STEREO,
};

static const char * valid_sbc_bitpool[] = {
    "2 - 250",
};


/******************************************************************************
 * This structure defines the A2DP Sink variable.
 */
typedef struct {
    const char *name;            /**< Run-time variable name */
    const char *description;     /**< Run-time variable description */
    const char **valid_options;  /**< List of valid variable values */
    int valid_option_cnt;        /**< Size of valid value list */
} A2DP_SRC_VARIABLE;

/******************************************************************************
 * List of A2DP Sink variables.
 */
const A2DP_SRC_VARIABLE variable_list[] = {
    { "codec type", "Valid Codec Type to Use",
      valid_codecs, _ARRAYSIZE(valid_codecs) },
    { "sbc freq", "Valid SBC Freq to Use",
      valid_sbc_freq, _ARRAYSIZE(valid_sbc_freq) },
    { "aptx freq", "Valid APTX Freq to Use",
      valid_aptx_freq, _ARRAYSIZE(valid_aptx_freq) },
    { "sbc channels", "Valid SBC Channels to Use",
      valid_sbc_channels, _ARRAYSIZE(valid_sbc_channels) },
    { "sbc blocks", "Valid SBC Blocks to Use",
      valid_sbc_blocks, _ARRAYSIZE(valid_sbc_blocks) },
    { "sbc subbands", "Valid SBC Subbands to Use",
      valid_sbc_subbands, _ARRAYSIZE(valid_sbc_subbands) },
    { "sbc allocation", "Valid SBC Allocation to Use",
      valid_sbc_allocation, _ARRAYSIZE(valid_sbc_allocation) },
    { "aptx channels", "Valid APTX Channel to Use",
      valid_aptx_channels, _ARRAYSIZE(valid_aptx_channels) },
    { "sbc bitpool", "Valid SBC Bitpool to Use",
      valid_sbc_bitpool, _ARRAYSIZE(valid_sbc_bitpool) },
};

/******************************************************************************
 *
 * Basic utilities.
 *
 */

#ifndef isdelimiter
#define isdelimiter(c) ((c) == ' ' || (c) == ',' || (c) == '\f' || (c) == '\n' || \
        (c) == '\r' || (c) == '\t' || (c) == '\v')
#endif

static int find_str_in_list(const char *str, const char * const *list,
                                   int list_size)
{
    int i;
    int item = list_size;
    int match_cnt = 0;
    int len;

    if (str == NULL || list == NULL || list_size <= 0)
        return -1;


    for (i = 0; i < list_size; i++) {
        if (!OI_StrcmpInsensitive(list[i], str)) {
            item = i;
            match_cnt++;
        }
    }

    if (match_cnt == 1) {
        return item;
    } else {
        return list_size;
    }
}

static void print_help(const A2DP_SRC_VARIABLE *var)
{
    int i;
    if (var) {
        printf("\n=====HELP=====\n%s:\t%s\n(valid options:", var->name, var->description);
        for (i = 0; i < (uint8_t)var->valid_option_cnt; i++) {
            printf(" %s", var->valid_options[i]);
        }
        printf(")\n");
    }
}

static const char * skip_delimiter(const char *data)
{
    if (data == NULL)
        return NULL;

    while (*data && isdelimiter(*data)) {
        data++;
    }
    return data;
}

static int ParseUserInput (char *input, char output[][COMMAND_ARG_SIZE]) {
    char *temp_arg = NULL;
    char delim[] = ",";
    char *ptr1;
    int param_count = 0;
    bool status = false;

    if (input == NULL || output == NULL)
        return 0;

    if ((temp_arg = strtok_r(input, delim, &ptr1)) != NULL ) {
        strlcpy(output[param_count], temp_arg, COMMAND_ARG_SIZE);
        output[param_count ++][COMMAND_ARG_SIZE - 1] = '\0';
        ALOGE(LOGTAG_A2DP " %s ", output[param_count -1]);
    }

    while ((temp_arg = strtok_r(NULL, delim, &ptr1))) {
        strlcpy(output[param_count], temp_arg, COMMAND_ARG_SIZE);
        output[param_count ++][COMMAND_ARG_SIZE - 1] = '\0';
        ALOGE(LOGTAG_A2DP " %s ", output[param_count -1]);
    }

    ALOGE(LOGTAG_A2DP " %s: returning %d \n", __func__, param_count);
    return param_count;
}

/* This function is used for testing purpose. Parses string which represents codec list*/

static bool A2dpCodecList(char *codec_param_list, int *num_codec_configs)
{
    int i = 0, j = 0, k = 0;
    char output_list[COMMAND_ARG_SIZE][COMMAND_ARG_SIZE];
    int codec_params_list_size;

    if (*codec_param_list == '\0') {
        ALOGE(LOGTAG_A2DP " codec list cannot be set to nothing \n");
        fprintf(stdout, "codec list cannot be set to nothing \n");
        print_help(&variable_list[0]);
        return false;
    }
    fprintf(stdout, "Codec List: %s\n", codec_param_list);

    codec_params_list_size = ParseUserInput(codec_param_list, output_list);

    while (j < codec_params_list_size) {
        i = find_str_in_list(output_list[j], valid_codecs, _ARRAYSIZE(valid_codecs));
        if (i >= _ARRAYSIZE(valid_codecs)) {
            fprintf(stdout, "Invalid codec type values: %s\n", output_list[j]);
            print_help(&variable_list[0]);
            return false;
        }
        j++;
        a2dpSrcCodecList[k].codec_type = valid_codec_values[i];
        switch (a2dpSrcCodecList[k].codec_type) {
            case A2DP_SOURCE_AUDIO_CODEC_SBC:
                /* check number of parameters passed are ok or not */
                if (j + SBC_PARAM_LEN > codec_params_list_size) {
                    fprintf(stdout, "Invalid SBC Parameters passed\n");
                    return false;
                }
                i = find_str_in_list(output_list[j], valid_sbc_freq,
                    _ARRAYSIZE(valid_sbc_freq));
                if (i >= _ARRAYSIZE(valid_sbc_freq)) {
                    fprintf(stdout, "Invalid SBC Sampling Freq: %s\n", output_list[j]);
                    print_help(&variable_list[1]);
                    return false;
                }
                a2dpSrcCodecList[k].codec_config.sbc_config.samp_freq =
                    valid_sbc_freq_values[i];
                j++;
                i = find_str_in_list(output_list[j], valid_sbc_channels,
                    _ARRAYSIZE(valid_sbc_channels));
                if (i >= _ARRAYSIZE(valid_sbc_channels)) {
                    fprintf(stdout, "Invalid SBC Channels: %s\n", output_list[j]);
                    print_help(&variable_list[3]);
                    return false;
                }
                a2dpSrcCodecList[k].codec_config.sbc_config.ch_mode =
                    valid_sbc_channels_values[i];
                j++;
                i = find_str_in_list(output_list[j], valid_sbc_blocks,
                    _ARRAYSIZE(valid_sbc_blocks));
                if (i >= _ARRAYSIZE(valid_sbc_blocks)) {
                    fprintf(stdout, "Invalid SBC Blocks: %s\n", output_list[j]);
                    print_help(&variable_list[4]);
                    return false;
                }
                a2dpSrcCodecList[k].codec_config.sbc_config.block_len =
                    valid_sbc_blocks_values[i];
                j++;
                i = find_str_in_list(output_list[j], valid_sbc_subbands,
                    _ARRAYSIZE(valid_sbc_subbands));
                if (i >= _ARRAYSIZE(valid_sbc_subbands)) {
                    fprintf(stdout, "Invalid SBC Sub bands: %s\n", output_list[j]);
                    print_help(&variable_list[5]);
                    return false;
                }
                a2dpSrcCodecList[k].codec_config.sbc_config.num_subbands =
                    valid_sbc_subbands_values[i];
                j++;
                i = find_str_in_list(output_list[j], valid_sbc_allocation,
                    _ARRAYSIZE(valid_sbc_allocation));
                if (i >= _ARRAYSIZE(valid_sbc_allocation)) {
                    fprintf(stdout, "Invalid SBC Allocation Mode: %s\n",
                        output_list[j]);
                    print_help(&variable_list[6]);
                    return false;
                }
                a2dpSrcCodecList[k].codec_config.sbc_config.alloc_mthd =
                    valid_sbc_allocation_values[i];
                j++;
                a2dpSrcCodecList[k].codec_config.sbc_config.max_bitpool =
                    atoi (output_list[j++]);
                ALOGD(LOGTAG_A2DP "Max Bitool %d",
                    a2dpSrcCodecList[k].codec_config.sbc_config.max_bitpool);
                if (a2dpSrcCodecList[k].codec_config.sbc_config.max_bitpool <
                    SBC_MIN_BITPOOL ||
                    a2dpSrcCodecList[k].codec_config.sbc_config.max_bitpool >
                    SBC_MAX_BITPOOL) {
                    fprintf(stdout, "Invalid SBC Max bitpool %s\n",
                        output_list[j - 1]);
                    print_help(&variable_list[8]);
                    return false;
                }
                a2dpSrcCodecList[k].codec_config.sbc_config.min_bitpool =
                    atoi (output_list[j++]);
                ALOGD(LOGTAG_A2DP "Min Bitool %d",
                    a2dpSrcCodecList[k].codec_config.sbc_config.min_bitpool);
                if (a2dpSrcCodecList[k].codec_config.sbc_config.min_bitpool <
                    SBC_MIN_BITPOOL ||
                    a2dpSrcCodecList[k].codec_config.sbc_config.min_bitpool >
                    SBC_MAX_BITPOOL) {
                    fprintf(stdout, "Invalid SBC Min bitpool %s\n",
                        output_list[j - 1]);
                    print_help(&variable_list[8]);
                    return false;
                }
                break;
            case A2DP_SOURCE_AUDIO_CODEC_APTX:
                /* check number of parameters passed are ok or not */
                if (j + APTX_PARAM_LEN > codec_params_list_size) {
                    fprintf(stdout, "Invalid APTX Parameters passed\n");
                    return false;
                }
                i = find_str_in_list(output_list[j], valid_aptx_freq,
                    _ARRAYSIZE(valid_aptx_freq));
                if (i >= _ARRAYSIZE(valid_aptx_freq)) {
                    fprintf(stdout, "Invalid APTX Sampling Freq: %s\n",
                        output_list[j]);
                    print_help(&variable_list[2]);
                    return false;
                }
                a2dpSrcCodecList[k].codec_config.aptx_config.sampling_freq =
                    valid_aptx_freq_values[i];
                j++;
                i = find_str_in_list(output_list[j], valid_aptx_channels,
                    _ARRAYSIZE(valid_aptx_channels));
                if (i >= _ARRAYSIZE(valid_aptx_channels)) {
                    fprintf(stdout, "Invalid APTX Channel Mode: %s\n",
                        output_list[j]);
                    print_help(&variable_list[7]);
                    return false;
                }
                a2dpSrcCodecList[k].codec_config.aptx_config.channel_count =
                    valid_aptx_channels_values[i];
                j++;
                break;
        }
        k++;
        if (k >= MAX_NUM_CODEC_CONFIGS) {
            fprintf(stdout, "num_codec_configs  exceeds max number(%d) = %d\n",
                k, MAX_NUM_CODEC_CONFIGS);
            return false;
        }
    }
    *num_codec_configs = k;
    fprintf(stdout, "num_codec_configs  = %d\n", *num_codec_configs);
    return true;
}

void registerMediaPlayers () {
    ALOGD(LOGTAG_AVRCP "registerMediaPlayers");

    char* playerName1 = "Music";/*Music*/;
    char* playerName2 = "Music2";/*Music2*/;

    char featureMasks[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    char featureMasks2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    featureMasks[5] = featureMasks[5] | 0x01;
    featureMasks[5] = featureMasks[5] | 0x04;
    featureMasks[5] = featureMasks[5] | 0x02;
    featureMasks[7] = featureMasks[7] | 0x04;

    featureMasks2[5] = featureMasks2[5] | 0x01;
    featureMasks2[5] = featureMasks2[5] | 0x04;
    featureMasks2[5] = featureMasks2[5] | 0x02;
    featureMasks2[7] = featureMasks2[7] | 0x04;

    pA2dpSource->pMediaPlayerList.push_back(MediaPlayerInfo (0, 1, 0, 2, 0x006A, 5,
            playerName1, "com.default.music", false, false, 0x01, true, 0, 0, featureMasks));

    pA2dpSource->pMediaPlayerList.push_back(MediaPlayerInfo (1, 1, 0, 2, 0x006A, 6,
            playerName2, "com.external.music", false, false, 0x01, true, 0, 0, featureMasks2));

    ALOGD(LOGTAG_AVRCP "Exit registerMediaPlayers()");
}

void A2dp_Source:: updateResetNotification(btrc_event_id_t noti) {
    ALOGD(LOGTAG_AVRCP "updateResetNotification for %d", noti);
    btrc_register_notification_t param;
    long TrackNumberRsp = -1L;
    switch(noti)
    {
        case BTRC_EVT_PLAY_STATUS_CHANGED:
            if (mPlayStatusNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                mPlayStatusNotiType = BTRC_NOTIFICATION_TYPE_REJECT;
                param.play_status = BTRC_PLAYSTATE_PAUSED;
                sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_PLAY_STATUS_CHANGED,
                        mPlayStatusNotiType, &param, &pA2dpSource->mConnectedAvrcpDevice);
            }
            break;
        case BTRC_EVT_TRACK_CHANGE:
            if (mTrackChangeNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                mTrackChangeNotiType = BTRC_NOTIFICATION_TYPE_REJECT;
                mCurrentTrackID = TRACK_IS_SELECTED;
                TrackNumberRsp = mCurrentTrackID;
                ALOGD(LOGTAG_AVRCP " TrackNumberRsp = %l", TrackNumberRsp);
                for (int i = 0; i < 8; ++i) {
                    param.track[i] = (uint8_t) (TrackNumberRsp >> (56 - 8 * i));
                }
                sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_TRACK_CHANGE,
                        mTrackChangeNotiType, &param, &pA2dpSource->mConnectedAvrcpDevice);
            }
            break;
        case BTRC_EVT_PLAY_POS_CHANGED:
            if (mPlayPosChangedNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                mPlayPosChangedNotiType = BTRC_NOTIFICATION_TYPE_REJECT;
                param.song_pos = -1;
                sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_PLAY_POS_CHANGED,
                                mPlayPosChangedNotiType, &param, &pA2dpSource->mConnectedAvrcpDevice);
            }
        default:
            ALOGD(LOGTAG_AVRCP "Invalid Noti");
            break;
    }
}

void resetAndSendPlayerStatusReject() {
    ALOGD(LOGTAG_A2DP "resetAndSendPlayerStatusReject");
    pA2dpSource->updateResetNotification(BTRC_EVT_PLAY_STATUS_CHANGED);
    pA2dpSource->updateResetNotification(BTRC_EVT_TRACK_CHANGE);
    pA2dpSource->updateResetNotification(BTRC_EVT_PLAY_POS_CHANGED);
}

void BtA2dpSourceMsgHandler(void *msg) {
    BtEvent* pEvent = NULL;
    int num_codec_cfgs = 0;
    if(!msg) {
        ALOGE("Msg is NULL, bail out!!");
        return;
    }

    pEvent = ( BtEvent *) msg;
    switch(pEvent->event_id) {
        case PROFILE_API_START:
            ALOGD(LOGTAG_A2DP "enable a2dp source");
            if (pA2dpSource) {
                pA2dpSource->HandleEnableSource();
            }
            break;
        case PROFILE_API_STOP:
            ALOGD(LOGTAG_A2DP "disable a2dp source");
            if (pA2dpSource) {
                pA2dpSource->HandleDisableSource();
            }
            break;
        case AVRCP_TARGET_CONNECTED_CB:
        case AVRCP_TARGET_DISCONNECTED_CB:
        case A2DP_SOURCE_AUDIO_CMD_REQ:
        case AVRCP_TARGET_GET_ELE_ATTR:
        case AVRCP_TARGET_GET_PLAY_STATUS:
        case AVRCP_TARGET_REG_NOTI:
        case AVRCP_TARGET_TRACK_CHANGED:
        case AVRCP_TARGET_VOLUME_CHANGED:
        case AVRCP_TARGET_ADDR_PLAYER_CHANGED:
        case AVRCP_TARGET_AVAIL_PLAYER_CHANGED:
        case AVRCP_TARGET_SET_ABS_VOL:
        case AVRCP_TARGET_ABS_VOL_TIMEOUT:
        case AVRCP_TARGET_SEND_VOL_UP_DOWN:
        case AVRCP_TARGET_GET_FOLDER_ITEMS_CB:
        case AVRCP_TARGET_SET_ADDR_PLAYER_CB:
        case AVRCP_TARGET_USE_BIGGER_METADATA:
        case AVRCP_TARGET_LIST_PLAYER_APP_ATTR:
        case AVRCP_TARGET_LIST_PLAYER_APP_VALUES:
        case AVRCP_TARGET_GET_PLAYER_APP_VALUE:
        case AVRCP_TARGET_SET_PLAYER_APP_VALUE:
        case AVRCP_SET_EQUALIZER_VAL:
        case AVRCP_SET_REPEAT_VAL:
        case AVRCP_SET_SHUFFLE_VAL:
        case AVRCP_SET_SCAN_VAL:
        case AVRCP_TARGET_PLAY_POSITION_TIMEOUT:
            if (pA2dpSource) {
                pA2dpSource->HandleAvrcpEvents(( BtEvent *) msg);
            }
            break;
        case A2DP_SOURCE_CODEC_LIST:
            A2dpCodecList(pEvent->a2dpCodecListEvent.codec_list, &num_codec_cfgs);
            if (num_codec_cfgs)
                pA2dpSource->UpdateSupportedCodecs(num_codec_cfgs);
            break;
        default:
            if(pA2dpSource) {
               pA2dpSource->ProcessEvent(( BtEvent *) msg);
            }
            break;
    }
    delete pEvent;
}

#ifdef __cplusplus
}
#endif

static void BtA2dpLoadA2dpHal() {
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    const hw_module_t *module;
    ALOGD(LOGTAG_A2DP "Load A2dp HAL");
    if (hw_get_module_by_class(AUDIO_HARDWARE_MODULE_ID,
                               AUDIO_HARDWARE_MODULE_ID_A2DP,
                               &module)) {
        ALOGE(LOGTAG_A2DP "A2dp Hal module not found");
        return;
    }
    pthread_mutex_lock(&a2dp_hal_mutex);
    if (audio_hw_device_open(module, &a2dp_device)) {
        a2dp_device = NULL;
        ALOGE(LOGTAG_A2DP "A2dp Hal device can not be opened");
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return;
    }
    pthread_mutex_unlock(&a2dp_hal_mutex);
#endif
    ALOGD(LOGTAG_A2DP "A2dp HAL successfully loaded");
}

static void BtA2dpStopStreaming()
{
    ALOGD(LOGTAG_A2DP "Stop A2dp Streaming");
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    pthread_mutex_lock(&a2dp_hal_mutex);
    if(!output_stream)
    {
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return;
    }
    output_stream->common.set_parameters(&output_stream->common, "A2dpSuspended=false");
    output_stream->common.standby(&output_stream->common);
    pthread_mutex_unlock(&a2dp_hal_mutex);
#endif
    ALOGD(LOGTAG_A2DP "A2dp stream successfully stopped");
}

static void BtA2dpCloseOutputStream()
{
    ALOGD(LOGTAG_A2DP "Close A2dp Output Stream");
    media_playing = false;
    if (playback_thread != NULL)
    {
        pthread_join(playback_thread, NULL);
        playback_thread = NULL;
    }
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    pthread_mutex_lock(&a2dp_hal_mutex);
    if(!a2dp_device)
    {
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return;
    }
    if(!output_stream)
    {
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return;
    }
    a2dp_device->close_output_stream(a2dp_device, output_stream);
    output_stream = NULL;
    pthread_mutex_unlock(&a2dp_hal_mutex);
#endif
    ALOGD(LOGTAG_A2DP "A2dp Output Stream successfully closed");
}

static void BtA2dpUnloadA2dpHal() {
    ALOGD(LOGTAG_A2DP "Unload A2dp HAL");
    BtA2dpCloseOutputStream();
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    pthread_mutex_lock(&a2dp_hal_mutex);
    if(!a2dp_device)
    {
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return;
    }
    if (audio_hw_device_close(a2dp_device) < 0) {
        ALOGE(LOGTAG_A2DP "A2dp HAL could not be closed gracefully");
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return;
    }
    a2dp_device = NULL;
    pthread_mutex_unlock(&a2dp_hal_mutex);
#endif
    ALOGD(LOGTAG_A2DP "A2dp HAL successfully Unloaded");
}

static void BtA2dpOpenOutputStream()
{
    int ret = -1;
    ALOGD(LOGTAG_A2DP "Open A2dp Output Stream");
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    pthread_mutex_lock(&a2dp_hal_mutex);
    if (!a2dp_device) {
        ALOGE(LOGTAG_A2DP "Invalid A2dp HAL device. Bail out!");
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return;
    }
    ret = a2dp_device->open_output_stream(a2dp_device, 0, AUDIO_DEVICE_OUT_ALL_A2DP,
            AUDIO_OUTPUT_FLAG_NONE, NULL, &output_stream, NULL);
    if (ret < 0) {
        output_stream = NULL;
        ALOGE(LOGTAG_A2DP "open output stream returned %d\n", ret);
    }
    pthread_mutex_unlock(&a2dp_hal_mutex);
#endif
    ALOGD(LOGTAG_A2DP "A2dp Output Stream successfully opened");
}

static void BtA2dpSuspendStreaming()
{
    ALOGD(LOGTAG_A2DP "Suspend A2dp Stream");
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    pthread_mutex_lock(&a2dp_hal_mutex);
    if(!output_stream)
    {
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return;
    }
    output_stream->common.set_parameters(&output_stream->common, "A2dpSuspended=true");
    pthread_mutex_unlock(&a2dp_hal_mutex);
#endif
    ALOGD(LOGTAG_A2DP "A2dp Stream suspended successfully");
}

static void BtA2dpResumeStreaming()
{
    ALOGD(LOGTAG_A2DP "Resume A2dp Stream");
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    pthread_mutex_lock(&a2dp_hal_mutex);
    if(!output_stream)
    {
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return;
    }
    output_stream->common.set_parameters(&output_stream->common, "A2dpSuspended=false");
    pthread_mutex_unlock(&a2dp_hal_mutex);
#endif
    ALOGD(LOGTAG_A2DP "A2dp Stream resumed successfully");
}

int get_codec_relay_data(void)
{
    pthread_mutex_lock(&a2dp_sink_relay_mutex);
    if(list_is_empty(a2dp_sink_relay_data_list)) {
        pthread_mutex_unlock(&a2dp_sink_relay_mutex);
        return INVALID_CODEC;
    }
    t_SINK_RELAY_DATA* ptr = (t_SINK_RELAY_DATA*)list_front(a2dp_sink_relay_data_list);
    pthread_mutex_unlock(&a2dp_sink_relay_mutex);
    return ptr->codec_type;
}
void flush_relay_data(void)
{
    if(!a2dp_sink_relay_data_list)
        return;
    ALOGD("flush relay data, list_length = %d",list_length(a2dp_sink_relay_data_list));
    t_SINK_RELAY_DATA* ptr;
    pthread_mutex_lock(&a2dp_sink_relay_mutex);
    while (!list_is_empty(a2dp_sink_relay_data_list))
    {
        ptr = (t_SINK_RELAY_DATA*)list_front(a2dp_sink_relay_data_list);
        list_remove(a2dp_sink_relay_data_list, ptr);
        osi_free(ptr);
    }
    pthread_mutex_unlock(&a2dp_sink_relay_mutex);
    return;
}

void enque_relay_data(uint8_t* buffer, size_t size, uint8_t codec_type)
{
    ALOGD(" enque_relay_data size %d list_len = %d codec=%d", size, list_length(a2dp_sink_relay_data_list),codec_type);
    pthread_mutex_lock(&a2dp_sink_relay_mutex);
    if (list_length(a2dp_sink_relay_data_list) > 10) {
        ALOGE(LOGTAG_A2DP "%s:a2dp sink relay queue is full",__func__);
        pthread_mutex_unlock(&a2dp_sink_relay_mutex);
        return;
    }
    /* allocate memory, first 4 bytes will have size, next 4 bytes will have offset */
    t_SINK_RELAY_DATA* ptr = (t_SINK_RELAY_DATA*)osi_malloc(size + sizeof(t_SINK_RELAY_DATA));
    uint8_t* data_ptr;
    if(ptr != NULL)
    {
        data_ptr  = (uint8_t*)(ptr+1);
        memcpy(data_ptr, (uint8_t*)buffer, size);
        ptr->codec_type = codec_type;// 0 is for SBC
        ptr->offset = 0;
        ptr->len = size;
        ALOGD(" enque data codec = %d, size=%d",codec_type,size);
    }
    else
    {
        ALOGE(LOGTAG_A2DP "%s:can not alloc t_SINK_RELAY_DATA",__func__);
        pthread_mutex_unlock(&a2dp_sink_relay_mutex);
        return;
    }
    list_append(a2dp_sink_relay_data_list, ptr);
    pthread_mutex_unlock(&a2dp_sink_relay_mutex);
}

size_t get_sbc_data(uint8_t* buffer, size_t size)
{
    ALOGD("revise1 get SBC Data size %d list_len = %d", size, list_length(a2dp_sink_relay_data_list));
    uint8_t* start_buf_ptr = buffer;
    uint8_t* end_buf_ptr = buffer + size;
    uint8_t* data_ptr;
    size_t data_len=0;
    pthread_mutex_lock(&a2dp_sink_relay_mutex);
    if(list_is_empty(a2dp_sink_relay_data_list)) {
            pthread_mutex_unlock(&a2dp_sink_relay_mutex);
            return 0;
    }
    t_SINK_RELAY_DATA* ptr = (t_SINK_RELAY_DATA*)list_front(a2dp_sink_relay_data_list);
    //ALOGD("len=%d,offset=%d, end-stat=%d",ptr->len,ptr->offset,end_buf_ptr - start_buf_ptr);
    while((start_buf_ptr < end_buf_ptr) && (!list_is_empty(a2dp_sink_relay_data_list)))
    {
        data_ptr = (uint8_t*)(ptr + 1);
        /* packets in topmost element are more than what is to be written */
        if((ptr->len - ptr->offset) > (end_buf_ptr - start_buf_ptr))
        {
            ALOGD("packet is more than left buffer len=%d, gap=%d",ptr->len,end_buf_ptr - start_buf_ptr);
            break;
            memcpy(start_buf_ptr, data_ptr + ptr->offset, (end_buf_ptr - start_buf_ptr));
            ptr->offset += (end_buf_ptr -  start_buf_ptr);
            start_buf_ptr += (end_buf_ptr -  start_buf_ptr);
        }
        else /* packets in topmost element is lesser than what is required */
        {
            memcpy(start_buf_ptr, data_ptr + ptr->offset, (ptr->len - ptr->offset));
            data_len+=(ptr->len - ptr->offset);
            PRINTBIT(start_buf_ptr,4);
            start_buf_ptr += (ptr->len - ptr->offset);
            ptr->offset += (ptr->len - ptr->offset);
           //ALOGD("ptr->len=%d,ptr->offset=%d, end-stat=%d,data_len=%d",ptr->len,ptr->offset,end_buf_ptr - start_buf_ptr,data_len);
            list_remove(a2dp_sink_relay_data_list, ptr);
            osi_free(ptr);
            if (!list_is_empty(a2dp_sink_relay_data_list)) {
                ptr = (t_SINK_RELAY_DATA*)list_front(a2dp_sink_relay_data_list);
            }
        }
    }
    pthread_mutex_unlock(&a2dp_sink_relay_mutex);
    if(start_buf_ptr == end_buf_ptr)
        return size;
    else
        return data_len;

}

size_t get_pcm_data(uint8_t* buffer, size_t size)
{
    ALOGD(" get PCM Data size %d list_len = %d", size, list_length(a2dp_sink_relay_data_list));
    uint8_t* start_buf_ptr = buffer;
    uint8_t* end_buf_ptr = buffer + size;
    uint8_t* data_ptr;
    pthread_mutex_lock(&a2dp_sink_relay_mutex);
    if(list_is_empty(a2dp_sink_relay_data_list)) {
        pthread_mutex_unlock(&a2dp_sink_relay_mutex);
        return 0;
    }
    t_SINK_RELAY_DATA* ptr = (t_SINK_RELAY_DATA*)list_front(a2dp_sink_relay_data_list);
    if(ptr->codec_type != A2DP_SINK_AUDIO_CODEC_PCM)
    {
        list_remove(a2dp_sink_relay_data_list, ptr);
        osi_free(ptr);
        pthread_mutex_unlock(&a2dp_sink_relay_mutex);
        return 0;
    }

    while((start_buf_ptr < end_buf_ptr) && (!list_is_empty(a2dp_sink_relay_data_list)))
    {
        data_ptr = (uint8_t*)(ptr + 1);
        /* packets in topmost element are more than what is to be written */
        if((ptr->len - ptr->offset) > (end_buf_ptr - start_buf_ptr))
        {
            memcpy(start_buf_ptr, data_ptr + ptr->offset, (end_buf_ptr - start_buf_ptr));
            ptr->offset += (end_buf_ptr -  start_buf_ptr);
            start_buf_ptr += (end_buf_ptr -  start_buf_ptr);
        }
        else /* packets in topmost element is lesser than what is required */
        {
            memcpy(start_buf_ptr, data_ptr + ptr->offset, (ptr->len - ptr->offset));
            start_buf_ptr += (ptr->len - ptr->offset);
            ptr->offset += (ptr->len - ptr->offset);
            list_remove(a2dp_sink_relay_data_list, ptr);
            osi_free(ptr);
            if (!list_is_empty(a2dp_sink_relay_data_list)) {
                ptr = (t_SINK_RELAY_DATA*)list_front(a2dp_sink_relay_data_list);
            }
        }
    }
    pthread_mutex_unlock(&a2dp_sink_relay_mutex);
    if(start_buf_ptr == end_buf_ptr)
        return size;
    else
        return(end_buf_ptr - start_buf_ptr);
}

static void *thread_func(void *in_param)
{
    SrcStreamStatus srcStream = SRC_NO_STREAMING;
    size_t len = 0;
    ssize_t write_len = 0;
    FILE *in_file = (FILE *)in_param;
    size_t out_buffer_size = 0;
    int codec_type;
    short buffer[AUDIO_STREAM_OUTPUT_BUFFER_SZ];
    btav_codec_config_t src_codec_cfg;
    btav_codec_config_t snk_codec_cfg;
    int src_codec_type = A2DP_SINK_AUDIO_CODEC_SBC;
    uint16_t snk_codec_type;
    uint16_t use_file_stream =0;
    uint8_t codecinfo[20];
    uint8_t tmpval;
    ALOGD(LOGTAG_A2DP "Streaming thread started");
#if (defined(BT_AUDIO_HAL_INTEGRATION))
    pthread_mutex_lock(&a2dp_hal_mutex);
    if(!output_stream)
    {
        pthread_mutex_unlock(&a2dp_hal_mutex);
        return NULL;
    }
    out_buffer_size = output_stream->common.get_buffer_size(&output_stream->common);
    pthread_mutex_unlock(&a2dp_hal_mutex);
    if (out_buffer_size <= 0 || out_buffer_size > AUDIO_STREAM_OUTPUT_BUFFER_SZ) {
        ALOGE(LOGTAG_A2DP "Wrong buffer size. Bail out %u!!", out_buffer_size);
       if (in_file) fclose(in_file);
          return NULL;
    }
#endif
    if ( pA2dpSource->get_codec_cfg((uint8_t *)codecinfo,&tmpval))
    {
        ALOGD("Got codec type = %d",tmpval);
        src_codec_type = (int) tmpval;
        if( tmpval == A2DP_SINK_AUDIO_CODEC_SBC)
        {
            memcpy(&src_codec_cfg,codecinfo,sizeof(btav_sbc_codec_config_t));
            src_codec_type = A2DP_SINK_AUDIO_CODEC_SBC;
        }
        PRINTBIT(codecinfo,10);
    }

    while (media_playing) {
        if(is_sink_relay_enabled)
        {
            ALOGD(LOGTAG_A2DP "try to get the codec information of snk side");
            if( GetCodecInfoByAddr(NULL,&snk_codec_type,&snk_codec_cfg))
            {
                if((a2dp_playstatus == A2DP_SOURCE_AUDIO_SUSPENDED) &&(srcStream != SRC_STREAMING))
                {
                    ALOGD(LOGTAG_A2DP" resume: playStatus = %d  srcStreamStatus=%d",playStatus,srcStream);
                    BtA2dpResumeStreaming();
                }
                srcStream = SRC_STREAMING;
                if (snk_codec_type != A2DP_SINK_AUDIO_CODEC_SBC)
                    use_file_stream = 1;
                else
                {
                        codec_type = get_codec_relay_data();
                        if(codec_type == INVALID_CODEC)
                        {
                            //ALOGD(LOGTAG_A2DP "enque relay empty");
                            len = 0;
                        }
                        else if(codec_type == A2DP_SINK_AUDIO_CODEC_PCM)
                        {
                            if((src_codec_type == A2DP_SINK_AUDIO_CODEC_SBC)
                               &&(!memcmp(&src_codec_cfg,&snk_codec_cfg,5)))
                            {
                                use_file_stream = 0;
                                len = get_pcm_data((uint8_t*)buffer, out_buffer_size);
                            }
                            else
                            {
                                ALOGD(LOGTAG_A2DP "audio parameter not mach, using file");
                                len=0;
                                use_file_stream = 1;
                            }
                        }
                        else if(codec_type == A2DP_SINK_AUDIO_CODEC_SBC)//pcm data
                        {
                            if(src_codec_type == A2DP_SINK_AUDIO_CODEC_SBC)
                            {
                                PRINTBIT(&snk_codec_cfg,7);
                                PRINTBIT(&src_codec_cfg,7);
                                //if src and snk codec match, compare codec config here;
                                //if(src_codec_type == A2DP_SINK_AUDIO_CODEC_SBC
                                if (!memcmp(&src_codec_cfg,&snk_codec_cfg,sizeof(btav_sbc_codec_config_t)))
                                    len = get_sbc_data((uint8_t*)buffer, out_buffer_size);
                                else
                                {
                                    ALOGD(LOGTAG_A2DP "sbc codec not match, and decoding is not enabled. using file");
                                    use_file_stream = 1;
                                }
                            }
                            else
                            {
                                len=0;
                                use_file_stream = 1;
                            }
                        }
                 }
            }
            else
            {
                use_file_stream = 0;
                ALOGD(LOGTAG_A2DP "cannot get the snk info, may be no streaming ");
                if((a2dp_playstatus == A2DP_SOURCE_AUDIO_STARTED) &&( srcStream != SRC_NO_STREAMING))
                {
                    ALOGD(LOGTAG_A2DP" suspend: playStatus = %d  srcStreamStatus=%d",playStatus,srcStream);
                    BtA2dpSuspendStreaming();
                }
                srcStream= SRC_NO_STREAMING;
                len =0;
            }
            if (len == 0 && (use_file_stream ==0)) {
                ALOGD(LOGTAG_A2DP "Read %d bytes from file sleep 20ms", len);
                usleep(20000);
                continue;
            }
        }
        ALOGD("use file steaming %d relay %d",use_file_stream,is_sink_relay_enabled);
        if(!is_sink_relay_enabled || use_file_stream)
        {
             /* Use file for streaming */
             ALOGD(LOGTAG_A2DP "use file steaming Read %d buffer size", out_buffer_size);
             len = fread(buffer, out_buffer_size, 1, in_file);
             if (len == 0) {
                 ALOGD(LOGTAG_A2DP "Read %d bytes from file", len);
                 fseek(in_file, 0, SEEK_SET);
                 continue;
             }
             codec_type = A2DP_SINK_AUDIO_CODEC_PCM;
             len = out_buffer_size;
        }
        ALOGD(LOGTAG_A2DP "Read %d bytes from file   ==%d ", len,sizeof(len));
#if (defined(BT_AUDIO_HAL_INTEGRATION))
        pthread_mutex_lock(&a2dp_hal_mutex);
        if (!output_stream) {
            pthread_mutex_unlock(&a2dp_hal_mutex);
            break;
        }
        //ALOGD(LOGTAG_A2DP"list the content of buffer send to the device");
        //PRINTBIT(buffer,7);
        //ALOGD(LOGTAG_A2DP"**QCOM** size wanna to write =%d, acctully = %d",len,write_len);
        if(len!=0)
        {
            if(src_codec_type == NON_A2DP_MEDIA_CT)
            {
                write_len = output_stream->write(output_stream, buffer, len);
            }
            else
            {
                write_len = output_stream->write(output_stream, &codec_type, sizeof(codec_type));
                write_len = output_stream->write(output_stream, &len, sizeof(len));
                write_len = output_stream->write(output_stream, buffer, len);
            }
        }
        pthread_mutex_unlock(&a2dp_hal_mutex);
#endif
        ALOGD(LOGTAG_A2DP "codec_type %d Wrote %d bytes to A2dp Hal",codec_type, write_len);
    };
    media_playing = false;
    if (in_file) fclose(in_file);
    ALOGD(LOGTAG_A2DP "Streaming thread about to finish");
    return NULL;
}

static void BtA2dpStartStreaming()
{
    FILE *in_file = NULL;

    ALOGD(LOGTAG_A2DP "Start A2dp Stream");
    if (true || !is_sink_relay_enabled) {
        in_file = fopen("/data/misc/bluetooth/pcmtest.wav", "r");
        if (!in_file) {
            ALOGE(LOGTAG_A2DP "Cannot open input file. Bail out!!");
            return;
        }
    }
    ALOGD(LOGTAG_A2DP "Successfully opened input file for playback");
    media_playing = true;
    if (pthread_create(&playback_thread, NULL, thread_func, in_file) != 0) {
        ALOGD(LOGTAG_A2DP "Cannot create playback thread!\n");
        if (in_file) fclose(in_file);
        return;
    }
    return;
}

static void bta2dp_connection_state_callback(btav_connection_state_t state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG_A2DP " Connection State CB");
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->a2dpSourceEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    switch( state ) {
        case BTAV_CONNECTION_STATE_DISCONNECTED:
            pEvent->a2dpSourceEvent.event_id = A2DP_SOURCE_DISCONNECTED_CB;
        break;
        case BTAV_CONNECTION_STATE_CONNECTING:
            pEvent->a2dpSourceEvent.event_id = A2DP_SOURCE_CONNECTING_CB;
        break;
        case BTAV_CONNECTION_STATE_CONNECTED:
            pEvent->a2dpSourceEvent.event_id = A2DP_SOURCE_CONNECTED_CB;
        break;
        case BTAV_CONNECTION_STATE_DISCONNECTING:
            pEvent->a2dpSourceEvent.event_id = A2DP_SOURCE_DISCONNECTING_CB;
        break;
    }
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void bta2dp_audio_state_callback(btav_audio_state_t state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG_A2DP " Audio State CB");
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->a2dpSourceEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    switch( state ) {
        case BTAV_AUDIO_STATE_REMOTE_SUSPEND:
            pEvent->a2dpSourceEvent.event_id = A2DP_SOURCE_AUDIO_SUSPENDED;
            a2dp_playstatus = A2DP_SOURCE_AUDIO_SUSPENDED;
        break;
        case BTAV_AUDIO_STATE_STOPPED:
            pEvent->a2dpSourceEvent.event_id = A2DP_SOURCE_AUDIO_STOPPED;
            a2dp_playstatus = A2DP_SOURCE_AUDIO_STOPPED;
        break;
        case BTAV_AUDIO_STATE_STARTED:
            pEvent->a2dpSourceEvent.event_id = A2DP_SOURCE_AUDIO_STARTED;
            a2dp_playstatus = A2DP_SOURCE_AUDIO_STARTED;
        break;
    }
    ALOGD(LOGTAG_A2DP " Audio State = %d",a2dp_playstatus);
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void bta2dp_connection_priority_vendor_callback(bt_bdaddr_t* bd_addr) {
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->a2dpSourceEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->a2dpSourceEvent.event_id = A2DP_SOURCE_CONNECTION_PRIORITY_REQ;
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void bta2dp_multicast_state_vendor_callback(int state) {
    ALOGD(LOGTAG_A2DP " Multicast State CB");
}

static void bta2dp_delay_report_vendor_callback(bt_bdaddr_t *bd_addr, uint16_t report_delay) {
    char str[18];
    bdaddr_to_string(bd_addr,str, 18);
    ALOGD(LOGTAG_A2DP "Received delay report! [%s] the delay is [%d]", str, report_delay);
    fprintf(stdout, "Received delay report! the delay is %d ms\n", report_delay);
}

static void bta2dp_audio_codec_config_vendor_callback(bt_bdaddr_t *bd_addr, uint16_t codec_type,
        btav_codec_config_t codec_config) {
    ALOGD(LOGTAG_A2DP " bta2dp_audio_codec_config_vendor_callback codec_type=%d",codec_type);

    BtEvent *pEvent = new BtEvent;
    pEvent->a2dpSourceEvent.event_id = A2DP_SOURCE_CODEC_CONFIG_CB;
    memcpy(&pEvent->a2dpSourceEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->a2dpSourceEvent.buf_size = sizeof(btav_codec_config_t);
    pEvent->a2dpSourceEvent.buf_ptr = (uint8_t*)osi_malloc(pEvent->a2dpSourceEvent.buf_size);
    memcpy(pEvent->a2dpSourceEvent.buf_ptr, &codec_config, pEvent->a2dpSourceEvent.buf_size);
    pEvent->a2dpSourceEvent.arg1 = codec_type;
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static btav_callbacks_t sBluetoothA2dpSourceCallbacks = {
    sizeof(sBluetoothA2dpSourceCallbacks),
    bta2dp_connection_state_callback,
    bta2dp_audio_state_callback,
    NULL,
};

static btav_vendor_callbacks_t sBluetoothA2dpSourceVendorCallbacks = {
    sizeof(sBluetoothA2dpSourceVendorCallbacks),
    bta2dp_connection_priority_vendor_callback,
    bta2dp_multicast_state_vendor_callback,
    NULL,
    NULL,
    bta2dp_delay_report_vendor_callback,
    bta2dp_audio_codec_config_vendor_callback,
};

static void btavrc_target_passthrough_cmd_vendor_callback(int id, int key_state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG_AVRCP " btavrcp_target_passthrough_cmd_callback id = %d key_state = %d", id, key_state);
    if (key_state == KEY_PRESSED) {
        BtEvent *event = new BtEvent;
        event->avrcpTargetEvent.event_id = A2DP_SOURCE_AUDIO_CMD_REQ;
        /*As there is no player impl available at this point hence STOP/PAUSE has got same functionality*/
        if(id == CMD_ID_PAUSE)
            id = CMD_ID_STOP;
        event->avrcpTargetEvent.key_id = id;
        PostMessage (THREAD_ID_A2DP_SOURCE, event);
    }
}

static void btavrc_target_setaddrplayer_cmd_vendor_callback(uint32_t player_id, bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP " btavrc_target_setaddrplayer_cmd_vendor_callback ");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_SET_ADDR_PLAYER_CB;
    pEvent->avrcpTargetEvent.arg1 = (uint16_t)player_id;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    PostMessage (THREAD_ID_A2DP_SOURCE, pEvent);
}

static void btavrc_target_getfolderitems_cmd_vendor_callback(btrc_vendor_browse_folderitem_t id,
                  btrc_vendor_getfolderitem_t *param, bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP " btavrc_target_getfolderitems_cmd_vendor_callback ");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_GET_FOLDER_ITEMS_CB;

    FolderListEntries* folderItem = (FolderListEntries*)osi_malloc(sizeof(FolderListEntries));
    memcpy(&folderItem->p_attr, &param->attrs, sizeof(param->attrs));
    folderItem->mStart = param->start_item;
    folderItem->mEnd = param->end_item;
    folderItem->mSize = param->size;
    folderItem->mNumAttr = param->attr_count;

    pEvent->avrcpTargetEvent.buf_size = sizeof(folderItem);
    pEvent->avrcpTargetEvent.buf_ptr = (uint8_t*)osi_malloc(pEvent->avrcpTargetEvent.buf_size);
    memcpy(pEvent->avrcpTargetEvent.buf_ptr, &folderItem, pEvent->avrcpTargetEvent.buf_size);
    pEvent->avrcpTargetEvent.arg1 = (uint16_t)id;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    PostMessage (THREAD_ID_A2DP_SOURCE, pEvent);
}

static void btavrc_target_connection_state_vendor_callback(bool state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG_AVRCP " btavrcp_target_connection_state_callback state = %d", state);
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    if (state == true)
        pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_CONNECTED_CB;
    else
        pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_DISCONNECTED_CB;
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void btavrcp_target_rcfeatures_callback( bt_bdaddr_t* bd_addr, btrc_remote_features_t features) {
    ALOGD(LOGTAG_AVRCP " btavrcp_target_rcfeatures_callback features = %d", features);
    if ((features & BTRC_FEAT_ABSOLUTE_VOLUME) != 0) {
        ALOGD(LOGTAG_AVRCP " Abs vol supported for dev ");
        pA2dpSource->mAbsVolRemoteSupported = true;
    } else {
        ALOGD(LOGTAG_AVRCP " Abs vol NOT supported for dev ");
    }
    pA2dpSource->mVolCmdSetInProgress = false;
    pA2dpSource->mVolCmdAdjustInProgress = false;
    pA2dpSource->mInitialRemoteVolume = -1;
    pA2dpSource->mLastRemoteVolume = -1;
    pA2dpSource->mRemoteVolume = -1;
    pA2dpSource->mLastLocalVolume = -1;
    pA2dpSource->mLocalVolume = -1;
}

static void btavrcp_target_getelemattr_vendor_callback(uint8_t num_attr,
        btrc_vendor_media_attr_t *p_attrs, bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP " btavrcp_target_getelemattr_vendor_callback ");
    int i;
    for (i = 0; i < num_attr; ++i) {
        ALOGD(LOGTAG_AVRCP " btavrcp_target_getelemattr_callback features = %d", p_attrs[i]);
    }
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_GET_ELE_ATTR;

    ItemAttr* itemAttr = (ItemAttr*)osi_malloc(sizeof(ItemAttr));
    itemAttr->p_attr = (btrc_media_attr_t*)osi_malloc(num_attr * sizeof(btrc_media_attr_t));
    memcpy(itemAttr->p_attr, p_attrs, num_attr * sizeof(btrc_media_attr_t));
    itemAttr->mUid = 0;
    itemAttr->mSize = 0;

    pEvent->avrcpTargetEvent.buf_size = sizeof(ItemAttr);
    pEvent->avrcpTargetEvent.buf_ptr = (uint8_t*)osi_malloc(pEvent->avrcpTargetEvent.buf_size);
    memcpy(pEvent->avrcpTargetEvent.buf_ptr, itemAttr, pEvent->avrcpTargetEvent.buf_size);
    ItemAttr* pAttr = (ItemAttr*)pEvent->avrcpTargetEvent.buf_ptr;
    pAttr->p_attr = (btrc_media_attr_t*)osi_malloc(num_attr * sizeof(btrc_media_attr_t));
    memcpy(pAttr->p_attr, itemAttr->p_attr, num_attr * sizeof(btrc_media_attr_t));
    pEvent->avrcpTargetEvent.arg1 = (uint16_t)num_attr;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
    osi_free(itemAttr->p_attr);
    osi_free(itemAttr);
}

static void btavrcp_target_getplaystatus_vendor_callback(bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP " btavrcp_target_getplaystatus_vendor_callback ");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_GET_PLAY_STATUS;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void btavrcp_target_listplayerapp_attr_vendor_callback(bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP " btavrcp_target_listplayerapp_attr_vendor_callback ");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_LIST_PLAYER_APP_ATTR;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void    btavrcp_target_listplayerapp_values_vendor_callback(btrc_player_attr_t attr_id, bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP "btavrcp_target_listplayerapp_values_vendor_callback");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_LIST_PLAYER_APP_VALUES;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->avrcpTargetEvent.attr_id = attr_id;
    ALOGD(LOGTAG_AVRCP "attr_id:%d", attr_id);
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void  btavrcp_target_getplayerapp_value_vendor_callback(uint8_t num_attr,
                                                    btrc_player_attr_t *p_attrs, bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP "btavrcp_target_getplayerapp_value_vendor_callback");
    BtEvent *pEvent = new BtEvent;
    int i;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_GET_PLAYER_APP_VALUE;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->avrcpTargetEvent.arg3 = num_attr;
    ALOGD(LOGTAG_AVRCP "num_attr:%d",num_attr);
    for (i = 0; i < num_attr; i++)
        pEvent->avrcpTargetEvent.attr_ids[i] = p_attrs[i];
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void btavrcp_target_setplayerapp_value_vendor_cb(btrc_player_settings_t *p_vals, bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP "btavrcp_target_setplayerapp_value_vendor_cb");
    BtEvent *pEvent = new BtEvent;
    uint8_t i;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_SET_PLAYER_APP_VALUE;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->avrcpTargetEvent.arg3 = p_vals->num_attr;
    ALOGD(LOGTAG_AVRCP "num_attr:%d", p_vals->num_attr);
    for (i = 0; i < p_vals->num_attr; i++) {
        pEvent->avrcpTargetEvent.attr_ids[i] = p_vals->attr_ids[i];
        pEvent->avrcpTargetEvent.attr_values[i] = p_vals->attr_values[i];
        ALOGD(LOGTAG_AVRCP "attr_ids:%d", p_vals->attr_ids[i]);
        ALOGD(LOGTAG_AVRCP "attr_values:%d", p_vals->attr_values[i]);
    }
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void btavrcp_target_regnoti_vendor_callback(btrc_vendor_event_id_t event_id, uint32_t param,
        bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP " btavrcp_target_regnoti_vendor_callback ");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_REG_NOTI;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->avrcpTargetEvent.arg1 = (uint16_t)event_id;
    pEvent->avrcpTargetEvent.arg2 = param;
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static void btavrcp_target_volchanged_vendor_callback(uint8_t volume, uint8_t ctype,
        bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG_AVRCP " btavrcp_target_volchanged_vendor_callback ");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_VOLUME_CHANGED;
    memcpy(&pEvent->avrcpTargetEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->avrcpTargetEvent.arg3 = volume;
    pEvent->avrcpTargetEvent.arg4 = (AvrcRspType)ctype;
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

static btrc_callbacks_t sBluetoothAvrcpTargetCallbacks = {
   sizeof(sBluetoothAvrcpTargetCallbacks),
   btavrcp_target_rcfeatures_callback,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
};

static btrc_vendor_callbacks_t sBluetoothAvrcpTargetVendorCallbacks = {
   sizeof(sBluetoothAvrcpTargetVendorCallbacks),
   btavrcp_target_getplaystatus_vendor_callback,
   btavrcp_target_listplayerapp_attr_vendor_callback,
   btavrcp_target_listplayerapp_values_vendor_callback,
   btavrcp_target_getplayerapp_value_vendor_callback,
   NULL,
   NULL,
   btavrcp_target_setplayerapp_value_vendor_cb,
   btavrcp_target_getelemattr_vendor_callback,
   btavrcp_target_regnoti_vendor_callback,
   btavrcp_target_volchanged_vendor_callback,
   btavrc_target_passthrough_cmd_vendor_callback,
   btavrc_target_getfolderitems_cmd_vendor_callback,
   btavrc_target_setaddrplayer_cmd_vendor_callback,
   NULL,
   NULL,
   NULL,
   NULL,
   btavrc_target_connection_state_vendor_callback,
   NULL,
};

const char* getString(int mAttrType) {
    const char* new_str = "";
    const char* title1 = "Here, on the other hand, I've gone crazy \
        and really let the literal span several lines, \
        without bothering with quoting each line's \
        and really let the literal span several lines";
    const char* artistName1 = "Here, on the other hand, I've gone crazy \
        and really let the literal span several lines, \
        without bothering with quoting each line's \
        and really let the literal span several lines";
    const char* title = "abc1";
    const char* artistName = "abc2";
    const char* albumName = "abc3";
    const char* mediaNumber = "abc4";
    const char* mediaTotalNumber = "abc5";
    const char* genre = "abc6";
    const char* playingTimeMs = "abc7";
    const char* tracknum = "abc8";

    switch (mAttrType) {
        case ATTR_TRACK_NUM:
            return tracknum;
        case ATTR_TITLE:
            if (use_bigger_metadata)
                return title1;
            else
                return title;
        case ATTR_ARTIST_NAME:
            if (use_bigger_metadata)
                return artistName1;
            else
                return artistName;
        case ATTR_ALBUM_NAME:
            return albumName;
        case ATTR_MEDIA_NUMBER:
            return mediaNumber;
        case ATTR_MEDIA_TOTAL_NUMBER:
            return mediaTotalNumber;
        case ATTR_GENRE:
            return genre;
        case ATTR_PLAYING_TIME_MS:
            return playingTimeMs;
        default:
            return new_str;
    }
}

void set_abs_volume_timer_handler(void *context) {
    ALOGD(LOGTAG_AVRCP " set_abs_volume_timer_handler ");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_ABS_VOL_TIMEOUT;
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

void A2dp_Source::StartSetAbsVolTimer() {
    if(abs_vol_timer) {
        ALOGD(LOGTAG_AVRCP " Abs Vol Timer still running + ");
        return;
    }
    alarm_set(set_abs_volume_timer, A2DP_SOURCE_SET_ABS_VOL_TIMER_DURATION,
           set_abs_volume_timer_handler, NULL);
    abs_vol_timer = true;
}

void A2dp_Source::StopSetAbsVolTimer() {
    ALOGD(LOGTAG_AVRCP " StopSetAbsVolTimer ");
    if((set_abs_volume_timer != NULL) && (abs_vol_timer)) {
        alarm_cancel(set_abs_volume_timer);
        ALOGD(LOGTAG_AVRCP " StopSetAbsVolTimer -1");
        abs_vol_timer = false;
    }
}

int convertToAudioStreamVolume(int volume) {
    // Rescale volume to match AudioSystem's volume
    return (int) round((double) volume*mAudioStreamMax/AVRCP_MAX_VOL);
}

int convertToAvrcpVolume(int volume) {
    return (int) ceil((double) volume*AVRCP_MAX_VOL/mAudioStreamMax);
}

bool isAbsoluteVolumeSupported() {
    ALOGD(LOGTAG_AVRCP " isAbsoluteVolumeSupported() %d ", pA2dpSource->mAbsVolRemoteSupported);
    return pA2dpSource->mAbsVolRemoteSupported;
}

void PlayPosTimehandler(void *context) {
    ALOGD(LOGTAG_AVRCP "PlayPosTimehandler ");
    BtEvent *pEvent = new BtEvent;
    pEvent->avrcpTargetEvent.event_id = AVRCP_TARGET_PLAY_POSITION_TIMEOUT;
    PostMessage(THREAD_ID_A2DP_SOURCE, pEvent);
}

void A2dp_Source::StartPlayPostionTimer() {
    ALOGD(LOGTAG_AVRCP "%s:Entered",__func__);
    if(play_pos_timer) {
        ALOGD(LOGTAG_AVRCP "Play postion Timer still running + ");
        return;
    }
    ALOGD(LOGTAG_AVRCP "play_position_interval:%d",play_position_interval);
    alarm_set(set_play_postion_timer, play_position_interval * 1000,
                                    PlayPosTimehandler, NULL);
    play_pos_timer = true;
}

void A2dp_Source::StopPlayPostionTimer() {
    ALOGD(LOGTAG_AVRCP " StopPlayPostionTimer ");
    if((set_play_postion_timer != NULL) && (play_pos_timer)) {
        alarm_cancel(set_play_postion_timer);
        ALOGD(LOGTAG_AVRCP " StopPlayPostionTimer -1");
        play_pos_timer = false;
    }
}

void A2dp_Source::HandleAvrcpEvents(BtEvent* pEvent) {
    ALOGD(LOGTAG_AVRCP " HandleAvrcpEvents event = %s",
            dump_message(pEvent->avrcpTargetEvent.event_id));
    uint8_t absvol, avrcpVolume;
    long TrackNumberRsp = -1L, pecentVolChanged;
    char *folderItems, *playerEntry;
    uint16_t num_attr, num_val, scope, set_addr_player_id = 0;
    bool isSetVol, volAdj = false, player_found = false;
    int i, pos = 0, song_len = 0, volIndex, start = 0, count = 0, countElementLength = 0;
    int countTotalBytes = 0, countTemp = 0, checkLength = 0, folderItemLengths[32];
    int availableMediaPlayers = 0, positionItemStart = 0;
    AvrcRspType ctype;
    AvrcKeyDir dir;
    ItemAttr *item = NULL;
    FolderListEntries *folderitem = NULL;
    btrc_element_attr_val_t *pAttrs = NULL;
    btrc_register_notification_t param;
    btrc_vendor_folder_list_entries_t *p_param;
    btrc_player_attr_t p_attr[BTRC_MAX_APP_SETTINGS];
    uint8_t *attr_values;

    switch(pEvent->avrcpTargetEvent.event_id) {
        case AVRCP_TARGET_USE_BIGGER_METADATA:
            ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_USE_BIGGER_METADATA ");
            use_bigger_metadata = true;
            break;
        case AVRCP_TARGET_SET_ADDR_PLAYER_CB:
            set_addr_player_id = pEvent->avrcpTargetEvent.arg1;
            if (pMediaPlayerList.size() > 0) {
                list<MediaPlayerInfo>::iterator p = pMediaPlayerList.begin();
                while (p != pMediaPlayerList.end()) {
                    if (p->mPlayerId == set_addr_player_id)
                    {
                        player_found = true;
                        ALOGD(LOGTAG_AVRCP " valid player found for set addr player ");
                        break;
                    }
                    p++;
                }
            }
            else {
                ALOGE(LOGTAG_AVRCP "  No media players");
            }

            if (!player_found)
            {
                ALOGE(LOGTAG_AVRCP " Since not a valid player %d send error", set_addr_player_id);
                sBtAvrcpTargetVendorInterface->set_addressed_player_response_vendor(
                        (btrc_status_t)0x11, &pEvent->avrcpTargetEvent.bd_addr);
                break;
            }

            ALOGD(LOGTAG_AVRCP " Send response for set addressed player %d", set_addr_player_id);
            sBtAvrcpTargetVendorInterface->set_addressed_player_response_vendor((btrc_status_t)0x04,
                    &pEvent->avrcpTargetEvent.bd_addr);

            if (mCurrentAddrPlayerId == set_addr_player_id)
            {
                ALOGD(LOGTAG_AVRCP " Player already addresssed %d", set_addr_player_id);
            }
            else
            {
                mPreviousAddrPlayerId = mCurrentAddrPlayerId;
                mCurrentAddrPlayerId = set_addr_player_id;
                ALOGD(LOGTAG_AVRCP " mPreviousAddrPlayerId %d mCurrentAddrPlayerId = %d",
                                     mPreviousAddrPlayerId, mCurrentAddrPlayerId);
                if (mAddrPlayerChangedNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                    mAddrPlayerChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                    param.player_id = mCurrentAddrPlayerId;
                    sBtAvrcpTargetInterface->register_notification_rsp(
                            BTRC_EVT_ADDRESSED_PLAYER_CHANGED,
                            mAddrPlayerChangedNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                    if (mPreviousAddrPlayerId != -1)
                        resetAndSendPlayerStatusReject();
                }
            }
            break;
        case AVRCP_TARGET_GET_FOLDER_ITEMS_CB:
            scope = pEvent->avrcpTargetEvent.arg1;
            ALOGD(LOGTAG_AVRCP " Send response for get folder items for scope %d", scope);
            if (pEvent->avrcpTargetEvent.buf_ptr == NULL) {
                break;
            }
            folderitem = (FolderListEntries*)osi_malloc(sizeof(FolderListEntries));
            memcpy(&folderitem, pEvent->avrcpTargetEvent.buf_ptr, pEvent->avrcpTargetEvent.buf_size);
            ALOGD(LOGTAG_AVRCP "  %d %d %d %d", folderitem->mStart, folderitem->mEnd,
                                                folderitem->mSize, folderitem->mNumAttr);
            start = folderitem->mStart;
            folderItems = (char*) osi_malloc(folderitem->mSize * sizeof(char));
            if (scope == 0x00) {
                if (pMediaPlayerList.size() > 0) {
                    list<MediaPlayerInfo>::iterator p = pMediaPlayerList.begin();
                    while (p != pMediaPlayerList.end()) {
                        if (start == 0) {
                            playerEntry = (char*)osi_malloc(p->RetrievePlayerEntryLength()*
                                                               sizeof(char));
                            playerEntry = p->RetrievePlayerItemEntry();
                            int length = p->RetrievePlayerEntryLength();
                            folderItemLengths[availableMediaPlayers ++] = length;
                            for (count = 0; count < length; count ++) {
                                folderItems[positionItemStart + count] = playerEntry[count];
                            }
                            positionItemStart += length; // move start to next item star
                            osi_free(playerEntry);
                        }
                        else if (start > 0) {
                            --start;
                        }
                        p++;
                    }
                }
                else {
                    ALOGE(LOGTAG_AVRCP "  No media players");
                }
            }
            else {
                ALOGE(LOGTAG_AVRCP " Incorrect scope");
            }
            p_param = (btrc_vendor_folder_list_entries_t*)osi_malloc(
                                    sizeof(btrc_vendor_folder_list_entries_t));
            p_param->status = 0x04;
            p_param->uid_counter = 0;
            p_param->item_count = availableMediaPlayers;
            p_param->p_item_list =
               (btrc_vendor_folder_list_item_t*) osi_malloc (p_param->item_count*
                                      sizeof(btrc_vendor_folder_list_item_t));
            for (count = 0; count < p_param->item_count; count++) {
                p_param->p_item_list[count].item_type =
                    folderItems[countTotalBytes]; countTotalBytes++;
                p_param->p_item_list[count].u.player.player_id =
                    (uint16_t)(folderItems[countTotalBytes] & 0x00ff); countTotalBytes++;
                p_param->p_item_list[count].u.player.player_id +=
                    (uint16_t)((folderItems[countTotalBytes] << 8) & 0xff00); countTotalBytes++;
                p_param->p_item_list[count].u.player.major_type =
                    folderItems[countTotalBytes]; countTotalBytes++;
                p_param->p_item_list[count].u.player.sub_type =
                    (uint32_t)(folderItems[countTotalBytes] & 0x000000ff); countTotalBytes++;
                p_param->p_item_list[count].u.player.sub_type +=
                    (uint32_t)((folderItems[countTotalBytes] << 8) & 0x0000ff00); countTotalBytes++;
                p_param->p_item_list[count].u.player.sub_type +=
                    (uint32_t)((folderItems[countTotalBytes] << 16) & 0x00ff0000); countTotalBytes++;
                p_param->p_item_list[count].u.player.sub_type +=
                    (uint32_t)((folderItems[countTotalBytes] << 24) & 0xff000000); countTotalBytes++;
                p_param->p_item_list[count].u.player.play_status =
                    folderItems[countTotalBytes]; countTotalBytes++;
                for (countTemp = 0; countTemp < 16; countTemp ++) {
                    p_param->p_item_list[count].u.player.features[countTemp] =
                    folderItems[countTotalBytes];
                    ALOGD(LOGTAG_A2DP "player feat sending in resp %d",
                        p_param->p_item_list[count].u.player.features[countTemp]);
                    countTotalBytes++;
                }
                p_param->p_item_list[count].u.player.name.charset_id =
                    (uint16_t)(folderItems[countTotalBytes] & 0x00ff); countTotalBytes++;
                p_param->p_item_list[count].u.player.name.charset_id +=
                    (uint16_t)((folderItems[countTotalBytes] << 8) & 0xff00); countTotalBytes++;
                p_param->p_item_list[count].u.player.name.str_len =
                    (uint16_t)(folderItems[countTotalBytes] & 0x00ff); countTotalBytes++;
                p_param->p_item_list[count].u.player.name.str_len +=
                    (uint16_t)((folderItems[countTotalBytes] << 8) & 0xff00); countTotalBytes++;
                p_param->p_item_list[count].u.player.name.p_str =
                    new uint8_t[p_param->p_item_list[count].u.player.name.str_len];
                for (countTemp = 0; countTemp < p_param->p_item_list[count].u.player.name.str_len;
                              countTemp ++) {
                    p_param->p_item_list[count].u.player.name.p_str[countTemp] =
                        folderItems[countTotalBytes]; countTotalBytes++;
                }
                /*To check if byte feeding went well*/
                checkLength += folderItemLengths[count];
                ALOGD(LOGTAG_AVRCP "checkLength = %u countTotalBytes = %u", checkLength,
                        countTotalBytes);
                if (checkLength != countTotalBytes) {
                    ALOGE(LOGTAG_AVRCP "Error Populating Intermediate Folder Entry");
                }
            }
            sBtAvrcpTargetVendorInterface->get_folder_items_response_vendor(p_param,
                                           &pEvent->avrcpTargetEvent.bd_addr);
            osi_free(pEvent->avrcpTargetEvent.buf_ptr);
            osi_free(folderitem);
            osi_free(folderItems);
            osi_free(p_param->p_item_list);
            osi_free(p_param);
            break;
        case AVRCP_TARGET_ABS_VOL_TIMEOUT:
            ALOGD(LOGTAG_AVRCP " MESSAGE_ABS_VOL_TIMEOUT: Volume change cmd timed out");
            mVolCmdSetInProgress = false;
            mVolCmdAdjustInProgress = false;
            break;
        case AVRCP_TARGET_SEND_VOL_UP_DOWN:
            dir = (AvrcKeyDir)pEvent->avrcpTargetEvent.arg3;
            ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_SEND_VOL_UP_DOWN, dir = %d ", dir);
            if (dir == AVRC_KEY_UP)
            {
                sBtAvrcpTargetInterface->send_pass_through_cmd(&mConnectedDevice, CMD_ID_VOL_UP, 0);
                sBtAvrcpTargetInterface->send_pass_through_cmd(&mConnectedDevice, CMD_ID_VOL_UP, 1);
            }
            else if (dir == AVRC_KEY_DOWN)
            {
                sBtAvrcpTargetInterface->send_pass_through_cmd(&mConnectedDevice,
                                                                        CMD_ID_VOL_DOWN, 0);
                sBtAvrcpTargetInterface->send_pass_through_cmd(&mConnectedDevice,
                                                                        CMD_ID_VOL_DOWN, 1);
            }
            else
            {
                ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_SEND_VOL_UP_DOWN: Invalid value");
            }
            break;
        case AVRCP_TARGET_VOLUME_CHANGED:
            if (!isAbsoluteVolumeSupported()) {
                ALOGD(LOGTAG_AVRCP "ignore AVRCP_TARGET_VOLUME_CHANGED");
                break;
            }
            absvol = pEvent->avrcpTargetEvent.arg3 & 0x7f; // discard MSB as it is RFD
            ctype = (AvrcRspType)pEvent->avrcpTargetEvent.arg4;
            ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_VOLUME_CHANGED, vol = %d absvol = %d ctype = %x",
                    pEvent->avrcpTargetEvent.arg3, absvol, ctype);

            if (ctype == AVRC_RSP_ACCEPT || ctype == AVRC_RSP_REJ) {
                if ((mVolCmdSetInProgress == false) && (mVolCmdAdjustInProgress == false)) {
                    ALOGD(LOGTAG_AVRCP "Unsolicited response, ignored");
                    break;
                }
                pA2dpSource->StopSetAbsVolTimer();
                volAdj = mVolCmdAdjustInProgress;
                mVolCmdSetInProgress = false;
                mVolCmdAdjustInProgress = false;
            }

            volIndex = convertToAudioStreamVolume(absvol);
            ALOGD(LOGTAG_AVRCP " Volume Index = %d", volIndex);

            if (mInitialRemoteVolume == -1) {
                mInitialRemoteVolume = absvol;
            }

            if (mLocalVolume != volIndex && (ctype == AVRC_RSP_ACCEPT ||
                    ctype == AVRC_RSP_CHANGED || ctype == AVRC_RSP_INTERIM)) {
                /* If the volume has successfully changed */
                mLocalVolume = volIndex;
                if (mLastLocalVolume != -1 && ctype == AVRC_RSP_ACCEPT) {
                    if (mLastLocalVolume != volIndex) {
                        /* remote volume changed more than requested due to
                                      * local and remote has different volume steps */
                        ALOGD(LOGTAG_AVRCP "Remote returned vol does not match desired volume %d",
                        mLastLocalVolume, " vs %d", volIndex);
                        mLastLocalVolume = mLocalVolume;
                    }
                }

                // remember the remote volume value, as it's the one supported by remote
                if (volAdj) {
                    ALOGD(LOGTAG_AVRCP "TODO : remember the remote volume value,"
                                             "as it's the one supported by remote");
                }

                mRemoteVolume = absvol;
                pecentVolChanged = ((long)absvol * 100) / 0x7f;
                ALOGD(LOGTAG_AVRCP " percent volume changed: %d", pecentVolChanged, "%");
            }
            else if (ctype == AVRC_RSP_REJ) {
                ALOGD(LOGTAG_AVRCP "setAbsoluteVolume call rejected");
            }
            break;
        case AVRCP_TARGET_SET_ABS_VOL:
            ALOGD(LOGTAG_AVRCP "AVRCP_TARGET_SET_ABS_VOL, vol step = %d",
                                               pEvent->avrcpTargetEvent.arg3);
            if (!isAbsoluteVolumeSupported()) {
                ALOGD(LOGTAG_AVRCP "ignore MESSAGE_SET_ABSOLUTE_VOLUME");
                break;
            }
            if (pEvent->avrcpTargetEvent.arg3 < 0 ||
                           pEvent->avrcpTargetEvent.arg3 > mAudioStreamMax) {
                ALOGD(LOGTAG_AVRCP "wrong vol step input");
                break;
            }
            if (mVolCmdSetInProgress || mVolCmdAdjustInProgress){
                ALOGD(LOGTAG_AVRCP "There is already a volume command in progress.");
                break;
            }
            if (mInitialRemoteVolume == -1) {
                ALOGD(LOGTAG_AVRCP "remote never tell us initial volume, black list it.");
                break;
            }
            avrcpVolume = std::min(AVRCP_MAX_VOL,
                          std::max(0, convertToAvrcpVolume(pEvent->avrcpTargetEvent.arg3)));
            isSetVol = sBtAvrcpTargetInterface->set_volume(avrcpVolume,
                                                &pEvent->avrcpTargetEvent.bd_addr);
            if (isSetVol == BT_STATUS_SUCCESS) {
                pA2dpSource->StartSetAbsVolTimer();
                mVolCmdSetInProgress = true;
                mLastRemoteVolume = avrcpVolume;
                mLastLocalVolume = pEvent->avrcpTargetEvent.arg3;
            } else {
                ALOGE(LOGTAG_AVRCP "setVolumeNative failed");
            }
            break;
        case AVRCP_TARGET_TRACK_CHANGED:
            ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_TRACK_CHANGED");
            if (mTrackChangeNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                mCurrentTrackID = TRACK_IS_SELECTED;
                TrackNumberRsp = mCurrentTrackID;
                mTrackChangeNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                ALOGD(LOGTAG_AVRCP " TrackNumberRsp = %l", TrackNumberRsp);
                for (int i = 0; i < 8; ++i) {
                    param.track[i] = (uint8_t) (TrackNumberRsp >> (56 - 8 * i));
                }
                sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_TRACK_CHANGE,
                        mTrackChangeNotiType,
                        &param, &pEvent->avrcpTargetEvent.bd_addr);
            }
            break;
        case AVRCP_TARGET_ADDR_PLAYER_CHANGED:
            ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_ADDR_PLAYER_CHANGED %d",
                                         pEvent->avrcpTargetEvent.arg3);
            if (mCurrentAddrPlayerId != pEvent->avrcpTargetEvent.arg3) {
                mPreviousAddrPlayerId = mCurrentAddrPlayerId;
                mCurrentAddrPlayerId = pEvent->avrcpTargetEvent.arg3;
                ALOGD(LOGTAG_AVRCP " mPreviousAddrPlayerId = %d mCurrentAddrPlayerId = %d",
                                     mPreviousAddrPlayerId, mCurrentAddrPlayerId);
                if (mAddrPlayerChangedNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                    mAddrPlayerChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                    param.player_id = mCurrentAddrPlayerId;
                    sBtAvrcpTargetInterface->register_notification_rsp(
                            BTRC_EVT_ADDRESSED_PLAYER_CHANGED,
                            mAddrPlayerChangedNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                    if (mPreviousAddrPlayerId != -1)
                        resetAndSendPlayerStatusReject();
                }
            }
            break;
        case AVRCP_TARGET_AVAIL_PLAYER_CHANGED:
            ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_AVAIL_PLAYER_CHANGED");
            if (mAvailPlayerChangedNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                mAvailPlayerChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                sBtAvrcpTargetInterface->register_notification_rsp(
                        BTRC_EVT_AVAILABLE_PLAYERS_CHANGED,
                        mAvailPlayerChangedNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
            }
            break;
        case AVRCP_TARGET_GET_ELE_ATTR:
            num_attr = pEvent->avrcpTargetEvent.arg1;
            if (pEvent->avrcpTargetEvent.buf_ptr == NULL) {
                break;
            }
            ALOGD(LOGTAG_AVRCP " Send response for Get element attribute, num_attr %d", num_attr);
            item = (ItemAttr*)osi_malloc(sizeof(ItemAttr));
            memcpy(item, pEvent->avrcpTargetEvent.buf_ptr, pEvent->avrcpTargetEvent.buf_size);
            ALOGD(LOGTAG_AVRCP " Uid %d Size %d", item->mUid, item->mSize);
            for (i = 0; i < num_attr; ++i) {
                ALOGD(LOGTAG_AVRCP " attr[%d] %d", i, item->p_attr[i]);
            }
            pAttrs = (btrc_element_attr_val_t*)osi_malloc(num_attr*sizeof(btrc_element_attr_val_t));
            for (int i = 0; i < num_attr; ++i) {
                pAttrs[i].attr_id = item->p_attr[i];
                memcpy(pAttrs[i].text, getString(pAttrs[i].attr_id),
                                strlen(getString(pAttrs[i].attr_id))+1);
                ALOGD(LOGTAG_AVRCP " %d %s", pAttrs[i].attr_id, pAttrs[i].text);
            }
            sBtAvrcpTargetInterface->get_element_attr_rsp((uint8_t)num_attr, pAttrs,
                                                 &pEvent->avrcpTargetEvent.bd_addr);
            osi_free(pEvent->avrcpTargetEvent.buf_ptr);
            osi_free(item->p_attr);
            osi_free(item);
            osi_free(pAttrs);
            use_bigger_metadata = false;
            break;
        case AVRCP_SET_EQUALIZER_VAL:
            if (mAppSettingChangedNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                mCurrentEqualizer = (AvrcKeyDir)pEvent->avrcpTargetEvent.arg3;
                mAppSettingChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                param.player_setting.num_attr = NUMPLAYER_ATTRIBUTE;
                param.player_setting.attr_ids[0] = ATTRIBUTE_EQUALIZER;
                param.player_setting.attr_values[0]= mCurrentEqualizer;
                param.player_setting.attr_ids[1] = ATTRIBUTE_REPEATMODE;
                param.player_setting.attr_values[1] = mCurrentRepeat;
                param.player_setting.attr_ids[2] = ATTRIBUTE_SHUFFLEMODE;
                param.player_setting.attr_values[2] = mCurrentShuffle;
                param.player_setting.attr_ids[3] = ATTRIBUTE_SCANMODE;
                param.player_setting.attr_values[3] = mCurrentScan;
                sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_APP_SETTINGS_CHANGED,
                            mAppSettingChangedNotiType, &param, &pA2dpSource->mConnectedAvrcpDevice);
            }
            break;
        case AVRCP_SET_REPEAT_VAL:
            if (mAppSettingChangedNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                mCurrentRepeat = (AvrcKeyDir)pEvent->avrcpTargetEvent.arg3;
                mAppSettingChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                param.player_setting.num_attr = NUMPLAYER_ATTRIBUTE;
                param.player_setting.attr_ids[0] = ATTRIBUTE_EQUALIZER;
                param.player_setting.attr_values[0]= mCurrentEqualizer;
                param.player_setting.attr_ids[1] = ATTRIBUTE_REPEATMODE;
                param.player_setting.attr_values[1] = mCurrentRepeat;
                param.player_setting.attr_ids[2] = ATTRIBUTE_SHUFFLEMODE;
                param.player_setting.attr_values[2] = mCurrentShuffle;
                param.player_setting.attr_ids[3] = ATTRIBUTE_SCANMODE;
                param.player_setting.attr_values[3] = mCurrentScan;
                sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_APP_SETTINGS_CHANGED,
                     mAppSettingChangedNotiType, &param, &pA2dpSource->mConnectedAvrcpDevice);
            }
            break;
        case AVRCP_SET_SHUFFLE_VAL:
            if (mAppSettingChangedNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                mCurrentShuffle = (AvrcKeyDir)pEvent->avrcpTargetEvent.arg3;
                mAppSettingChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                param.player_setting.num_attr = NUMPLAYER_ATTRIBUTE;
                param.player_setting.attr_ids[0] = ATTRIBUTE_EQUALIZER;
                param.player_setting.attr_values[0]= mCurrentEqualizer;
                param.player_setting.attr_ids[1] = ATTRIBUTE_REPEATMODE;
                param.player_setting.attr_values[1] = mCurrentRepeat;
                param.player_setting.attr_ids[2] = ATTRIBUTE_SHUFFLEMODE;
                param.player_setting.attr_values[2] = mCurrentShuffle;
                param.player_setting.attr_ids[3] = ATTRIBUTE_SCANMODE;
                param.player_setting.attr_values[3] = mCurrentScan;
                sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_APP_SETTINGS_CHANGED,
                       mAppSettingChangedNotiType, &param, &pA2dpSource->mConnectedAvrcpDevice);
            }
            break;
        case AVRCP_SET_SCAN_VAL:
            if (mAppSettingChangedNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                mCurrentScan = (AvrcKeyDir)pEvent->avrcpTargetEvent.arg3;
                mAppSettingChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                param.player_setting.num_attr = NUMPLAYER_ATTRIBUTE;
                param.player_setting.attr_ids[0] = ATTRIBUTE_EQUALIZER;
                param.player_setting.attr_values[0]= mCurrentEqualizer;
                param.player_setting.attr_ids[1] = ATTRIBUTE_REPEATMODE;
                param.player_setting.attr_values[1] = mCurrentRepeat;
                param.player_setting.attr_ids[2] = ATTRIBUTE_SHUFFLEMODE;
                param.player_setting.attr_values[2] = mCurrentShuffle;
                param.player_setting.attr_ids[3] = ATTRIBUTE_SCANMODE;
                param.player_setting.attr_values[3] = mCurrentScan;
                sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_APP_SETTINGS_CHANGED,
                        mAppSettingChangedNotiType, &param, &pA2dpSource->mConnectedAvrcpDevice);
            }
            break;
        case AVRCP_TARGET_PLAY_POSITION_TIMEOUT:
            param.song_pos = a2dp_play_position;
            pA2dpSource->StopPlayPostionTimer();
            mPlayPosChangedNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
            sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_PLAY_POS_CHANGED,
                                  mPlayPosChangedNotiType, &param, &pA2dpSource->mConnectedAvrcpDevice);
            break;
        case AVRCP_TARGET_GET_PLAY_STATUS:
            ALOGD(LOGTAG_AVRCP " Send response for Get play status = %d",playStatus);
            pos = 10L;
            song_len = 100L;
            if(playStatus == BTRC_PLAYSTATE_ERROR)
            {
                playStatus = BTRC_PLAYSTATE_STOPPED;
                ALOGD(LOGTAG_AVRCP " set  play status as stopped = %d",playStatus);
            }
            sBtAvrcpTargetInterface->get_play_status_rsp(playStatus,
                    song_len, pos, &pEvent->avrcpTargetEvent.bd_addr);
            break;
        case AVRCP_TARGET_LIST_PLAYER_APP_ATTR:
            ALOGD(LOGTAG_AVRCP " Send response for list player app attr");
            num_attr =  4;
            p_attr[0] = BTRC_PLAYER_ATTR_EQUALIZER;
            p_attr[1] = BTRC_PLAYER_ATTR_REPEAT;
            p_attr[2] = BTRC_PLAYER_ATTR_SHUFFLE;
            p_attr[3] = BTRC_PLAYER_ATTR_SCAN;
            sBtAvrcpTargetInterface->list_player_app_attr_rsp(num_attr, p_attr,
                                               &pEvent->avrcpTargetEvent.bd_addr);
            break;
        case AVRCP_TARGET_LIST_PLAYER_APP_VALUES:
            ALOGD(LOGTAG_AVRCP "attr_id:%d", pEvent->avrcpTargetEvent.attr_id);
            switch(pEvent->avrcpTargetEvent.attr_id) {
                case BTRC_PLAYER_ATTR_EQUALIZER:
                    num_val = 2;
                    attr_values = (uint8_t *)osi_malloc(sizeof(uint8_t) * num_val);
                    attr_values[0] = BTRC_PLAYER_VAL_OFF_EQUALIZER;
                    attr_values[1] = BTRC_PLAYER_VAL_ON_EQUALIZER;
                    break;
                case BTRC_PLAYER_ATTR_REPEAT:
                    num_val = 4;
                    attr_values = (uint8_t *)osi_malloc(sizeof(uint8_t) * num_val);
                    attr_values[0] = BTRC_PLAYER_VAL_OFF_REPEAT;
                    attr_values[1] = BTRC_PLAYER_VAL_SINGLE_REPEAT;
                    attr_values[2] = BTRC_PLAYER_VAL_ALL_REPEAT;
                    attr_values[3] = BTRC_PLAYER_VAL_GROUP_REPEAT;
                    break;
                case BTRC_PLAYER_ATTR_SHUFFLE:
                    num_val = 3;
                    attr_values = (uint8_t *)osi_malloc(sizeof(uint8_t) * num_val);
                    attr_values[0] = BTRC_PLAYER_VAL_OFF_SHUFFLE;
                    attr_values[1] = BTRC_PLAYER_VAL_ALL_SHUFFLE;
                    attr_values[2] = BTRC_PLAYER_VAL_GROUP_SHUFFLE;
                    break;
                case BTRC_PLAYER_ATTR_SCAN:
                    num_val = 3;
                    attr_values = (uint8_t *)osi_malloc(sizeof(uint8_t) * num_val);
                    attr_values[0] = BTRC_PLAYER_VAL_OFF_SCAN;
                    attr_values[1] = BTRC_PLAYER_VAL_ON_SCAN;
                    attr_values[2] = BTRC_PLAYER_VAL_GRP_SCAN;
                    break;
            }
            sBtAvrcpTargetInterface->list_player_app_value_rsp(num_val, attr_values,
                                         &pEvent->avrcpTargetEvent.bd_addr);
            if (attr_values)
                osi_free(attr_values);
            break;
        case AVRCP_TARGET_GET_PLAYER_APP_VALUE:
            ALOGD(LOGTAG_AVRCP "No of attr:%d", pEvent->avrcpTargetEvent.arg3);
            btrc_player_settings_t get_app_rsp;
            memset(&get_app_rsp, 0, sizeof(btrc_player_settings_t));
            get_app_rsp.num_attr = pEvent->avrcpTargetEvent.arg3;
            for(i = 0; i < pEvent->avrcpTargetEvent.arg3; i++) {
                get_app_rsp.attr_ids[i] = pEvent->avrcpTargetEvent.attr_ids[i];
                if (pEvent->avrcpTargetEvent.attr_ids[i] == BTRC_PLAYER_ATTR_EQUALIZER) {
                    get_app_rsp.attr_values[i] = mCurrentEqualizer;
                } else if (pEvent->avrcpTargetEvent.attr_ids[i] == BTRC_PLAYER_ATTR_REPEAT) {
                    get_app_rsp.attr_values[i] = mCurrentRepeat;
                } else if (pEvent->avrcpTargetEvent.attr_ids[i] == BTRC_PLAYER_ATTR_SHUFFLE) {
                    get_app_rsp.attr_values[i] = mCurrentShuffle;
                } else if (pEvent->avrcpTargetEvent.attr_ids[i] == BTRC_PLAYER_ATTR_SCAN) {
                    get_app_rsp.attr_values[i] = mCurrentScan;
                }
            }
            sBtAvrcpTargetInterface->get_player_app_value_rsp(&get_app_rsp, &pEvent->avrcpTargetEvent.bd_addr);
            break;
        case AVRCP_TARGET_SET_PLAYER_APP_VALUE:
            for (i = 0; i < pEvent->avrcpTargetEvent.arg3; i++) {
                ALOGD(LOGTAG_AVRCP "attr_ids:%d", pEvent->avrcpTargetEvent.attr_ids[i]);
                ALOGD(LOGTAG_AVRCP "attr_values:%d", pEvent->avrcpTargetEvent.attr_values[i]);
                if (pEvent->avrcpTargetEvent.attr_ids[i] == BTRC_PLAYER_ATTR_EQUALIZER)
                    mCurrentEqualizer = pEvent->avrcpTargetEvent.attr_values[i];
                else if (pEvent->avrcpTargetEvent.attr_ids[i] == BTRC_PLAYER_ATTR_REPEAT)
                    mCurrentRepeat = pEvent->avrcpTargetEvent.attr_values[i];
                else if (pEvent->avrcpTargetEvent.attr_ids[i] == BTRC_PLAYER_ATTR_SHUFFLE)
                    mCurrentShuffle = pEvent->avrcpTargetEvent.attr_values[i];
                else if (pEvent->avrcpTargetEvent.attr_ids[i] == BTRC_PLAYER_ATTR_SCAN)
                    mCurrentScan = pEvent->avrcpTargetEvent.attr_values[i];
            }
            sBtAvrcpTargetInterface->set_player_app_value_rsp(BTRC_STS_NO_ERROR, &pEvent->avrcpTargetEvent.bd_addr);
            break;
        case AVRCP_TARGET_REG_NOTI:
            switch(pEvent->avrcpTargetEvent.arg1) {
                case BTRC_EVT_PLAY_STATUS_CHANGED :
                    ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_REG_NOTI: BTRC_EVT_PLAY_STATUS_CHANGED %d",playStatus);
                    mPlayStatusNotiType = BTRC_NOTIFICATION_TYPE_INTERIM;
                    param.play_status = playStatus;
                    sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_PLAY_STATUS_CHANGED,
                            mPlayStatusNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                    break;
                case BTRC_EVT_TRACK_CHANGE:
                    ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_REG_NOTI: BTRC_EVT_TRACK_CHANGE");
                    mTrackChangeNotiType = BTRC_NOTIFICATION_TYPE_INTERIM;
                    TrackNumberRsp = mCurrentTrackID;
                    ALOGD(LOGTAG_AVRCP " TrackNumberRsp = %l", TrackNumberRsp);
                    for (int i = 0; i < 8; ++i) {
                        param.track[i] = (uint8_t) (TrackNumberRsp >> (56 - 8 * i));
                    }
                    sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_TRACK_CHANGE,
                            mTrackChangeNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                    break;
                case BTRC_EVT_PLAY_POS_CHANGED:
                    ALOGD(LOGTAG_AVRCP "AVRCP_TARGET_REG_NOTI: BTRC_EVT_PLAY_POS_CHANGED");
                    ALOGD(LOGTAG_AVRCP "play_position_interval:%d", pEvent->avrcpTargetEvent.arg2);
                    param.song_pos = a2dp_play_position;
                    mPlayPosChangedNotiType = BTRC_NOTIFICATION_TYPE_INTERIM;
                    play_position_interval = pEvent->avrcpTargetEvent.arg2;  //Interval sec
                    sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_PLAY_POS_CHANGED,
                                    mPlayPosChangedNotiType, &param, &pA2dpSource->mConnectedAvrcpDevice);
                    if (playStatus == BTRC_PLAYSTATE_PLAYING)
                        pA2dpSource->StartPlayPostionTimer();
                    break;
                case BTRC_EVT_APP_SETTINGS_CHANGED:
                    ALOGD(LOGTAG_AVRCP " AVRCP_TARGET_REG_NOTI: BTRC_EVT_APP_SETTINGS_CHANGED");
                    mAppSettingChangedNotiType = BTRC_NOTIFICATION_TYPE_INTERIM;
                    param.player_setting.num_attr = NUMPLAYER_ATTRIBUTE;
                    param.player_setting.attr_ids[0] = ATTRIBUTE_EQUALIZER;
                    param.player_setting.attr_values[0]= mCurrentEqualizer;
                    param.player_setting.attr_ids[1] = ATTRIBUTE_REPEATMODE;
                    param.player_setting.attr_values[1] = mCurrentRepeat;
                    param.player_setting.attr_ids[2] = ATTRIBUTE_SHUFFLEMODE;
                    param.player_setting.attr_values[2] = mCurrentShuffle;
                    param.player_setting.attr_ids[3] = ATTRIBUTE_SCANMODE;
                    param.player_setting.attr_values[3] = mCurrentScan;
                    sBtAvrcpTargetInterface->register_notification_rsp(BTRC_EVT_APP_SETTINGS_CHANGED,
                                           mAppSettingChangedNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                    break;
                case BTRC_EVT_ADDRESSED_PLAYER_CHANGED:
                    ALOGD(LOGTAG_AVRCP "AVRCP_TARGET_REG_NOTI: BTRC_EVT_ADDRESSED_PLAYER_CHANGED ");
                    mAddrPlayerChangedNotiType = BTRC_NOTIFICATION_TYPE_INTERIM;
                    param.player_id = (uint16_t)mCurrentAddrPlayerId;
                    sBtAvrcpTargetInterface->register_notification_rsp(
                            BTRC_EVT_ADDRESSED_PLAYER_CHANGED,
                            mAddrPlayerChangedNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                    break;
                case BTRC_EVT_AVAILABLE_PLAYERS_CHANGED:
                    ALOGD(LOGTAG_AVRCP "AVRCP_TARGET_REG_NOTI: BTRC_EVT_AVAILABLE_PLAYERS_CHANGED ");
                    mAvailPlayerChangedNotiType = BTRC_NOTIFICATION_TYPE_INTERIM;
                    sBtAvrcpTargetInterface->register_notification_rsp(
                            BTRC_EVT_AVAILABLE_PLAYERS_CHANGED,
                            mAvailPlayerChangedNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                    break;
                default:
                    ALOGE(LOGTAG_AVRCP "AVRCP_TARGET_REG_NOTI: unhandled event ");
                    break;
            }
            break;
        case AVRCP_TARGET_CONNECTED_CB:
            mAvrcpConnected = true;
            memcpy(&mConnectedAvrcpDevice, &pEvent->avrcpTargetEvent.bd_addr, sizeof(bt_bdaddr_t));
            break;
        case AVRCP_TARGET_DISCONNECTED_CB:
            mAvrcpConnected = false;
            memset(&mConnectedAvrcpDevice, 0, sizeof(bt_bdaddr_t));
            break;
        case A2DP_SOURCE_AUDIO_CMD_REQ:
            uint8_t key_id = pEvent->avrcpTargetEvent.key_id;
            if (!mAvrcpConnected || (memcmp(&mConnectedAvrcpDevice, &mConnectedDevice,
                           sizeof(bt_bdaddr_t)) != 0)) {
                ALOGD(LOGTAG_AVRCP " No Active connection. Bail out!! ");
                break;
            }
            switch(key_id) {
                case CMD_ID_PLAY:
                    if (media_playing)
                        BtA2dpResumeStreaming();
                    else
                        BtA2dpStartStreaming();
                    if (playStatus != BTRC_PLAYSTATE_PLAYING)
                    {
                        playStatus = BTRC_PLAYSTATE_PLAYING;
                        if (mPlayStatusNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                            param.play_status = playStatus;
                            mPlayStatusNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                            sBtAvrcpTargetInterface->register_notification_rsp(
                                    BTRC_EVT_PLAY_STATUS_CHANGED,
                                    mPlayStatusNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                        }
                    }
                    if (mTrackChangeNotiType == BTRC_NOTIFICATION_TYPE_INTERIM)
                    {
                        mCurrentTrackID = TRACK_IS_SELECTED;
                        TrackNumberRsp = mCurrentTrackID;
                        mTrackChangeNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                        ALOGD(LOGTAG_AVRCP " TrackNumberRsp = %l", TrackNumberRsp);
                        for (int i = 0; i < 8; ++i) {
                            param.track[i] = (uint8_t) (TrackNumberRsp >> (56 - 8 * i));
                        }
                        sBtAvrcpTargetInterface->register_notification_rsp(
                                BTRC_EVT_TRACK_CHANGE,
                                mTrackChangeNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                    }
                    if (mPlayPosChangedNotiType == BTRC_NOTIFICATION_TYPE_INTERIM)
                        pA2dpSource->StartPlayPostionTimer();

                    break;
                case CMD_ID_PAUSE:
                    /*Pause key id is mapped to A2dp suspend*/
                    BtA2dpSuspendStreaming();
                    pA2dpSource->StopPlayPostionTimer();
                    if (playStatus != BTRC_PLAYSTATE_PAUSED)
                    {
                        playStatus = BTRC_PLAYSTATE_PAUSED;
                        if (mPlayStatusNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                            param.play_status = playStatus;
                            mPlayStatusNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                            sBtAvrcpTargetInterface->register_notification_rsp(
                                    BTRC_EVT_PLAY_STATUS_CHANGED,
                                    mPlayStatusNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                        }
                    }
                    break;
                case CMD_ID_STOP:
                    /*Pause and Stop passthrough commands are handled here*/
                    media_playing = false;
                    BtA2dpStopStreaming();
                    pA2dpSource->StopPlayPostionTimer();
                    if (playStatus != BTRC_PLAYSTATE_STOPPED)
                    {
                        playStatus = BTRC_PLAYSTATE_STOPPED;
                        if (mPlayStatusNotiType == BTRC_NOTIFICATION_TYPE_INTERIM) {
                            param.play_status = playStatus;
                            mPlayStatusNotiType = BTRC_NOTIFICATION_TYPE_CHANGED;
                            sBtAvrcpTargetInterface->register_notification_rsp(
                                    BTRC_EVT_PLAY_STATUS_CHANGED,
                                    mPlayStatusNotiType, &param, &pEvent->avrcpTargetEvent.bd_addr);
                        }
                    }
                    break;
                default:
                   ALOGE(LOGTAG_AVRCP " Command not supported ");
                   break;
            }
            break;
    }
}

void A2dp_Source::HandleEnableSource(void) {
    BtEvent *pEvent = new BtEvent;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    if (bluetooth_interface != NULL)
    {
        sBtA2dpSourceInterface = (btav_interface_t *)bluetooth_interface->
                get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID);
        sBtA2dpSourceVendorInterface = (btav_vendor_interface_t *)bluetooth_interface->
                get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_VENDOR_ID);
        if (sBtA2dpSourceInterface == NULL)
        {
             pEvent->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
             pEvent->profile_start_event.profile_id = PROFILE_ID_A2DP_SOURCE;
             pEvent->profile_start_event.status = false;
             PostMessage(THREAD_ID_GAP, pEvent);
             return;
        }
        enable_delay_report = config_get_bool (config, CONFIG_DEFAULT_SECTION, "BtA2dpDelayReportEnable", false);
        ALOGD(LOGTAG_A2DP " ~~ Try to get config , enable_delay_report %d", enable_delay_report);
        //TODO: check and update
#ifdef USE_LIBHW_AOSP
        sBtA2dpSourceInterface->init(&sBluetoothA2dpSourceCallbacks);
#else
        sBtA2dpSourceInterface->init(&sBluetoothA2dpSourceCallbacks, 1, 0);
#endif
        property_get("persist.bt.a2dp_offload_cap", value, "false");
        ALOGD(LOGTAG_A2DP "offload_cap:%s", value);
        if (strcmp(value, "false") == 0)
        {
            if(enable_delay_report)
                sBtA2dpSourceVendorInterface->init_vendor(&sBluetoothA2dpSourceVendorCallbacks, 1, 0, A2DP_SRC_ENABLE_DELAY_REPORTING, NULL);
            else
                sBtA2dpSourceVendorInterface->init_vendor(&sBluetoothA2dpSourceVendorCallbacks, 1, 0, 0, NULL);
        }
        else
        {
            if(enable_delay_report)
                sBtA2dpSourceVendorInterface->init_vendor(&sBluetoothA2dpSourceVendorCallbacks, 1, 0, A2DP_SRC_ENABLE_DELAY_REPORTING, value);
            else
                sBtA2dpSourceVendorInterface->init_vendor(&sBluetoothA2dpSourceVendorCallbacks, 1, 0, 0, value);
        }
        //sBtA2dpSourceVendorInterface->init_vendor(&sBluetoothA2dpSourceVendorCallbacks, 1, 0, NULL);
        pEvent->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
        pEvent->profile_start_event.profile_id = PROFILE_ID_A2DP_SOURCE;
        pEvent->profile_start_event.status = true;
        // AVRCP TG Initialization
        sBtAvrcpTargetInterface = (btrc_interface_t *)bluetooth_interface->
                get_profile_interface(BT_PROFILE_AV_RC_ID);
        if (sBtAvrcpTargetInterface != NULL) {
        //TODO: check and update
#ifdef USE_LIBHW_AOSP
            sBtAvrcpTargetInterface->init(&sBluetoothAvrcpTargetCallbacks);
#else
            sBtAvrcpTargetInterface->init(&sBluetoothAvrcpTargetCallbacks, 1);
#endif
        }
        // AVRCP TG vendor Initialization
        sBtAvrcpTargetVendorInterface = (btrc_vendor_interface_t *)bluetooth_interface->
                get_profile_interface(BT_PROFILE_AV_RC_VENDOR_ID);
        if (sBtAvrcpTargetVendorInterface != NULL) {
            ALOGD(LOGTAG_A2DP "init_vendor for TG");
            sBtAvrcpTargetVendorInterface->init_vendor(&sBluetoothAvrcpTargetVendorCallbacks, 1);
        }
        change_state(STATE_A2DP_SOURCE_DISCONNECTED);
        PostMessage(THREAD_ID_GAP, pEvent);
        ALOGD(LOGTAG_A2DP "Calling BtA2dpLoadA2dpHal");
        BtA2dpLoadA2dpHal();
        media_playing = false;
        playStatus = BTRC_PLAYSTATE_ERROR;
        mCurrentTrackID = NO_TRACK_SELECTED;
        registerMediaPlayers();
    }
    a2dp_sink_relay_data_list = list_new(NULL);
}

void A2dp_Source::HandleDisableSource(void) {
   change_state(STATE_A2DP_SOURCE_NOT_STARTED);
   BtA2dpUnloadA2dpHal();
   if(sBtA2dpSourceInterface != NULL) {
       sBtA2dpSourceInterface->cleanup();
       sBtA2dpSourceInterface = NULL;
   }
   if (sBtAvrcpTargetInterface != NULL) {
       sBtAvrcpTargetInterface->cleanup();
       sBtAvrcpTargetInterface = NULL;
   }
   BtEvent *pEvent = new BtEvent;
   pEvent->profile_stop_event.event_id = PROFILE_EVENT_STOP_DONE;
   pEvent->profile_stop_event.profile_id = PROFILE_ID_A2DP_SOURCE;
   pEvent->profile_stop_event.status = true;
   PostMessage(THREAD_ID_GAP, pEvent);
   media_playing = false;
   playStatus = BTRC_PLAYSTATE_ERROR;
   mCurrentTrackID = NO_TRACK_SELECTED;
   if(a2dp_sink_relay_data_list != NULL)
   list_free(a2dp_sink_relay_data_list);
}

void A2dp_Source::ProcessEvent(BtEvent* pEvent) {
    switch(mSourceState) {
        case STATE_A2DP_SOURCE_DISCONNECTED:
            state_disconnected_handler(pEvent);
            break;
        case STATE_A2DP_SOURCE_PENDING:
            state_pending_handler(pEvent);
            break;
        case STATE_A2DP_SOURCE_CONNECTED:
            state_connected_handler(pEvent);
            break;
        case STATE_A2DP_SOURCE_NOT_STARTED:
            fprintf(stdout, "Ignore!! Make sure BT is turned on!!\n");
            ALOGE(LOGTAG_A2DP " STATE UNINITIALIZED, return");
            break;
    }
}

char* A2dp_Source::dump_message(BluetoothEventId event_id) {
    switch(event_id) {
    case A2DP_SOURCE_API_CONNECT_REQ:
        return"API_CONNECT_REQ";
    case A2DP_SOURCE_API_DISCONNECT_REQ:
        return "API_DISCONNECT_REQ";
    case A2DP_SOURCE_DISCONNECTED_CB:
        return "DISCONNECTED_CB";
    case A2DP_SOURCE_CONNECTING_CB:
        return "CONNECING_CB";
    case A2DP_SOURCE_CONNECTED_CB:
        return "CONNECTED_CB";
    case A2DP_SOURCE_DISCONNECTING_CB:
        return "DISCONNECTING_CB";
    case A2DP_SOURCE_AUDIO_SUSPENDED:
        return "AUDIO_SUSPENDED_CB";
    case A2DP_SOURCE_AUDIO_STOPPED:
        return "AUDIO_STOPPED_CB";
    case A2DP_SOURCE_AUDIO_STARTED:
        return "AUDIO_STARTED_CB";
    case AVRCP_TARGET_CONNECTED_CB:
        return "AVRCP_TARGET_CONNECTED_CB";
    case AVRCP_TARGET_DISCONNECTED_CB:
        return "AVRCP_TARGET_DISCONNECTED_CB";
    case A2DP_SOURCE_AUDIO_CMD_REQ:
        return "AUDIO_CMD_REQ";
    case AVRCP_TARGET_GET_ELE_ATTR:
        return "AVRCP_TARGET_GET_ELE_ATTR";
    case AVRCP_TARGET_GET_PLAY_STATUS:
        return "AVRCP_TARGET_GET_PLAY_STATUS";
    case AVRCP_TARGET_REG_NOTI:
        return "AVRCP_TARGET_REG_NOTI";
    case AVRCP_TARGET_TRACK_CHANGED:
        return "AVRCP_TARGET_TRACK_CHANGED";
    case AVRCP_TARGET_VOLUME_CHANGED:
        return "AVRCP_TARGET_VOLUME_CHANGED";
    case AVRCP_TARGET_SET_ABS_VOL:
        return "AVRCP_TARGET_SET_ABS_VOL";
    case AVRCP_TARGET_ABS_VOL_TIMEOUT:
        return "AVRCP_TARGET_ABS_VOL_TIMEOUT";
    case AVRCP_TARGET_SEND_VOL_UP_DOWN:
        return "AVRCP_TARGET_SEND_VOL_UP_DOWN";
    case AVRCP_TARGET_GET_FOLDER_ITEMS_CB:
        return "AVRCP_TARGET_GET_FOLDER_ITEMS_CB";
    case AVRCP_TARGET_SET_ADDR_PLAYER_CB:
        return "AVRCP_TARGET_SET_ADDR_PLAYER_CB";
    case AVRCP_TARGET_USE_BIGGER_METADATA:
        return "AVRCP_TARGET_USE_BIGGER_METADATA";
    case A2DP_SOURCE_CONNECTION_PRIORITY_REQ:
        return "CONNECTION_PRIORITY_REQ";
    case A2DP_SOURCE_CODEC_CONFIG_CB:
        return "CODEC_CONFIG_CB";
    case AVRCP_TARGET_LIST_PLAYER_APP_ATTR:
        return "AVRCP_TARGET_LIST_PLAYER_APP_ATTR";
    case AVRCP_TARGET_LIST_PLAYER_APP_VALUES:
        return " AVRCP_TARGET_LIST_PLAYER_APP_VALUES";
    case AVRCP_TARGET_GET_PLAYER_APP_VALUE:
        return "AVRCP_TARGET_GET_PLAYER_APP_VALUE";
    case AVRCP_TARGET_SET_PLAYER_APP_VALUE:
        return "AVRCP_TARGET_SET_PLAYER_APP_VALUE";
    case AVRCP_TARGET_PLAY_POSITION_TIMEOUT:
        return "AVRCP_TARGET_PLAY_POSITION_TIMEOUT";
    }
    return "UNKNOWN";
}

void A2dp_Source::state_disconnected_handler(BtEvent* pEvent) {
    char str[18];
    ALOGD(LOGTAG_A2DP "state_disconnected_handler Processing event %s", dump_message(pEvent->event_id));
    switch(pEvent->event_id) {
        case A2DP_SOURCE_API_CONNECT_REQ:
            memcpy(&mConnectingDevice, &pEvent->a2dpSourceEvent.bd_addr, sizeof(bt_bdaddr_t));
            if (sBtA2dpSourceInterface != NULL) {
                sBtA2dpSourceInterface->connect(&pEvent->a2dpSourceEvent.bd_addr);
            }
            bdaddr_to_string(&mConnectingDevice, str, 18);
            fprintf(stdout, "A2DP Source Connecting to %s\n", str);
            change_state(STATE_A2DP_SOURCE_PENDING);
            break;
        case A2DP_SOURCE_API_DISCONNECT_REQ:
            fprintf(stdout, "A2DP Source Disconnect can not be processed\n");
            break;
        case A2DP_SOURCE_CONNECTING_CB:
            memcpy(&mConnectingDevice, &pEvent->a2dpSourceEvent.bd_addr, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&mConnectingDevice, str, 18);
            fprintf(stdout, "A2DP Source Connecting to %s\n", str);
            change_state(STATE_A2DP_SOURCE_PENDING);
            break;
        case A2DP_SOURCE_CONNECTED_CB:
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            memcpy(&mConnectedDevice, &pEvent->a2dpSourceEvent.bd_addr, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&mConnectedDevice, str, 18);
            fprintf(stdout, "A2DP Source Connected to %s\n", str);
            change_state(STATE_A2DP_SOURCE_CONNECTED);
            BtA2dpOpenOutputStream();
            break;
        case A2DP_SOURCE_CONNECTION_PRIORITY_REQ:
            if (sBtA2dpSourceVendorInterface != NULL) {
                sBtA2dpSourceVendorInterface->allow_connection_vendor(1, &pEvent->a2dpSourceEvent.bd_addr);
            }
            break;
        default:
            fprintf(stdout, "Event not processed in disconnected state %d ", pEvent->event_id);
            ALOGE(LOGTAG_A2DP " event not handled %d ", pEvent->event_id);
            break;
    }
}
void A2dp_Source::state_pending_handler(BtEvent* pEvent) {
    char str[18];
    bt_bdaddr_t mDevice;
    uint32_t freq;
    char *mode;
    bool is_valid_codec = true;
    ALOGD(LOGTAG_A2DP "state_pending_handler Processing event %s", dump_message(pEvent->event_id));
    switch(pEvent->event_id) {
        case A2DP_SOURCE_CONNECTED_CB:
            memcpy(&mConnectedDevice, &pEvent->a2dpSourceEvent.bd_addr, sizeof(bt_bdaddr_t));
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&mConnectedDevice, str, 18);
            fprintf(stdout, "A2DP Source Connected to %s\n", str);
            change_state(STATE_A2DP_SOURCE_CONNECTED);
            BtA2dpOpenOutputStream();
            break;
        case A2DP_SOURCE_DISCONNECTED_CB:
            fprintf(stdout, "A2DP Source DisConnected \n");
            media_playing = false;
            playStatus = BTRC_PLAYSTATE_ERROR;
            mCurrentTrackID = NO_TRACK_SELECTED;
            pA2dpSource->mAbsVolRemoteSupported = false;
            BtA2dpCloseOutputStream();
            memset(&mConnectedDevice, 0, sizeof(bt_bdaddr_t));
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            change_state(STATE_A2DP_SOURCE_DISCONNECTED);
            break;
        case A2DP_SOURCE_API_CONNECT_REQ:
            bdaddr_to_string(&mConnectingDevice, str, 18);
            fprintf(stdout, "A2DP Source already Connecting to %s\n", str);
            break;
        case A2DP_SOURCE_API_DISCONNECT_REQ:
            fprintf(stdout, "A2DP Source Disconnect can not be processed\n");
            break;
        case A2DP_SOURCE_CONNECTION_PRIORITY_REQ:
            if (sBtA2dpSourceVendorInterface != NULL) {
                sBtA2dpSourceVendorInterface->allow_connection_vendor(1, &pEvent->a2dpSourceEvent.bd_addr);
            }
            break;
        case A2DP_SOURCE_CODEC_CONFIG_CB:
            memcpy(&mDevice, &pEvent->a2dpSourceEvent.bd_addr, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&mDevice, str, 18);
            fprintf(stdout, "Codec Configuration for device %s\n", str);
             if (pEvent->a2dpSourceEvent.buf_ptr == NULL) {
                 break;
             }
             switch (pEvent->a2dpSourceEvent.arg1) {
                case A2DP_SOURCE_AUDIO_CODEC_SBC: {
                    fprintf(stdout, "Codec type = SBC \n");
                    btav_codec_config_t *codec_config = (btav_codec_config_t *)
                        pEvent->a2dpSourceEvent.buf_ptr;
                    freq = pA2dpSource->get_a2dp_sbc_sampling_rate(
                        (codec_config->sbc_config.samp_freq));
                    mode = pA2dpSource->get_a2dp_sbc_channel_mode(
                        (codec_config->sbc_config.ch_mode));
                }
                    break;
                case A2DP_SOURCE_AUDIO_CODEC_APTX: {
                    fprintf(stdout, "Codec type = APTX\n");
                    btav_codec_config_t *codec_config = (btav_codec_config_t *)
                        pEvent->a2dpSourceEvent.buf_ptr;
                    freq = pA2dpSource->get_a2dp_aptx_sampling_rate(
                        (codec_config->aptx_config.sampling_freq));
                    mode = pA2dpSource->get_a2dp_aptx_channel_mode(
                        (codec_config->aptx_config.channel_count));
                }
                    break;
                default:
                    is_valid_codec = false;
                    ALOGE(LOGTAG_A2DP " Invalid codec type %d ", pEvent->a2dpSourceEvent.arg1);
                    break;
             }
             if (is_valid_codec) {
                 fprintf(stdout, "Sample Rate = %d\n", freq);
                 fprintf(stdout, "Channel Mode = %s\n", mode);
                 if (pEvent->a2dpSourceEvent.arg1 == A2DP_SOURCE_AUDIO_CODEC_SBC) {
                     btav_codec_config_t *codec_config = (btav_codec_config_t *)
                         pEvent->a2dpSourceEvent.buf_ptr;
                     fprintf(stdout, "Block Len = %d\n",
                        pA2dpSource->get_a2dp_sbc_block_len(codec_config->sbc_config.block_len));
                     fprintf(stdout, "Num of Subbands = %d\n",
                        pA2dpSource->get_a2dp_sbc_sub_band(codec_config->sbc_config.num_subbands));
                     fprintf(stdout, "Allocation Method = %s\n",
                        pA2dpSource->get_a2dp_sbc_allocation_mth(codec_config->sbc_config.alloc_mthd));
                     fprintf(stdout, "Min Bitpool = %d\n", codec_config->sbc_config.min_bitpool);
                     fprintf(stdout, "Max Bitpool = %d\n", codec_config->sbc_config.max_bitpool);
                 }
             }
             osi_free(pEvent->a2dpSourceEvent.buf_ptr);
             break;
        default:
            ALOGE(LOGTAG_A2DP " event not handled %d ", pEvent->event_id);
            break;
    }
}

void A2dp_Source::state_connected_handler(BtEvent* pEvent) {
    char str[18];
    bt_bdaddr_t mDevice;
    uint32_t freq;
    char *mode;
    bool is_valid_codec = true;
    BtEvent *pControlRequest, *pReleaseControlReq;
    ALOGD(LOGTAG_A2DP "state_connected_handler Processing event %s", dump_message(pEvent->event_id));
    switch(pEvent->event_id) {
        case A2DP_SOURCE_API_CONNECT_REQ:
            bdaddr_to_string(&mConnectedDevice, str, 18);
            fprintf(stdout, "A2DP Source Already Connected to %s\n", str);
            break;
        case A2DP_SOURCE_API_DISCONNECT_REQ:
            if (memcmp(&mConnectedDevice, &pEvent->a2dpSourceEvent.bd_addr, sizeof(bt_bdaddr_t)))
            {
                bdaddr_to_string(&pEvent->a2dpSourceEvent.bd_addr, str, 18);
                fprintf(stdout, "Device not connected: %s\n", str);
                break;
            }
            bdaddr_to_string(&mConnectedDevice, str, 18);
            fprintf(stdout, "A2DP Source DisConnecting: %s\n", str);
            media_playing = false;
            playStatus = BTRC_PLAYSTATE_ERROR;
            mCurrentTrackID = NO_TRACK_SELECTED;
            pA2dpSource->mAbsVolRemoteSupported = false;
            if (sBtA2dpSourceInterface != NULL) {
                sBtA2dpSourceInterface->disconnect(&pEvent->a2dpSourceEvent.bd_addr);
            }
            change_state(STATE_A2DP_SOURCE_PENDING);
            break;
        case A2DP_SOURCE_CONNECTION_PRIORITY_REQ:
            if (sBtA2dpSourceVendorInterface != NULL) {
                sBtA2dpSourceVendorInterface->allow_connection_vendor(1, &pEvent->a2dpSourceEvent.bd_addr);
            }
            break;
        case A2DP_SOURCE_DISCONNECTED_CB:
            media_playing = false;
            playStatus = BTRC_PLAYSTATE_ERROR;
            mCurrentTrackID = NO_TRACK_SELECTED;
            pA2dpSource->mAbsVolRemoteSupported = false;
            BtA2dpCloseOutputStream();
            memset(&mConnectedDevice, 0, sizeof(bt_bdaddr_t));
            memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            fprintf(stdout, "A2DP Source DisConnected \n");
            change_state(STATE_A2DP_SOURCE_DISCONNECTED);
            break;
        case A2DP_SOURCE_DISCONNECTING_CB:
            fprintf(stdout, "A2DP Source DisConnecting \n");
            change_state(STATE_A2DP_SOURCE_PENDING);
            break;
        case A2DP_SOURCE_AUDIO_STARTED:
            fprintf(stdout, "A2DP Source Audio state changes to: %d	\n",pEvent->event_id);
            break;

        case A2DP_SOURCE_AUDIO_SUSPENDED:
            fprintf(stdout, "A2DP Source Audio state changes to: %d	\n",pEvent->event_id);
            break;
        case A2DP_SOURCE_AUDIO_STOPPED:
            fprintf(stdout, "A2DP Source Audio state changes to: %d ", pEvent->event_id);
            break;
        case A2DP_SOURCE_CODEC_CONFIG_CB:
            memcpy(&mDevice, &pEvent->a2dpSourceEvent.bd_addr, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&mDevice, str, 18);
            fprintf(stdout, "Codec Configuration for device %s\n", str);
             if (pEvent->a2dpSourceEvent.buf_ptr == NULL) {
                 break;
             }
             switch (pEvent->a2dpSourceEvent.arg1) {
                case A2DP_SOURCE_AUDIO_CODEC_SBC: {
                    fprintf(stdout, "Codec type = SBC \n");
                    btav_codec_config_t *codec_config = (btav_codec_config_t *)
                        pEvent->a2dpSourceEvent.buf_ptr;
                    freq = pA2dpSource->get_a2dp_sbc_sampling_rate(
                        (codec_config->sbc_config.samp_freq));
                    mode = pA2dpSource->get_a2dp_sbc_channel_mode(
                        (codec_config->sbc_config.ch_mode));
                }
                    break;
                case A2DP_SOURCE_AUDIO_CODEC_APTX: {
                    fprintf(stdout, "Codec type = APTX \n");
                    btav_codec_config_t *codec_config = (btav_codec_config_t *)
                        pEvent->a2dpSourceEvent.buf_ptr;
                    freq = pA2dpSource->get_a2dp_aptx_sampling_rate(
                        (codec_config->aptx_config.sampling_freq));
                    mode = pA2dpSource->get_a2dp_aptx_channel_mode(
                        (codec_config->aptx_config.channel_count));
                }
                    break;
                default:
                    is_valid_codec = false;
                    ALOGE(LOGTAG_A2DP " Invalid codec type %d ", pEvent->a2dpSourceEvent.arg1);
                    break;
             }
             if (is_valid_codec) {
                 fprintf(stdout, "Sample Rate = %d\n", freq);
                 fprintf(stdout, "Channel Mode = %s\n", mode);
                 if (pEvent->a2dpSourceEvent.arg1 == A2DP_SOURCE_AUDIO_CODEC_SBC) {
                     btav_codec_config_t *codec_config = (btav_codec_config_t *)
                         pEvent->a2dpSourceEvent.buf_ptr;
                     fprintf(stdout, "Block Len = %d\n",
                        pA2dpSource->get_a2dp_sbc_block_len(codec_config->sbc_config.block_len));
                     fprintf(stdout, "Num of Subbands = %d\n",
                        pA2dpSource->get_a2dp_sbc_sub_band(codec_config->sbc_config.num_subbands));
                     fprintf(stdout, "Allocation Method = %s\n",
                        pA2dpSource->get_a2dp_sbc_allocation_mth(codec_config->sbc_config.alloc_mthd));
                     fprintf(stdout, "Min Bitpool = %d\n", codec_config->sbc_config.min_bitpool);
                     fprintf(stdout, "Max Bitpool = %d\n", codec_config->sbc_config.max_bitpool);
                 }
             }
             osi_free(pEvent->a2dpSourceEvent.buf_ptr);
             break;
        default:
            fprintf(stdout, "Event not processed in connected state %d ", pEvent->event_id);
            ALOGE(LOGTAG_A2DP " event not handled %d ", pEvent->event_id);
            break;
    }
}

A2dpSourceState A2dp_Source::get_state() {
   ALOGD(LOGTAG_A2DP "current state changed to %d ", mSourceState);
   return mSourceState;
}

bool A2dp_Source::get_codec_cfg(uint8_t* info, uint8_t* type)
{
    return sBtA2dpSourceVendorInterface-> get_src_codec_config(info,type);
}

void A2dp_Source::change_state(A2dpSourceState mState) {
   ALOGD(LOGTAG_A2DP " current State = %d, new state = %d", mSourceState, mState);
   pthread_mutex_lock(&lock);
   mSourceState = mState;
   pthread_mutex_unlock(&lock);
   ALOGD(LOGTAG_A2DP " state changed to %d ", mState);
}


void A2dp_Source::UpdateSupportedCodecs(uint8_t num_codec_cfgss) {
    bt_status_t status;
    if (sBtA2dpSourceVendorInterface != NULL) {
        status = sBtA2dpSourceVendorInterface->update_supported_codecs_param_vendor
            (a2dpSrcCodecList, num_codec_cfgss);
        if (BT_STATUS_SUCCESS != status) {
            ALOGE(LOGTAG_A2DP " UpdateSupportedCodecs: failed, status = %d", status);
            fprintf(stdout, "UpdateSupportedCodecs: failed, status = %d\n", status);
        }
    }
}

uint32_t A2dp_Source::get_a2dp_sbc_sampling_rate(uint8_t frequency) {
    uint32_t freq = 48000;
    switch (frequency) {
        case SBC_SAMP_FREQ_16:
            freq = 16000;
            break;
        case SBC_SAMP_FREQ_32:
            freq = 32000;
            break;
        case SBC_SAMP_FREQ_44:
            freq = 44100;
            break;
        case SBC_SAMP_FREQ_48:
            freq = 48000;
            break;
    }
    return freq;
}

char * A2dp_Source::get_a2dp_sbc_channel_mode(uint8_t channeltype) {
    switch (channeltype) {
        case SBC_CH_MONO:
            return "mono";
        case SBC_CH_DUAL:
            return "dual";
        case SBC_CH_STEREO:
            return "stereo";
        case SBC_CH_JOINT:
            return "joint";
    }
    return "NULL";
}

uint8_t A2dp_Source::get_a2dp_sbc_block_len(uint8_t blocklen) {
    switch (blocklen) {
        case SBC_BLOCKS_4:
            return 4;
        case SBC_BLOCKS_8:
            return 8;
        case SBC_BLOCKS_12:
            return 12;
        case SBC_BLOCKS_16:
            return 16;
    }
}

uint8_t A2dp_Source::get_a2dp_sbc_sub_band(uint8_t subband) {
    switch (subband) {
        case SBC_SUBBAND_4:
            return 4;
        case SBC_SUBBAND_8:
            return 8;
    }
}

char * A2dp_Source::get_a2dp_sbc_allocation_mth(uint8_t allocation) {
    switch (allocation) {
        case SBC_ALLOC_SNR:
            return "snr";
        case SBC_ALLOC_LOUDNESS:
            return "loudness";
    }
    return "NULL";
}

uint32_t A2dp_Source::get_a2dp_aptx_sampling_rate(uint8_t frequency) {
    uint32_t freq = 0;
    switch (frequency) {
        case APTX_SAMPLERATE_44100:
            freq = 44100;
            break;
        case APTX_SAMPLERATE_48000:
            freq = 48000;
            break;
    }
    return freq;
}

char * A2dp_Source::get_a2dp_aptx_channel_mode(uint8_t channel_count) {
    switch (channel_count) {
        case APTX_CHANNELS_MONO:
            return "mono";
        case APTX_CHANNELS_STEREO:
            return "stereo";
    }
    return "NULL";
}


A2dp_Source :: A2dp_Source(const bt_interface_t *bt_interface, config_t *config) {
    this->bluetooth_interface = bt_interface;
    this->config = config;
    sBtA2dpSourceInterface = NULL;
    sBtAvrcpTargetInterface = NULL;
    mSourceState = STATE_A2DP_SOURCE_NOT_STARTED;
    mAvrcpConnected = false;
    set_abs_volume_timer = alarm_new();
    set_play_postion_timer = alarm_new();
    abs_vol_timer = false;
    mVolCmdSetInProgress = false;
    mVolCmdAdjustInProgress = false;
    mInitialRemoteVolume = -1;
    mLastRemoteVolume = -1;
    mRemoteVolume = -1;
    mLastLocalVolume = -1;
    mLocalVolume = -1;
    mPreviousAddrPlayerId = 0;
    mCurrentAddrPlayerId = 0;
    mAbsVolRemoteSupported = false;
    memset(&mConnectedDevice, 0, sizeof(bt_bdaddr_t));
    memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
    memset(&mConnectedAvrcpDevice, 0, sizeof(bt_bdaddr_t));
    pthread_mutex_init(&this->lock, NULL);
    is_sink_relay_enabled = config_get_bool (config,
            CONFIG_DEFAULT_SECTION, "BtRelaySinkDatatoSrc", false);
    ALOGD(LOGTAG_A2DP " Sink Relay Enabled %d ", is_sink_relay_enabled);
}

A2dp_Source :: ~A2dp_Source() {
    mAvrcpConnected = false;
    mVolCmdSetInProgress = false;
    mVolCmdAdjustInProgress = false;
    mInitialRemoteVolume = -1;
    mLastRemoteVolume = -1;
    mRemoteVolume = -1;
    mLastLocalVolume = -1;
    mLocalVolume = -1;
    mPreviousAddrPlayerId = 0;
    mCurrentAddrPlayerId = 0;
    alarm_free(set_abs_volume_timer);
    alarm_free(set_play_postion_timer);
    set_abs_volume_timer = NULL;
    mAbsVolRemoteSupported = false;
    pthread_mutex_destroy(&lock);
}

MediaPlayerInfo :: MediaPlayerInfo(short playerId, char majorPlayerType, int playerSubType,
                                      char playState, short charsetId, short displayableNameLength,
                                      char* displayableName, char* playerPackageName,
                                      bool isAvailable, bool isFocussed, char itemType,
                                      bool isRemoteAddressable, short itemLength, short entryLength,
                                      char featureMask[]) {
    int i;
    mPlayerId = playerId;
    mMajorPlayerType = majorPlayerType;
    mPlayerSubType = playerSubType;
    mPlayState = playState;
    mCharsetId = charsetId;
    mDisplayableNameLength = displayableNameLength;
    memcpy(&mDisplayableName, &displayableName, strlen(displayableName)+1);
    memcpy(&mPlayerPackageName, &playerPackageName, strlen(playerPackageName)+1);
    ALOGD(LOGTAG_AVRCP "  %s %s", mDisplayableName, mPlayerPackageName);

    mIsAvailable = isAvailable;
    mIsFocussed = isFocussed;
    mItemType = itemType;
    mIsRemoteAddressable = isRemoteAddressable;
    mItemLength = (short)(mDisplayableNameLength + 2 + 1 + 4 + 1 + 2 + 2 + 16);
    mEntryLength = (short)(mItemLength + /* ITEM_LENGTH_LENGTH +*/ 1);
    memcpy(mFeatureMask, featureMask, 16);

    for (i = 0; i < 16; i++)
        ALOGD(LOGTAG_AVRCP " %d", mFeatureMask[i]);
}

int MediaPlayerInfo :: RetrievePlayerEntryLength() {
    return mEntryLength;
}

char* MediaPlayerInfo :: RetrievePlayerItemEntry() {
    int position = 0;
    int count;
    char* playerEntry1 = (char*)osi_malloc(mEntryLength * sizeof(char));

    playerEntry1[position] = (char)mItemType;
    ALOGD(LOGTAG_AVRCP "RetrievePlayerItemEntry type %d", playerEntry1[position]);
    position++;

    playerEntry1[position] = (char)(mPlayerId & 0xff);
    ALOGD(LOGTAG_AVRCP "RetrievePlayerItemEntry playerid %d", playerEntry1[position]);
    position++;

    playerEntry1[position] = (char)((mPlayerId >> 8) & 0xff);
    ALOGD(LOGTAG_AVRCP "RetrievePlayerItemEntry playerid %d", playerEntry1[position]);
    position++;

    playerEntry1[position] = (char)mMajorPlayerType;
    ALOGD(LOGTAG_AVRCP "RetrievePlayerItemEntry MajorPlayerType %d", playerEntry1[position]);
    position++;

    for (count = 0; count < 4; count++) {
        playerEntry1[position] = (char)((mPlayerSubType >> (8 * count)) & 0xff); position++;
    }

    playerEntry1[position] = (char)mPlayState; position++;
    for (count = 0; count < 16; count++) {
        playerEntry1[position] = (char)mFeatureMask[count];
        ALOGD(LOGTAG_AVRCP "RetrievePlayerItemEntry playerEntry1[%d] position %d,  %d", count,
                            position, playerEntry1[position]);
        position++;
    }
    playerEntry1[position] = (char)(mCharsetId & 0xff); position++;
    playerEntry1[position] = (char)((mCharsetId >> 8) & 0xff); position++;
    playerEntry1[position] = (char)(mDisplayableNameLength & 0xff); position++;
    playerEntry1[position] = (char)((mDisplayableNameLength >> 8) & 0xff); position++;

    for (count = 0; count < mDisplayableNameLength; count++) {
        playerEntry1[position] = (char)mDisplayableName[count]; position++;
    }
    if (position != mEntryLength) {
        ALOGE(LOGTAG_AVRCP "ERROR populating PlayerItemEntry: position: %d mEntryLength: %d",
                            position, mEntryLength);
    }
    return playerEntry1;
}

MediaPlayerInfo :: ~MediaPlayerInfo() {
}

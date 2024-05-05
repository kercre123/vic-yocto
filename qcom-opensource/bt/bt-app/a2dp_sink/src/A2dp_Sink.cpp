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

#include <list>
#include <map>
#include <iostream>
#include <string.h>
#include <hardware/bluetooth.h>
#include <hardware/hardware.h>
#include <hardware/bt_av.h>
#include "A2dp_Sink_Streaming.hpp"
#include "A2dp_Sink.hpp"
#include "Avrcp.hpp"
#include "Gap.hpp"
#include "hardware/bt_av_vendor.h"
#include <algorithm>
#include "oi_utils.h"

#define LOGTAG "A2DP_SINK"

using namespace std;
using std::list;
using std::string;

A2dp_Sink *pA2dpSink = NULL;
A2dp_Sink_Streaming *pA2dpSinkStream;
extern Avrcp *pAvrcp;
extern void flush_relay_data(void);
static const bt_bdaddr_t bd_addr_null= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
#define SBC_PARAM_LEN 1
#define APTX_PARAM_LEN 1
#define MP3_PARAM_LEN 2
#define AAC_PARAM_LEN 2

static btav_codec_configuration_t a2dpSnkCodecList[MAX_NUM_CODEC_CONFIGS];

static const char * valid_codecs[] = {
    "aac",
    "mp3",
    "sbc",
    "aptx"
};

static uint8_t valid_codec_values[] = {
    A2DP_SINK_AUDIO_CODEC_AAC,
    A2DP_SINK_AUDIO_CODEC_MP3,
    A2DP_SINK_AUDIO_CODEC_SBC,
    A2DP_SINK_AUDIO_CODEC_APTX,
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

static const char * valid_aac_freq[] = {
    "8",
    "11.025",
    "12",
    "16",
    "22.05",
    "24",
    "32",
    "44.1",
    "48",
    "64",
    "88.2",
    "96",
};

static uint16_t valid_aac_freq_values[] = {
    AAC_SAMP_FREQ_8000,
    AAC_SAMP_FREQ_11025,
    AAC_SAMP_FREQ_12000,
    AAC_SAMP_FREQ_16000,
    AAC_SAMP_FREQ_22050,
    AAC_SAMP_FREQ_24000,
    AAC_SAMP_FREQ_32000,
    AAC_SAMP_FREQ_44100,
    AAC_SAMP_FREQ_48000,
    AAC_SAMP_FREQ_64000,
    AAC_SAMP_FREQ_88200,
    AAC_SAMP_FREQ_96000,
};

static const char * valid_aac_obj_type[] = {
    "MPEG-2-LC",
    "MPEG-4-LC",
    "MPEG-4-LTP",
    "MPEG-4-SC"
};

static uint8_t valid_aac_obj_type_values[] = {
    AAC_OBJ_TYPE_MPEG_2_AAC_LC,
    AAC_OBJ_TYPE_MPEG_4_AAC_LC,
    AAC_OBJ_TYPE_MPEG_4_AAC_LTP,
    AAC_OBJ_TYPE_MPEG_4_AAC_SCA,
};

static const char * valid_mp3_freq[] = {
    "16",
    "22.05",
    "24",
    "32",
    "44.1",
    "48",
};

static uint8_t valid_mp3_freq_values[] = {
    MP3_SAMP_FREQ_16000,
    MP3_SAMP_FREQ_22050,
    MP3_SAMP_FREQ_24000,
    MP3_SAMP_FREQ_32000,
    MP3_SAMP_FREQ_44100,
    MP3_SAMP_FREQ_48000,
};

static const char * valid_mp3_layer[] = {
    "LAYER1",
    "LAYER2",
    "LAYER3",
};

static uint8_t valid_mp3_layer_values[] = {
    MP3_LAYER_1,
    MP3_LAYER_2,
    MP3_LAYER_3,
};

static const char * valid_aptx_freq[] = {
    "44.1",
    "48",
};

static uint8_t valid_aptx_freq_values[] = {
    APTX_SAMPLERATE_44100,
    APTX_SAMPLERATE_48000,
};

/******************************************************************************
 * This structure defines the A2DP Sink variable.
 */
typedef struct {
    const char *name;            /**< Run-time variable name */
    const char *description;     /**< Run-time variable description */
    const char **valid_options;  /**< List of valid variable values */
    int valid_option_cnt;        /**< Size of valid value list */
} A2DP_SINK_VARIABLE;

/******************************************************************************
 * List of A2DP Sink variables.
 */
const A2DP_SINK_VARIABLE variable_list[] = {
    { "codec type", "Valid Codec Type to Use",
      valid_codecs, _ARRAYSIZE(valid_codecs) },
    { "sbc freq", "Valid SBC Freq to Use",
      valid_sbc_freq, _ARRAYSIZE(valid_sbc_freq) },
    { "aac freq", "Valid AAC Freq to Use",
      valid_aac_freq, _ARRAYSIZE(valid_aac_freq) },
    { "mp3 freq", "Valid MP3 Freq to Use",
      valid_mp3_freq, _ARRAYSIZE(valid_mp3_freq) },
    { "aptx freq", "Valid APTX Freq to Use",
      valid_aptx_freq, _ARRAYSIZE(valid_aptx_freq) },
    { "aac object type", "Valid AAC Object Type to Use",
      valid_aac_obj_type, _ARRAYSIZE(valid_aac_obj_type) },
    { "mp3 layer", "Valid MP3 Layer to Use",
      valid_mp3_layer, _ARRAYSIZE(valid_mp3_layer) },
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

#ifndef ishyphon
#define ishyphon(c) (c == '-')
#endif

static int find_str_in_list(const char *str, const char * const *list,
                                   int list_size)
{
    int i;
    int item = list_size;
    int match_cnt = 0;

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

static void print_help(const A2DP_SINK_VARIABLE *var)
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

    while (*data && !ishyphon(*data)) {
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
        ALOGE(LOGTAG " %s ", output[param_count -1]);
    }

    while ((temp_arg = strtok_r(NULL, delim, &ptr1))) {
        if (param_count >= MAX_NUM_CODEC_CONFIGS) {
            return param_count;
        }
        strlcpy(output[param_count], temp_arg, COMMAND_ARG_SIZE);
        output[param_count ++][COMMAND_ARG_SIZE - 1] = '\0';
        ALOGE(LOGTAG " %s ", output[param_count -1]);
    }

    ALOGE(LOGTAG " %s: returning %d \n", __func__, param_count);
    return param_count;
}


/* This function is used for testing purpose. Parses string which represents codec list*/

static bool A2dpCodecList(char *codec_param_list, int *num_codec_configs)
{
    int i = 0, j = 0, k = 0;
    char output_list[COMMAND_ARG_SIZE][COMMAND_ARG_SIZE];
    int codec_params_list_size;

    if (*codec_param_list == '\0') {
        ALOGE(LOGTAG " codec list cannot be set to nothing \n");
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
        a2dpSnkCodecList[k].codec_type = valid_codec_values[i];
        switch (a2dpSnkCodecList[k].codec_type) {
            case A2DP_SINK_AUDIO_CODEC_AAC:
                /* check number of parameters passed are ok or not */
                if (j + AAC_PARAM_LEN > codec_params_list_size) {
                    fprintf(stdout, "Invalid AAC Parameters passed\n");
                    return false;
                }
                i = find_str_in_list(output_list[j], valid_aac_freq,
                    _ARRAYSIZE(valid_aac_freq));
                if (i >= _ARRAYSIZE(valid_aac_freq)) {
                    fprintf(stdout, "Invalid AAC Sampling Freq: %s\n", output_list[j]);
                    print_help(&variable_list[2]);
                    return false;
                }
                a2dpSnkCodecList[k].codec_config.aac_config.sampling_freq =
                    valid_aac_freq_values[i];
                j ++;
                i = find_str_in_list(output_list[j], valid_aac_obj_type,
                    _ARRAYSIZE(valid_aac_obj_type));
                if (i >= _ARRAYSIZE(valid_aac_obj_type)) {
                    fprintf(stdout, "Invalid AAC Object Type: %s\n", output_list[j]);
                    print_help(&variable_list[5]);
                    return false;
                }
                a2dpSnkCodecList[k].codec_config.aac_config.obj_type =
                    valid_aac_obj_type_values[i];
                j ++;
                break;
            case A2DP_SINK_AUDIO_CODEC_MP3:
                /* check number of parameters passed are ok or not */
                if (j + MP3_PARAM_LEN > codec_params_list_size) {
                    fprintf(stdout, "Invalid MP3 Parameters passed\n");
                    return false;
                }
                i = find_str_in_list(output_list[j], valid_mp3_freq,
                    _ARRAYSIZE(valid_mp3_freq));
                if (i >= _ARRAYSIZE(valid_mp3_freq)) {
                    fprintf(stdout, "Invalid MP3 Sampling Freq: %s\n",
                        output_list[j]);
                    print_help(&variable_list[3]);
                    return false;
                }
                a2dpSnkCodecList[k].codec_config.mp3_config.sampling_freq =
                    valid_mp3_freq_values[i];
                j ++;
                i = find_str_in_list(output_list[j], valid_mp3_layer,
                    _ARRAYSIZE(valid_mp3_layer));
                if (i >= _ARRAYSIZE(valid_mp3_layer)) {
                    fprintf(stdout, "Invalid MP3 Layer: %s\n", output_list[j]);
                    print_help(&variable_list[6]);
                    return false;
                }
                a2dpSnkCodecList[k].codec_config.mp3_config.layer =
                    valid_mp3_layer_values[i];
                j ++;
                break;
            case A2DP_SINK_AUDIO_CODEC_SBC:
                /* check number of parameters passed are ok or not */
                if (j + SBC_PARAM_LEN > codec_params_list_size) {
                    fprintf(stdout, "Invalid SBC Parameters passed\n");
                    return false;
                }
                i = find_str_in_list(output_list[j], valid_sbc_freq,
                    _ARRAYSIZE(valid_sbc_freq));
                if (i >= _ARRAYSIZE(valid_sbc_freq)) {
                    fprintf(stdout, "Invalid SBC Sampling Freq: %s\n",
                        output_list[j]);
                    print_help(&variable_list[1]);
                    return false;
                }
                a2dpSnkCodecList[k].codec_config.sbc_config.samp_freq =
                    valid_sbc_freq_values[i];
                j ++;
                break;
            case A2DP_SINK_AUDIO_CODEC_APTX:
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
                    print_help(&variable_list[4]);
                    return false;
                }
                a2dpSnkCodecList[k].codec_config.aptx_config.sampling_freq =
                    valid_aptx_freq_values[i];
                j ++;
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

void BtA2dpSinkMsgHandler(void *msg) {
    BtEvent* pEvent = NULL;
    BtEvent* pCleanupEvent = NULL;
    BtEvent *pCleanupSinkStreaming = NULL;
    int num_codec_configs = 0;
    if(!msg) {
        printf("Msg is NULL, return.\n");
        return;
    }

    pEvent = ( BtEvent *) msg;
    switch(pEvent->event_id) {
        case PROFILE_API_START:
            ALOGD(LOGTAG " enable a2dp sink");
            if (pA2dpSink) {
                pA2dpSink->HandleEnableSink();
            }
            break;
        case PROFILE_API_STOP:
            ALOGD(LOGTAG " disable a2dp sink");
            if (pA2dpSink) {
                pA2dpSink->HandleDisableSink();
            }
            break;
        case A2DP_SINK_CLEANUP_REQ:
            ALOGD(LOGTAG " cleanup a2dp sink");
            pCleanupSinkStreaming = new BtEvent;
            pCleanupSinkStreaming->a2dpSinkStreamingEvent.event_id =
                    A2DP_SINK_STREAMING_CLEANUP_REQ;
            if (pA2dpSinkStream) {
                thread_post(pA2dpSinkStream->threadInfo.thread_id,
                pA2dpSinkStream->threadInfo.thread_handler, (void*)pCleanupSinkStreaming);
            }
            pCleanupEvent = new BtEvent;
            pCleanupEvent->event_id = A2DP_SINK_CLEANUP_DONE;
            PostMessage(THREAD_ID_GAP, pCleanupEvent);
            break;
        case A2DP_SINK_STREAMING_DISABLE_DONE:
            ALOGD(LOGTAG " a2dp sink streaming disable done");
            if (pA2dpSink) {
                pA2dpSink->HandleSinkStreamingDisableDone();
            }
            break;
        case A2DP_SINK_CODEC_LIST:
            A2dpCodecList(pEvent->a2dpCodecListEvent.codec_list,
                &num_codec_configs);
            if (num_codec_configs)
                pA2dpSink->UpdateSupportedCodecs(num_codec_configs);
            break;
        default:
            if(pA2dpSink) {
               pA2dpSink->EventManager(( BtEvent *) msg, pEvent->a2dpSinkEvent.bd_addr);
            }
            break;
    }
    delete pEvent;
}

#ifdef __cplusplus
}
#endif

list<A2dp_Device>::iterator FindDeviceByAddr(list<A2dp_Device>& pA2dpDev, bt_bdaddr_t dev) {
    list<A2dp_Device>::iterator p = pA2dpDev.begin();
    while(p != pA2dpDev.end()) {
        if (memcmp(&dev, &p->mDevice, sizeof(bt_bdaddr_t)) == 0) {
            break;
        }
        p++;
    }
    return p;
}

bool GetCodecInfoByAddr(bt_bdaddr_t* bd_addr, uint16_t *dev_codec_type, btav_codec_config_t* codec_config)
{
    ALOGD(LOGTAG "enter func GetCodecINfo ===>");
    if(bd_addr == NULL)
    {
        bd_addr= &pA2dpSinkStream->mStreamingDevice;
        ALOGD(LOGTAG " check the steramding device codec");
        if (!memcmp(&pA2dpSinkStream->mStreamingDevice, &bd_addr_null, sizeof(bt_bdaddr_t)))
        {
            ALOGD(LOGTAG " the steaming device is empty ");
            return false;
        }
    }
    list<A2dp_Device>::iterator iter = FindDeviceByAddr(pA2dpSink->pA2dpDeviceList, *bd_addr);
    if(iter != pA2dpSink->pA2dpDeviceList.end())
    {
        ALOGD(LOGTAG " Audio Config CB: found matching device");
        *dev_codec_type = iter->dev_codec_type;
        memcpy((void*)codec_config,(void *) &iter->dev_codec_config, sizeof(iter->dev_codec_config));
        //for(int i=0;i<7;i++)
        ALOGE(LOGTAG "codec_type = %u", *dev_codec_type);
        return true;
    }
    else
    {
        ALOGE(LOGTAG " ERROR: Audio Config CB: No matching device");
        return false;
    }
}

static void bta2dp_connection_state_callback(btav_connection_state_t state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG " Connection State CB state = %d", state);
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->a2dpSinkEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    switch( state ) {
        case BTAV_CONNECTION_STATE_DISCONNECTED:
            pEvent->a2dpSinkEvent.event_id = A2DP_SINK_DISCONNECTED_CB;
        break;
        case BTAV_CONNECTION_STATE_CONNECTING:
            pEvent->a2dpSinkEvent.event_id = A2DP_SINK_CONNECTING_CB;
        break;
        case BTAV_CONNECTION_STATE_CONNECTED:
            pEvent->a2dpSinkEvent.event_id = A2DP_SINK_CONNECTED_CB;
        break;
        case BTAV_CONNECTION_STATE_DISCONNECTING:
            pEvent->a2dpSinkEvent.event_id = A2DP_SINK_DISCONNECTING_CB;
        break;
    }
    PostMessage(THREAD_ID_A2DP_SINK, pEvent);
}

static void bta2dp_audio_state_callback(btav_audio_state_t state, bt_bdaddr_t* bd_addr) {
    ALOGD(LOGTAG " Audio State CB state = %d", state);
    BtEvent *pEvent = new BtEvent;
    memcpy(&pEvent->a2dpSinkEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    switch( state ) {
        case BTAV_AUDIO_STATE_REMOTE_SUSPEND:
            pEvent->a2dpSinkEvent.event_id = A2DP_SINK_AUDIO_SUSPENDED;
        break;
        case BTAV_AUDIO_STATE_STOPPED:
            pEvent->a2dpSinkEvent.event_id = A2DP_SINK_AUDIO_STOPPED;
        break;
        case BTAV_AUDIO_STATE_STARTED:
            pEvent->a2dpSinkEvent.event_id = A2DP_SINK_AUDIO_STARTED;
        break;
    }
    PostMessage(THREAD_ID_A2DP_SINK, pEvent);
}

static void bta2dp_audio_config_callback(bt_bdaddr_t *bd_addr, uint32_t sample_rate,
        uint8_t channel_count) {
    ALOGD(LOGTAG " Audio Config CB sample_rate %d, channel_count %d", sample_rate, channel_count);
    list<A2dp_Device>::iterator iter = FindDeviceByAddr(pA2dpSink->pA2dpDeviceList, *bd_addr);
    if(iter != pA2dpSink->pA2dpDeviceList.end())
    {
        ALOGD(LOGTAG " Audio Config CB: found matching device");
        iter->av_config.sample_rate = sample_rate;
        iter->av_config.channel_count = channel_count;
    }
    else
    {
        ALOGE(LOGTAG " ERROR: Audio Config CB: No matching device");
    }
}

static void bta2dp_audio_data_read_callback(bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG " Audio Data Read Callback");
    BtEvent *pA2dpDataRead = new BtEvent;
    memcpy(&pA2dpDataRead->a2dpSinkEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    if (pA2dpSinkStream && pA2dpSinkStream->codec_type == A2DP_SINK_AUDIO_CODEC_SBC) {
        pA2dpDataRead->a2dpSinkStreamingEvent.event_id =
                A2DP_SINK_STREAMING_FETCH_PCM_DATA;
        if (pA2dpSinkStream) {
            thread_post(pA2dpSinkStream->threadInfo.thread_id,
                    pA2dpSinkStream->threadInfo.thread_handler, (void*)pA2dpDataRead);
        }
    } else {
        pA2dpDataRead->a2dpSinkStreamingEvent.event_id = A2DP_SINK_FILL_COMPRESS_BUFFER;
        if (pA2dpSinkStream) {
            thread_post(pA2dpSinkStream->threadInfo.thread_id
                    , pA2dpSinkStream->threadInfo.thread_handler, (void*)pA2dpDataRead);
        }
    }
}
static void bta2dp_audio_focus_request_vendor_callback(bt_bdaddr_t *bd_addr) {
    ALOGD(LOGTAG " bta2dp_audio_focus_request_vendor_callback ");
    BtEvent *pEvent = new BtEvent;
    pEvent->a2dpSinkEvent.event_id = A2DP_SINK_FOCUS_REQUEST_CB;
    memcpy(&pEvent->a2dpSinkEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    PostMessage(THREAD_ID_A2DP_SINK, pEvent);
}

static void bta2dp_audio_codec_config_vendor_callback(bt_bdaddr_t *bd_addr, uint16_t codec_type,
        btav_codec_config_t codec_config) {
    ALOGD(LOGTAG " bta2dp_audio_codec_config_vendor_callback codec_type=%d",codec_type);

    BtEvent *pEvent = new BtEvent;
    pEvent->a2dpSinkEvent.event_id = A2DP_SINK_CODEC_CONFIG;
    memcpy(&pEvent->a2dpSinkEvent.bd_addr, bd_addr, sizeof(bt_bdaddr_t));
    pEvent->a2dpSinkEvent.buf_size = sizeof(btav_codec_config_t);
    pEvent->a2dpSinkEvent.buf_ptr = (uint8_t*)osi_malloc(pEvent->a2dpSinkEvent.buf_size);
    memcpy(pEvent->a2dpSinkEvent.buf_ptr, &codec_config, pEvent->a2dpSinkEvent.buf_size);
    pEvent->a2dpSinkEvent.arg1 = codec_type;
    PostMessage(THREAD_ID_A2DP_SINK, pEvent);
}

static btav_callbacks_t sBluetoothA2dpSinkCallbacks = {
    sizeof(sBluetoothA2dpSinkCallbacks),
    bta2dp_connection_state_callback,
    bta2dp_audio_state_callback,
    bta2dp_audio_config_callback,
};

static btav_sink_vendor_callbacks_t sBluetoothA2dpSinkVendorCallbacks = {
    sizeof(sBluetoothA2dpSinkVendorCallbacks),
    bta2dp_audio_focus_request_vendor_callback,
    bta2dp_audio_codec_config_vendor_callback,
    bta2dp_audio_data_read_callback,
};

void A2dp_Sink::HandleEnableSink(void) {
    ALOGD(LOGTAG " HandleEnableSink ");

    uint8_t streaming_prarm = 0;
    BtEvent *pEvent = new BtEvent;
    max_a2dp_conn = config_get_int (config,
            CONFIG_DEFAULT_SECTION, "BtMaxA2dpConn", 1);

    if (bluetooth_interface != NULL)
    {
        sBtA2dpSinkInterface = (btav_interface_t *)bluetooth_interface->
                get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_SINK_ID);
        sBtA2dpSinkVendorInterface = (btav_sink_vendor_interface_t *)bluetooth_interface->
                get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_SINK_VENDOR_ID);

        if (sBtA2dpSinkInterface == NULL)
        {
             pEvent->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
             pEvent->profile_start_event.profile_id = PROFILE_ID_A2DP_SINK;
             pEvent->profile_start_event.status = false;
             PostMessage(THREAD_ID_GAP, pEvent);
             return;
        }
        pA2dpSink->mSinkState = SINK_STATE_STARTED;
        pA2dpSinkStream->fetch_rtp_info = config_get_bool (config,
                         CONFIG_DEFAULT_SECTION, "BtFetchRTPForSink", false);
        pA2dpSinkStream->sbc_decoding = config_get_bool (config,
            CONFIG_DEFAULT_SECTION, "BtEnableSBCDecoding", true);
        pA2dpSinkStream->enable_notification_cb = config_get_bool (config,
                         CONFIG_DEFAULT_SECTION, "BtMediaNotificationCb", false);
        ALOGD(LOGTAG " Fetch RTP Info %d, enable_notification_cb: %d",
                pA2dpSinkStream->fetch_rtp_info, pA2dpSinkStream->enable_notification_cb);

        pA2dpSinkStream->enable_delay_report = config_get_bool (config,CONFIG_DEFAULT_SECTION, "BtA2dpDelayReportEnable", false);
        ALOGD(LOGTAG " ~~ enable_delay_report  %d ", pA2dpSinkStream->enable_delay_report);
#ifdef USE_LIBHW_AOSP
        sBtA2dpSinkInterface->init(&sBluetoothA2dpSinkCallbacks);
#else
        sBtA2dpSinkInterface->init(&sBluetoothA2dpSinkCallbacks, max_a2dp_conn, 0);
#endif
        if (pA2dpSinkStream->fetch_rtp_info)
            streaming_prarm |= A2DP_SINK_RETREIVE_RTP_HEADER;
        if(pA2dpSinkStream->sbc_decoding)
            streaming_prarm |= A2DP_SINK_ENABLE_SBC_DECODING;
        if (pA2dpSinkStream->enable_delay_report)
            streaming_prarm |= A2DP_SINK_ENABLE_DELAY_REPORTING;
        if (pA2dpSinkStream->enable_notification_cb)
            streaming_prarm |= A2DP_SINK_ENABLE_NOTIFICATION_CB;

        sBtA2dpSinkVendorInterface->init_vendor(&sBluetoothA2dpSinkVendorCallbacks,
                    max_a2dp_conn, 0,
                    streaming_prarm);

        pEvent->profile_start_event.event_id = PROFILE_EVENT_START_DONE;
        pEvent->profile_start_event.profile_id = PROFILE_ID_A2DP_SINK;
        pEvent->profile_start_event.status = true;
        pA2dpSinkStream->GetLibInterface(sBtA2dpSinkVendorInterface);

        PostMessage(THREAD_ID_GAP, pEvent);
    }
    BtEvent *pEnableSinkStreaming = new BtEvent;
    pEnableSinkStreaming->a2dpSinkStreamingEvent.event_id = A2DP_SINK_STREAMING_API_START;
    if (pA2dpSinkStream) {
        thread_post(pA2dpSinkStream->threadInfo.thread_id,
        pA2dpSinkStream->threadInfo.thread_handler, (void*)pEnableSinkStreaming);
    }
}

void A2dp_Sink::HandleDisableSink(void) {
    ALOGD(LOGTAG " HandleDisableSink ");
    pA2dpSink->mSinkState = SINK_STATE_NOT_STARTED;

    BtEvent *pDisableSinkStreaming = new BtEvent;
    pDisableSinkStreaming->a2dpSinkStreamingEvent.event_id = A2DP_SINK_STREAMING_API_STOP;
    if (pA2dpSinkStream) {
        thread_post(pA2dpSinkStream->threadInfo.thread_id,
        pA2dpSinkStream->threadInfo.thread_handler, (void*)pDisableSinkStreaming);
    }
}

void A2dp_Sink::HandleSinkStreamingDisableDone(void) {
    ALOGD(LOGTAG " HandleSinkStreamingDisableDone ");
    if (pA2dpSink->pA2dpDeviceList.size() != 0)
        pA2dpSink->pA2dpDeviceList.clear();

    if (sBtA2dpSinkInterface != NULL) {
        sBtA2dpSinkInterface->cleanup();
        sBtA2dpSinkInterface = NULL;
    }
    if (sBtA2dpSinkVendorInterface != NULL) {
        sBtA2dpSinkVendorInterface->cleanup_vendor();
        sBtA2dpSinkVendorInterface = NULL;
    }

    BtEvent *pEvent = new BtEvent;
    pEvent->profile_stop_event.event_id = PROFILE_EVENT_STOP_DONE;
    pEvent->profile_stop_event.profile_id = PROFILE_ID_A2DP_SINK;
    pEvent->profile_stop_event.status = true;
    PostMessage(THREAD_ID_GAP, pEvent);
}

void A2dp_Sink::ProcessEvent(BtEvent* pEvent, list<A2dp_Device>::iterator iter) {
    switch(iter->mSinkDeviceState) {
        case DEVICE_STATE_DISCONNECTED:
            state_disconnected_handler(pEvent, iter);
            break;
        case DEVICE_STATE_PENDING:
            state_pending_handler(pEvent, iter);
            break;
        case DEVICE_STATE_CONNECTED:
            state_connected_handler(pEvent, iter);
            break;
    }
}

void A2dp_Sink::ConnectionManager(BtEvent* pEvent, bt_bdaddr_t dev) {
    ALOGD(LOGTAG " ConnectionManager ");
    A2dp_Device *newNode = NULL;
    list<A2dp_Device>::iterator iter;

    switch(pEvent->event_id) {
        case A2DP_SINK_API_CONNECT_REQ:
            if (pA2dpDeviceList.size() == max_a2dp_conn) {
                ALOGE(LOGTAG " already max devices connected");
                fprintf(stdout, "Already %d device connected\n", max_a2dp_conn);
                return;
            }
            if (pA2dpDeviceList.size() != 0) {
                ALOGD(LOGTAG " Atleast 1 remote device connected/connecting ");
                iter = FindDeviceByAddr(pA2dpDeviceList, dev);
                if (iter != pA2dpDeviceList.end())
                {
                    ALOGE(LOGTAG " Connect req for already connected/connecting device");
                    fprintf(stdout, "Connect req for already connected/connecting device\n");
                    return;
                }
            }
            if (pA2dpDeviceList.size() < max_a2dp_conn) {
                ALOGD(LOGTAG " pA2dpDeviceList.size() < max_a2dp_conn ");
                pA2dpDeviceList.push_back(A2dp_Device(config, dev));
                iter = pA2dpDeviceList.end();
                --iter;
            }
            break;
        case A2DP_SINK_CONNECTING_CB:
        case A2DP_SINK_CONNECTED_CB:
            iter = FindDeviceByAddr(pA2dpDeviceList, dev);
            bdstr_t bd_str;
            if (iter != pA2dpDeviceList.end())
            {
                ALOGD(LOGTAG " found a match, donot alloc new");
            }
            else if (pA2dpDeviceList.size() < max_a2dp_conn)
            {
                ALOGD(LOGTAG " reached end of list without a match, alloc new");
                pA2dpDeviceList.push_back(A2dp_Device(config, dev));
                iter = pA2dpDeviceList.end();
                --iter;
            }
            else
            {
                ALOGE(LOGTAG " already max devices connected");
                fprintf(stdout, "Already %d device connected\n", max_a2dp_conn);
                return;
            }
            if (!pAvrcp->rc_only_devices.empty())
            {
                bdaddr_to_string(&iter->mDevice, &bd_str[0], sizeof(bd_str));
                std::string deviceAddress(bd_str);
                std::list<std::string>::iterator bdstring;
                bdstring = std::find(pAvrcp->rc_only_devices.begin(), pAvrcp->rc_only_devices.end(), deviceAddress);
                if (bdstring != pAvrcp->rc_only_devices.end())
                {
                    ALOGE(LOGTAG "RC already connected earlier for this AV connected device, set RC connected");
                    iter->mAvrcpConnected = true;
                    pAvrcp->rc_only_devices.remove(deviceAddress);
                }
                else
                {
                    ALOGE(LOGTAG "RC not already connected with this device ");
                }
            }
            break;
        case A2DP_SINK_API_DISCONNECT_REQ:
        case A2DP_SINK_DISCONNECTING_CB:
        case A2DP_SINK_DISCONNECTED_CB:
            if (pA2dpDeviceList.size() == 0) {
                ALOGE(LOGTAG " no device to disconnect");
                fprintf(stdout, "No device connected\n");
                return;
            }
            else
            {
                iter = FindDeviceByAddr(pA2dpDeviceList, dev);
                if (iter == pA2dpDeviceList.end())
                {
                    ALOGE(LOGTAG " reached end of list without a match, cannot disconnect");
                    return;
                }
                else
                {
                    ALOGE(LOGTAG " found a match, disconnect this device iter = %x", iter);
                    if (pA2dpSinkStream->relay_sink_data)
                    {
                        flush_relay_data();
                    }
                }
            }
            break;
    }
    ProcessEvent(pEvent, iter);
}

void A2dp_Sink::EventManager(BtEvent* pEvent, bt_bdaddr_t dev) {
    ALOGD(LOGTAG " EventManager ");

    if (pA2dpSink->mSinkState == SINK_STATE_NOT_STARTED)
    {
       ALOGE(LOGTAG " SINK STATE UNINITIALIZED, return");
       return;
    }

    if(isConnectionEvent(pEvent->event_id))
    {
        ConnectionManager(pEvent, dev);
    }
    else
    {
        list<A2dp_Device>::iterator iter = FindDeviceByAddr(pA2dpDeviceList, dev);
        if (iter != pA2dpDeviceList.end())
        {
            ProcessEvent(pEvent, iter);
        }
        else
        {
            ALOGE(LOGTAG " no matching device ignore process event");
        }
    }
}

void A2dp_Sink::UpdateSupportedCodecs(uint8_t num_codec_configs) {
    bt_status_t status;
    int i;
    if (sBtA2dpSinkVendorInterface != NULL) {
        status = sBtA2dpSinkVendorInterface->update_supported_codecs_param_vendor
            (a2dpSnkCodecList, num_codec_configs);
        if (BT_STATUS_SUCCESS != status) {
            ALOGE(LOGTAG " UpdateSupportedCodecs: failed, status = %d", status);
            fprintf(stdout, "UpdateSupportedCodecs: failed, status = %d\n", status);
        }
    }
}

bool A2dp_Sink::isConnectionEvent(BluetoothEventId event_id) {
    bool ret = false;
    if (event_id >= A2DP_SINK_API_CONNECT_REQ && event_id <= A2DP_SINK_DISCONNECTING_CB)
        ret = true;
    ALOGD(LOGTAG " isConnectionEvent: %d", ret);
    return ret;
}
char* A2dp_Sink::dump_message(BluetoothEventId event_id) {
    switch(event_id) {
    case A2DP_SINK_API_CONNECT_REQ:
        return"API_CONNECT_REQ";
    case A2DP_SINK_API_DISCONNECT_REQ:
        return "API_DISCONNECT_REQ";
    case A2DP_SINK_DISCONNECTED_CB:
        return "DISCONNECTED_CB";
    case A2DP_SINK_CONNECTING_CB:
        return "CONNECING_CB";
    case A2DP_SINK_CONNECTED_CB:
        return "CONNECTED_CB";
    case A2DP_SINK_DISCONNECTING_CB:
        return "DISCONNECTING_CB";
    case A2DP_SINK_FOCUS_REQUEST_CB:
        return "FOCUS_REQUEST_CB";
    case A2DP_SINK_AUDIO_SUSPENDED:
        return "AUDIO_SUSPENDED_CB";
    case A2DP_SINK_AUDIO_STOPPED:
        return "AUDIO_STOPPED_CB";
    case A2DP_SINK_AUDIO_STARTED:
        return "AUDIO_STARTED_CB";
    case A2DP_SINK_CODEC_CONFIG:
        return "A2DP_SINK_CODEC_CONFIG";
    }
    return "UNKNOWN";
}

void A2dp_Sink::state_disconnected_handler(BtEvent* pEvent, list<A2dp_Device>::iterator iter) {
    char str[18];
    BtEvent *pOpenInputStream = NULL;
    ALOGD(LOGTAG "state_disconnected_handler Processing event %s", dump_message(pEvent->event_id));
    switch(pEvent->event_id) {
        case A2DP_SINK_API_CONNECT_REQ:
            memcpy(&iter->mConnectingDevice, &iter->mDevice, sizeof(bt_bdaddr_t));
            if (sBtA2dpSinkInterface != NULL) {
                sBtA2dpSinkInterface->connect(&iter->mDevice);
            }
            change_state(iter, DEVICE_STATE_PENDING);
            break;
        case A2DP_SINK_CONNECTING_CB:
            memcpy(&iter->mConnectingDevice, &iter->mDevice, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&iter->mConnectingDevice, str, 18);
            fprintf(stdout, "A2DP Sink Connecting to %s\n", str);
            change_state(iter, DEVICE_STATE_PENDING);
            break;
        case A2DP_SINK_CONNECTED_CB:
            memset(&iter->mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            memcpy(&iter->mConnectedDevice, &iter->mDevice, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&iter->mConnectedDevice, str, 18);
            fprintf(stdout, "A2DP Sink Connected to %s\n", str);
            change_state(iter, DEVICE_STATE_CONNECTED);
            pOpenInputStream = new BtEvent;
            pOpenInputStream->a2dpSinkStreamingEvent.event_id =
                    A2DP_SINK_STREAMING_OPEN_INPUT_STREAM;
            if (pA2dpSinkStream) {
                thread_post(pA2dpSinkStream->threadInfo.thread_id,
                pA2dpSinkStream->threadInfo.thread_handler, (void*)pOpenInputStream);
            }
            break;
        default:
            ALOGD(LOGTAG " event not handled %d ", pEvent->event_id);
            break;
    }
}
void A2dp_Sink::state_pending_handler(BtEvent* pEvent, list<A2dp_Device>::iterator iter) {
    char str[18];
    bool is_valid_codec = true;
    BtEvent *pOpenInputStream = NULL;
    ALOGD(LOGTAG " state_pending_handler Processing event %s", dump_message(pEvent->event_id));
    switch(pEvent->event_id) {
        case A2DP_SINK_CONNECTING_CB:
            ALOGD(LOGTAG " dummy event A2DP_SINK_CONNECTING_CB");
            break;
        case A2DP_SINK_CONNECTED_CB:
            memcpy(&iter->mConnectedDevice, &iter->mDevice, sizeof(bt_bdaddr_t));
            memset(&iter->mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&iter->mConnectedDevice, str, 18);
            fprintf(stdout,  "A2DP Sink Connected to %s\n", str);
            change_state(iter, DEVICE_STATE_CONNECTED);
            pOpenInputStream = new BtEvent;
            pOpenInputStream->a2dpSinkStreamingEvent.event_id =
                    A2DP_SINK_STREAMING_OPEN_INPUT_STREAM;
            if (pA2dpSinkStream) {
                thread_post(pA2dpSinkStream->threadInfo.thread_id,
                pA2dpSinkStream->threadInfo.thread_handler, (void*)pOpenInputStream);
            }
            break;
        case A2DP_SINK_DISCONNECTED_CB:
            fprintf(stdout, "A2DP Sink DisConnected\n");
            memset(&iter->mConnectedDevice, 0, sizeof(bt_bdaddr_t));
            memset(&iter->mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            change_state(iter, DEVICE_STATE_DISCONNECTED);
            break;
        case A2DP_SINK_API_CONNECT_REQ:
            bdaddr_to_string(&iter->mConnectingDevice, str, 18);
            fprintf(stdout, "A2DP Sink Connecting to %s\n", str);
            break;
        case A2DP_SINK_DISCONNECTING_CB:
            ALOGD(LOGTAG " dummy event A2DP_SINK_DISCONNECTING_CB");
            break;
        case A2DP_SINK_CODEC_CONFIG:
            iter->dev_codec_type = pEvent->a2dpSinkEvent.arg1;
            if (pEvent->a2dpSinkEvent.buf_ptr == NULL) {
                break;
            }
            memcpy(&iter->dev_codec_config, pEvent->a2dpSinkEvent.buf_ptr,
                    pEvent->a2dpSinkEvent.buf_size);
            osi_free(pEvent->a2dpSinkEvent.buf_ptr);
            memcpy(&iter->mDevice, &pEvent->a2dpSinkEvent.bd_addr, sizeof(bt_bdaddr_t));
            bdaddr_to_string(&iter->mDevice, str, 18);
            fprintf(stdout, "Codec Configuration for device %s\n", str);
            switch (iter->dev_codec_type) {
               case A2DP_SINK_AUDIO_CODEC_SBC:
                   fprintf(stdout, "Codec type = SBC\n");
                   iter->av_config.sample_rate = pA2dpSinkStream->
                       get_a2dp_sbc_sampling_rate(iter->dev_codec_config.sbc_config.samp_freq);
                   iter->av_config.channel_count = pA2dpSinkStream->
                       get_a2dp_sbc_channel_mode(iter->dev_codec_config.sbc_config.ch_mode);
                   break;
               case A2DP_SINK_AUDIO_CODEC_MP3:
                   fprintf(stdout, "Codec type = MP3\n");
                   iter->av_config.sample_rate = pA2dpSinkStream->
                       get_a2dp_mp3_sampling_rate(iter->dev_codec_config.mp3_config.sampling_freq);
                   iter->av_config.channel_count = pA2dpSinkStream->
                       get_a2dp_mp3_channel_mode(iter->dev_codec_config.mp3_config.channel_count);
                   break;
               case A2DP_SINK_AUDIO_CODEC_AAC:
                   fprintf(stdout, "Codec type = AAC\n");
                   iter->av_config.sample_rate = pA2dpSinkStream->
                       get_a2dp_aac_sampling_rate(iter->dev_codec_config.aac_config.sampling_freq);
                   iter->av_config.channel_count = pA2dpSinkStream->
                       get_a2dp_aac_channel_mode(iter->dev_codec_config.aac_config.channel_count);
                   break;
               case A2DP_SINK_AUDIO_CODEC_APTX:
                   fprintf(stdout, "Codec type = APTX\n");
                   iter->av_config.sample_rate = pA2dpSinkStream->
                       get_a2dp_aptx_sampling_rate(iter->dev_codec_config
                       .aptx_config.sampling_freq);
                   iter->av_config.channel_count = pA2dpSinkStream->
                       get_a2dp_aptx_channel_mode(iter->dev_codec_config
                       .aptx_config.channel_count);
                   break;
               default:
                   is_valid_codec = false;
                   ALOGE(LOGTAG " Invalid codec type %d ", iter->dev_codec_type);
                   break;
            }
            if (is_valid_codec) {
                fprintf(stdout, "Sample Rate = %d\n", iter->av_config.sample_rate);
                fprintf(stdout, "Channel Mode = %d\n", iter->av_config.channel_count);
            }
            break;
        default:
            ALOGD(LOGTAG " event not handled %d ", pEvent->event_id);
            break;
    }
}

void A2dp_Sink::state_connected_handler(BtEvent* pEvent, list<A2dp_Device>::iterator iter) {
    char str[18];
    bool is_valid_codec = true;
    uint32_t pcm_data_read = 0;
    BtEvent *pAMReleaseControl = NULL, *pCloseAudioStream = NULL, *pAMRequestControl = NULL;
    ALOGD(LOGTAG " state_connected_handler Processing event %s", dump_message(pEvent->event_id));
    switch(pEvent->event_id) {
        case A2DP_SINK_API_CONNECT_REQ:
            bdaddr_to_string(&iter->mConnectedDevice, str, 18);
            fprintf(stdout, "A2DP Sink Connected to %s\n", str);
            break;
        case A2DP_SINK_API_DISCONNECT_REQ:
            if (!memcmp(&pA2dpSinkStream->mStreamingDevice, &iter->mDevice, sizeof(bt_bdaddr_t)))
            {
                pCloseAudioStream = new BtEvent;
                pCloseAudioStream->a2dpSinkStreamingEvent.event_id =
                        A2DP_SINK_STREAMING_CLOSE_AUDIO_STREAM;
                if (pA2dpSinkStream) {
                    thread_post(pA2dpSinkStream->threadInfo.thread_id,
                    pA2dpSinkStream->threadInfo.thread_handler, (void*)pCloseAudioStream);
                }
            }
            bdaddr_to_string(&iter->mConnectedDevice, str, 18);
            fprintf(stdout, "A2DP Sink DisConnecting from %s\n", str);
            memset(&iter->mConnectedDevice, 0, sizeof(bt_bdaddr_t));
            memset(&iter->mConnectingDevice, 0, sizeof(bt_bdaddr_t));

            if (sBtA2dpSinkInterface != NULL) {
                sBtA2dpSinkInterface->disconnect(&iter->mDevice);
            }
            change_state(iter, DEVICE_STATE_PENDING);
            break;
        case A2DP_SINK_DISCONNECTED_CB:
            if (!memcmp(&pA2dpSinkStream->mStreamingDevice, &iter->mDevice, sizeof(bt_bdaddr_t)))
            {
                pCloseAudioStream = new BtEvent;
                pCloseAudioStream->a2dpSinkStreamingEvent.event_id =
                        A2DP_SINK_STREAMING_CLOSE_AUDIO_STREAM;
                if (pA2dpSinkStream) {
                    thread_post(pA2dpSinkStream->threadInfo.thread_id,
                    pA2dpSinkStream->threadInfo.thread_handler, (void*)pCloseAudioStream);
                }
            }
            memset(&iter->mConnectedDevice, 0, sizeof(bt_bdaddr_t));
            memset(&iter->mConnectingDevice, 0, sizeof(bt_bdaddr_t));
            fprintf(stdout, "A2DP Sink DisConnected \n");
            change_state(iter, DEVICE_STATE_DISCONNECTED);
            break;
        case A2DP_SINK_DISCONNECTING_CB:
            if (!memcmp(&pA2dpSinkStream->mStreamingDevice, &iter->mDevice, sizeof(bt_bdaddr_t)))
            {
                pCloseAudioStream = new BtEvent;
                pCloseAudioStream->a2dpSinkStreamingEvent.event_id =
                        A2DP_SINK_STREAMING_CLOSE_AUDIO_STREAM;
                if (pA2dpSinkStream) {
                    thread_post(pA2dpSinkStream->threadInfo.thread_id,
                    pA2dpSinkStream->threadInfo.thread_handler, (void*)pCloseAudioStream);
                }
            }
            fprintf(stdout, "A2DP Sink DisConnecting\n");
            change_state(iter, DEVICE_STATE_PENDING);
            break;
        case A2DP_SINK_CODEC_CONFIG:
             iter->dev_codec_type = pEvent->a2dpSinkEvent.arg1;
             if (pEvent->a2dpSinkEvent.buf_ptr == NULL) {
                 break;
             }
             memcpy(&iter->dev_codec_config, pEvent->a2dpSinkEvent.buf_ptr,
                     pEvent->a2dpSinkEvent.buf_size);
             osi_free(pEvent->a2dpSinkEvent.buf_ptr);
             memcpy(&iter->mDevice, &pEvent->a2dpSinkEvent.bd_addr, sizeof(bt_bdaddr_t));
             bdaddr_to_string(&iter->mDevice, str, 18);
             fprintf(stdout, "Codec Configuration for device %s\n", str);
             switch (iter->dev_codec_type) {
                case A2DP_SINK_AUDIO_CODEC_SBC:
                    fprintf(stdout, "Codec type = SBC\n");
                    iter->av_config.sample_rate = pA2dpSinkStream->
                        get_a2dp_sbc_sampling_rate(iter->dev_codec_config.sbc_config.samp_freq);
                    iter->av_config.channel_count = pA2dpSinkStream->
                        get_a2dp_sbc_channel_mode(iter->dev_codec_config.sbc_config.ch_mode);
                    break;
                case A2DP_SINK_AUDIO_CODEC_MP3:
                    fprintf(stdout, "Codec type = MP3\n");
                    iter->av_config.sample_rate = pA2dpSinkStream->
                        get_a2dp_mp3_sampling_rate(iter->dev_codec_config.mp3_config.sampling_freq);
                    iter->av_config.channel_count = pA2dpSinkStream->
                        get_a2dp_mp3_channel_mode(iter->dev_codec_config.mp3_config.channel_count);
                    break;
                case A2DP_SINK_AUDIO_CODEC_AAC:
                    fprintf(stdout, "Codec type = AAC\n");
                    iter->av_config.sample_rate = pA2dpSinkStream->
                        get_a2dp_aac_sampling_rate(iter->dev_codec_config.aac_config.sampling_freq);
                    iter->av_config.channel_count = pA2dpSinkStream->
                        get_a2dp_aac_channel_mode(iter->dev_codec_config.aac_config.channel_count);
                    break;
                case A2DP_SINK_AUDIO_CODEC_APTX:
                    fprintf(stdout, "Codec type = APTX\n");
                    iter->av_config.sample_rate = pA2dpSinkStream->
                        get_a2dp_aptx_sampling_rate(iter->dev_codec_config
                        .aptx_config.sampling_freq);
                    iter->av_config.channel_count = pA2dpSinkStream->
                        get_a2dp_aptx_channel_mode(iter->dev_codec_config
                        .aptx_config.channel_count);
                    break;
                default:
                    is_valid_codec = false;
                    ALOGE(LOGTAG " Invalid codec type %d ", iter->dev_codec_type);
                    break;
             }
             if (is_valid_codec) {
                 fprintf(stdout, "Sample Rate = %d\n", iter->av_config.sample_rate);
                 fprintf(stdout, "Channel Mode = %d\n", iter->av_config.channel_count);
             }
             break;
        case A2DP_SINK_AUDIO_STARTED:
        case A2DP_SINK_FOCUS_REQUEST_CB:
            bdaddr_to_string(&pA2dpSinkStream->mStreamingDevice, str, 18);
            ALOGD(LOGTAG " current streaming device %s", str);

            if (memcmp(&pA2dpSinkStream->mStreamingDevice, &bd_addr_null, sizeof(bt_bdaddr_t)) &&
                    memcmp(&pA2dpSinkStream->mStreamingDevice, &iter->mDevice, sizeof(bt_bdaddr_t)))
            {
                ALOGD(LOGTAG " another dev started streaming, pause previous one");
                if (pAvrcp != NULL)
                    pAvrcp->SendPassThruCommandNative(CMD_ID_PAUSE,
                            &pA2dpSinkStream->mStreamingDevice, 1);
                if (pA2dpSinkStream && !pA2dpSinkStream->use_bt_a2dp_hal)
                {
                    memset(&pA2dpSinkStream->mStreamingDevice, 0, sizeof(bt_bdaddr_t));
                    pAMReleaseControl = new BtEvent;
                    pAMReleaseControl->a2dpSinkStreamingEvent.event_id =
                            A2DP_SINK_STREAMING_AM_RELEASE_CONTROL;
                    if (pA2dpSinkStream) {
                        thread_post(pA2dpSinkStream->threadInfo.thread_id,
                        pA2dpSinkStream->threadInfo.thread_handler, (void*)pAMReleaseControl);
                    }
                }
                else
                {
                    pCloseAudioStream = new BtEvent;
                    pCloseAudioStream->a2dpSinkStreamingEvent.event_id =
                            A2DP_SINK_STREAMING_CLOSE_AUDIO_STREAM;
                    if (pA2dpSinkStream) {
                        thread_post(pA2dpSinkStream->threadInfo.thread_id,
                        pA2dpSinkStream->threadInfo.thread_handler, (void*)pCloseAudioStream);
                    }
                }
            }
            ALOGE(LOGTAG " updating avconfig parameters for this device");
            if (iter->dev_codec_type == A2DP_SINK_AUDIO_CODEC_SBC) {
                pA2dpSinkStream->sample_rate = iter->av_config.sample_rate;
                pA2dpSinkStream->channel_count = iter->av_config.channel_count;
            }
            pA2dpSinkStream->codec_type = iter->dev_codec_type;
            memcpy(&pA2dpSinkStream->codec_config, &iter->dev_codec_config,
                    sizeof(btav_codec_config_t));

            memcpy(&pA2dpSinkStream->mStreamingDevice, &pEvent->a2dpSinkEvent.bd_addr,
                    sizeof(bt_bdaddr_t));
            bdaddr_to_string(&pA2dpSinkStream->mStreamingDevice, str, 18);
            ALOGD(LOGTAG " A2DP_SINK_AUDIO_STARTED - set current streaming device as %s", str);

            sBtA2dpSinkVendorInterface->
                    update_streaming_device_vendor(&pA2dpSinkStream->mStreamingDevice);

            pAMRequestControl = new BtEvent;
            pAMRequestControl->a2dpSinkStreamingEvent.event_id =
                    A2DP_SINK_STREAMING_AM_REQUEST_CONTROL;
            if (pA2dpSinkStream) {
                thread_post(pA2dpSinkStream->threadInfo.thread_id,
                pA2dpSinkStream->threadInfo.thread_handler, (void*)pAMRequestControl);
            }
            break;
        case A2DP_SINK_AUDIO_SUSPENDED:
        case A2DP_SINK_AUDIO_STOPPED:
            if(memcmp(&pA2dpSinkStream->mStreamingDevice, &pEvent->a2dpSinkEvent.bd_addr,
                    sizeof(bt_bdaddr_t)))
            {
                ALOGD(LOGTAG " A2DP_SINK_AUDIO_SUSPENDED/STOPPED for non streaming device, ignore");
                break;
            }
            memset(&pA2dpSinkStream->mStreamingDevice, 0, sizeof(bt_bdaddr_t));

            pAMReleaseControl = new BtEvent;
            pAMReleaseControl->a2dpSinkStreamingEvent.event_id =
                    A2DP_SINK_STREAMING_AM_RELEASE_CONTROL;
            if (pA2dpSinkStream) {
                thread_post(pA2dpSinkStream->threadInfo.thread_id,
                pA2dpSinkStream->threadInfo.thread_handler, (void*)pAMReleaseControl);
            }
            break;
        default:
            ALOGD(LOGTAG " event not handled %d ", pEvent->event_id);
            break;
    }
}

void A2dp_Sink::change_state(list<A2dp_Device>::iterator iter, A2dpSinkDeviceState mState) {
   BtEvent *pA2dpSinkDisconnected = NULL;
   ALOGD(LOGTAG " current State = %d, new state = %d", iter->mSinkDeviceState, mState);
   pthread_mutex_lock(&lock);
   iter->mSinkDeviceState = mState;
   if (iter->mSinkDeviceState == DEVICE_STATE_DISCONNECTED)
   {
       if (!memcmp(&pA2dpSinkStream->mStreamingDevice, &iter->mDevice, sizeof(bt_bdaddr_t)))
       {
           memset(&pA2dpSinkStream->mStreamingDevice, 0, sizeof(bt_bdaddr_t));

           pA2dpSinkDisconnected = new BtEvent;
           pA2dpSinkDisconnected->a2dpSinkStreamingEvent.event_id =
                   A2DP_SINK_STREAMING_DISCONNECTED;
           if (pA2dpSinkStream) {
               thread_post(pA2dpSinkStream->threadInfo.thread_id,
               pA2dpSinkStream->threadInfo.thread_handler, (void*)pA2dpSinkDisconnected);
           }
       }
       pA2dpDeviceList.erase(iter);
       ALOGD(LOGTAG " iter %x deleted from list", iter);
   }
   ALOGD(LOGTAG " iter %x state changes to %d ", iter, mState);
   pthread_mutex_unlock(&lock);
}

A2dp_Sink :: A2dp_Sink(const bt_interface_t *bt_interface, config_t *config) {

    this->bluetooth_interface = bt_interface;
    this->config = config;
    sBtA2dpSinkInterface = NULL;
    mSinkState = SINK_STATE_NOT_STARTED;
    pthread_mutex_init(&this->lock, NULL);
    pA2dpSinkStream = new A2dp_Sink_Streaming(config);
    pA2dpSinkStream->threadInfo.thread_id = thread_new (pA2dpSinkStream->threadInfo.thread_name);
    max_a2dp_conn = 0;
}

A2dp_Sink :: ~A2dp_Sink() {
    pthread_mutex_destroy(&lock);
}

A2dp_Device :: A2dp_Device(config_t *config, bt_bdaddr_t dev) {
    this->config = config;
    memset(&mDevice, 0, sizeof(bt_bdaddr_t));
    memcpy(&mDevice, &dev, sizeof(bt_bdaddr_t));
    memset(&mConnectedDevice, 0, sizeof(bt_bdaddr_t));
    memset(&mConnectingDevice, 0, sizeof(bt_bdaddr_t));
    mSinkDeviceState = DEVICE_STATE_DISCONNECTED;
    memset(&av_config, 0, sizeof(A2dpSinkConfig_t));
    pthread_mutex_init(&this->lock, NULL);
    mAvrcpConnected = false;
    mNotificationLabel = -1;
    mAbsVolNotificationRequested = false;
}

A2dp_Device :: ~A2dp_Device() {
    pthread_mutex_destroy(&lock);
}


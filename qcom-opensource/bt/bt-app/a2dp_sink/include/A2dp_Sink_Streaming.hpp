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

#ifndef A2DP_SINK_STREAMING_APP_H
#define A2DP_SINK_STREAMING_APP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <map>
#include <string>
#include <hardware/bluetooth.h>
#include <hardware/bt_av.h>
#include <pthread.h>
#if (defined BT_AUDIO_HAL_INTEGRATION)
#include "qahw_api.h"
#include "qahw_defs.h"

#endif

#include "osi/include/log.h"
#include "osi/include/thread.h"
#include "osi/include/config.h"
#include "osi/include/allocator.h"
#include "osi/include/alarm.h"
#include "ipc.h"
#include "utils.h"
#include "hardware/bt_av_vendor.h"


typedef void (*A2dpSinkStreamingThreadHandler) (void *msg);

typedef struct {
    thread_t *thread_id;
    A2dpSinkStreamingThreadHandler thread_handler;
    char *thread_name;
} A2dpSinkStreamingThreadInfo;

#define A2DP_SINK_PCM_FETCH_TIMER_DURATION         35
#define A2DP_SINK_COMPRESS_FEED_TIMER_DURATION     40
#define A2DP_SINK_GBUF_MAX_SIZE 65535

class A2dp_Sink_Streaming {

  private:
    config_t *config;

  public:
    A2dp_Sink_Streaming(config_t *config);
    ~A2dp_Sink_Streaming();
    char* dump_message(BluetoothEventId event_id);
    void HandleEnableSinkStreaming();
    void HandleDisableSinkStreaming();
    pthread_mutex_t lock;
    bt_bdaddr_t mStreamingDevice;
    bt_bdaddr_t mResumingDevice;
    const btav_sink_vendor_interface_t *mBtA2dpSinkStreamingVendorInterface;
    ControlStatusType controlStatus;
    A2dpSinkStreamingThreadInfo threadInfo;
    uint32_t sample_rate;
    uint8_t channel_count;
    alarm_t *pcm_data_fetch_timer;
    alarm_t *compress_audio_feed_timer;
#if (defined BT_AUDIO_HAL_INTEGRATION)
    // structure for output stream
    qahw_stream_handle_t *out_stream;
    // structures used for loading A2DP HAL
    qahw_module_handle_t *a2dp_input_device;
    qahw_stream_handle_t *input_stream;
#endif
#if (defined USE_GST)
    uint8_t *gbuff;
#endif
    //apis for in_stream bt a2dp HAL, to read data.
    void LoadBtA2dpHAL();
    void UnLoadBtA2dpHAL();
    void OpenInputStream();
    void SuspendInputStream();
    void CloseInputStream();
    uint32_t ReadInputStream(uint8_t* data, uint32_t size);
    uint32_t GetInputStreamBufferSize();
    bool use_bt_a2dp_hal;
    bool sbc_decoding;
    bool fetch_rtp_info;
    bool enable_delay_report;
    bool relay_sink_data; /* if this is enabled, we relay Sink data to Src */
    bool enable_notification_cb; /* if enabled, notification is received from stack
                                    on incoming media data instrad of polling from
                                    BT-APP*/
    // apis for out_stream Audio HAL, to write data.
    void ConfigureAudioHal();
    void CloseAudioStream();
    size_t pcm_buf_size;
    size_t cuml_data_written_to_audio;
    size_t residual_compress_data;
    uint8_t* pcm_buf;
    bool pcm_timer;
    bool compress_offload_timer;
    void StartPcmTimer();
    void StopDataFetchTimer();
    void OnDisconnected();
    void GetLibInterface(const btav_sink_vendor_interface_t *sBtA2dpSinkStrVendorInterface);
    uint16_t codec_type;
    btav_codec_config_t codec_config;
    uint32_t get_a2dp_sbc_sampling_rate(uint8_t frequency);
    uint8_t get_a2dp_sbc_channel_mode(uint8_t channel_count);
    uint32_t get_a2dp_aac_sampling_rate(uint16_t frequency);
    uint8_t get_a2dp_aac_channel_mode(uint8_t channel_count);
    uint32_t get_a2dp_mp3_sampling_rate(uint16_t frequency);
    uint8_t get_a2dp_mp3_channel_mode(uint8_t channel_count);
    uint32_t get_a2dp_aptx_sampling_rate(uint8_t frequency);
    uint8_t get_a2dp_aptx_channel_mode(uint8_t channel_count);
    void FillCompressBuffertoAudioOutHal();
    void StartCompressAudioFeedTimer();
    void StopCompressAudioFeedTimer();
    void SetStreamVol(int curr_audio_index);
    uint64_t get_cur_time();
};

#endif

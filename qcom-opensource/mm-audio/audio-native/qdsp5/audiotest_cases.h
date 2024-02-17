/* audiotest_cases.h - native audio test application header
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2009-2011, 2012 The Linux Foundation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef AUDIOTEST_CASE_H
#define AUDIOTEST_CASE_H

/* Mp3 Test Module Interface Definition */
int mp3play_read_params(void);
int mp3_play_control_handler(void* private_data);
void mp3play_help_menu(void);

/* AMRNB Test Module Interface Definition */
int amrnbplay_read_params(void);
int amrnb_play_control_handler(void* private_data);
void amrnbplay_help_menu(void);
int amrnbrec_read_params(void);
int amrnb_rec_control_handler(void* private_data);
void amrnbrec_help_menu(void);

/* AMRWB Test Module Interface Definition */
int amrwbplay_read_params(void);
int amrwb_play_control_handler(void *private_data);
void amrwbplay_help_menu(void);

/* PCM Test Module Interface Definition */
int pcmplay_read_params(void);
int pcm_play_control_handler(void* private_data);
void pcmplay_help_menu(void);
int pcmrec_read_params(void);
int pcm_rec_control_handler(void* private_data);
void pcmrec_help_menu(void);

#ifdef AUDIOV2
/* ADPCM Test Module Interface Definition */
int adpcmplay_read_params(void);
int adpcm_play_control_handler(void *private_data);
void adpcmplay_help_menu(void);

#endif

/* WMA Test Module Interface Definition */
int wmaplay_read_params(void);
int wma_play_control_handler(void *private_data);
void wmaplay_help_menu(void);

/* WMAPRO Test Module Interface Definition */
int wmaproplay_read_params(void);
int wmapro_play_control_handler(void *private_data);
void wmaproplay_help_menu(void);

/* QCP Test Module Interface Definition */
int qcpplay_read_params(void);
int qcp_play_control_handler(void *private_data);
void qcpplay_help_menu(void);


/* Profile Module Interface Definition */
int profile_read_params(void);
int profile_control_handler(void *private_data);
void profile_help_menu(void);

/* AAC Test Module Interface Definition */
int aacplay_read_params(void);
int aac_play_control_handler(void* private_data);
void aacplay_help_menu(void);
int aacrec_read_params(void);
int aac_rec_control_handler(void* private_data);
void aacrec_help_menu(void);

/* Voice Memo Test Module Interface Definition */
int voicememo_read_params(void);
int voicememo_control_handler(void *private_data);
void voicememo_help_menu(void);

/* SND Test Module Interface Definition */
int sndsetdev_read_params(void);
void sndsetdev_help_menu(void);

#if defined(AUDIOV2) || defined(AUDIO7X27A)
/* Voice Enc Test Module Interface Definition */
int voiceenc_read_params(void);
int voiceenc_control_handler(void *private_data);
void voiceenc_help_menu(void);
#endif

#ifdef AUDIOV2
/* Mp3 Test Module Interface Definition */
int lpaplay_read_params(void);
int lpa_play_control_handler(void* private_data);
void lpaplay_help_menu(void);

#if defined(TARGET_USES_QCOM_MM_AUDIO)
/* Dev Control Test Module definition */
int devctl_read_params(void);
void devctl_help_menu(void);

#ifdef QDSP6V2
int hdmi_ac3_read_params(void);
void hdmi_ac3_help_menu(void);
int hdmi_dts_read_params(void);
void hdmi_dts_help_menu(void);
#endif

#endif

/* SBC Test Module Interface Definition */
int sbcrec_read_params(void);
int sbc_rec_control_handler(void* private_data);
void sbcrec_help_menu(void);

#endif /* AUDIOV2 */

#if defined(AUDIOV2) || defined(AUDIO7X27A)
/* FM Playback Test Module definition */
int fm_play_read_params(void);
int fm_play_control_handler(void *private_data);
void fm_play_help_menu(void);

#endif /* AUDIOV2 OR AUDIO7X27A*/

#ifdef QDSP6V2
/* MVS Test Module Interface Definition */
int mvstest_read_params(void);
void mvstest_help_menu(void);
int mvs_lp_test_control_handler(void* private_data);
#endif
#endif /* AUDIOTEST_CASE_H */

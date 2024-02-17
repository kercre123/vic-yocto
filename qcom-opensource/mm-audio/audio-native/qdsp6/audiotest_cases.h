/* audiotest_cases.h - native audio test application header
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2010, The Linux Foundation. All rights reserved.
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

/* PCM Test Module Interface Definition */
int pcmplay_read_params(void);
int pcm_play_control_handler(void* private_data);
void pcmplay_help_menu(void);
int pcmrec_read_params(void);
int pcm_rec_control_handler(void* private_data);
void pcmrec_help_menu(void);

/* QCP Test Module Interface Definition */
int qcprec_read_params(void);
int qcp_rec_control_handler(void *private_data);
void qcprec_help_menu(void);


/* AAC Test Module Interface Definition */
int aacplay_read_params(void);
int aac_play_control_handler(void* private_data);
void aacplay_help_menu(void);
int aacrec_read_params(void);
int aac_rec_control_handler(void* private_data);
void aacrec_help_menu(void);

int switchdev_read_params(void);
void switchdev_help_menu(void);
int mastervol_read_params(void);
void mastervol_help_menu(void);
int mastermute_read_params(void);
void mastermute_help_menu(void);

/* ATUTEST Module Interface Definition */
int atutest_read_params(void);
int atutest_control_handler(void* private_data);
void atutest_help_menu(void);

#endif /* AUDIOTEST_CASE_H */

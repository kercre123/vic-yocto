/* atutest.c - native ATU test application
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2010, 2012 The Linux Foundation. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include<unistd.h>
#include<string.h>
#include <errno.h>
#include <linux/msm_audio.h>
#include "audiotest_def.h"
#if defined(TARGET_USES_QCOM_MM_AUDIO)
#include "msm8k_atu.h"
#else
enum atu_path{
	ATU_TONE_PATH_LOCAL,
	ATU_TONE_PATH_TX,
	ATU_TONE_PATH_BOTH,
	ATU_TONE_PATH_32BIT_DUMMY = 0xFFFFFFFF
};
enum atu_status{
	ATU_REPEAT,
	ATU_STOP_DONE
};
#define atu_set_device(device) (-EPERM)
#define atu_set_rx_volume(rx_vol) (-EPERM)
#define atu_set_tx_volume(rx_vol) (-EPERM)
#define atu_start_sound_id(sound_id, repeat_cnt, tone_path, cb_ptr, client_data) (-EPERM)
#define atu_start_dtmf(f_hi_hz, f_low_hz, tone_duration_ms, tone_path, cb_ptr, client_data) (-EPERM)
#define atu_stop() (-EPERM)
#define atu_init() (-EPERM)
#define atu_dinit() (-EPERM)
#endif

int tone, repeat, device;

void atu_callback_function(enum atu_status status, const void *client_data)
{
	printf("ATU status %d \n", status);
}

void play_tone(int tone_id, unsigned int repeatCnt, unsigned int deviceId)
{
	int sleep_time_sec = 10;
	unsigned int repeat_cnt = 0;
	unsigned int id         = 0;
	unsigned int device_id = 2;
	enum atu_path tone_path = ATU_TONE_PATH_BOTH;

	printf("toneID=%d, deviceId=%d, repeat=%d\n",tone_id, deviceId, repeatCnt);

	atu_init();

	if(deviceId != 2)
        device_id = deviceId;
	atu_set_device(device_id);
	atu_set_rx_volume(2000);
	atu_set_tx_volume(2000);

	id = tone_id;

	if(repeatCnt > 0)
        repeat_cnt = repeatCnt;


	if (atu_start_sound_id(id, repeat_cnt, tone_path, atu_callback_function, NULL) < 0) {
		printf("Test case not supported\n");
		return;
	}
	sleep(sleep_time_sec);
	atu_stop();
	sleep(1);
	atu_dinit();
	return;
}

void* atutest_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	play_tone(tone, repeat, device);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int atutest_read_params(void) {
	struct audiotest_thread_context *context;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		struct audio_pvt_data *audio_data;
		audio_data = (struct audio_pvt_data *) malloc(sizeof(struct audio_pvt_data));
		if(!audio_data) {
			printf("error allocating audio instance structure \n");
			free_context(context);
			ret_val = -1;
		} else {
			token = strtok(NULL, " ");
			while (token != NULL) {
				if (!memcmp(token,"-id=", (sizeof("-id=")-1))) {
					context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
				} else if (!memcmp(token,"-tone=", (sizeof("-tone=") - 1))) {
					tone = atoi(&token[sizeof("-tone=") - 1]);
				} else if (!memcmp(token,"-repeat=", (sizeof("-repeat=") - 1))) {
					repeat = atoi(&token[sizeof("-repeat=") - 1]);
				} else if (!memcmp(token,"-device=", (sizeof("-device=") - 1))) {
					device = atoi(&token[sizeof("-device=") - 1]);
				} else {
					context->config.file_name = token;
				}
				token = strtok(NULL, " ");
			}
			context->type = AUDIOTEST_TEST_MOD_ATUTEST;
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL,
					atutest_thread, (void*) context);
		}
	}

	return ret_val;

}

int atutest_control_handler(void *private_data)
{
	int ret_val = 0;
	/* Nothing to do */
	return ret_val;
}

const char *atutest_help_txt =
"Play tone test: toneid, deviceid, repeatcnt \n\
echo \"playtone -id=xxx -tone=x -device=x -repeat=x\" > /data/audio_test \n\
tone = tone_id, device = device_id, repeat = repeat count\n\ ";

void atutest_help_menu(void) {
	printf("%s\n", atutest_help_txt);
}

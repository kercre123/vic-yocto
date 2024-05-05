/* fmtest.c - native FM test application
 *
 * Based on native fm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2010-2012 The Linux Foundation. All rights reserved.
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
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/msm_audio.h>
#include <pthread.h>
#include <errno.h>
#include "audiotest_def.h"

#ifdef AUDIO7X27A
#define SND_DEVICE_FM_HEADSET 0x23
#define SND_DEVICE_HEADSET 0x03
#define MAX_STRING 10
char DeviceSettingString[MAX_STRING];
#endif

const char     *dev_file_name;
int playback_stop = 1;
int afd;

int fm_play(struct audtest_config *cfg)
{
	static int fm_devid_rx;
	int ret = 0;
#ifdef AUDIOV2
	unsigned short dec_id;
#endif

	afd = open(dev_file_name, O_WRONLY);

	if (afd < 0) {
		perror("fm_play: cannot open audio device");
		return -1;
	}

	cfg->private_data = (void *)afd;
#ifdef AUDIOV2
	if (ioctl(afd, AUDIO_GET_SESSION_ID, &dec_id)) {
		perror("could not get decoder session id\n");
		close(afd);
		return -1;
	}
#if defined(TARGET_USES_QCOM_MM_AUDIO)
	if (devmgr_register_session(dec_id, DIR_RX) < 0) {
		ret = -1;
		goto exit;
	}

	fm_devid_rx = msm_get_device("fmradio_stereo_tx");

	if (msm_en_device(fm_devid_rx, 1) < 0) {
		perror("could not enable device\n");
		close(afd);
		return -1;
	}

	if (msm_route_stream(DIR_RX, dec_id, fm_devid_rx, 1) < 0) {
		perror("could not route stream to Device\n");
		msm_en_device(fm_devid_rx, 0);
		close(afd);
		return -1;
	}
#endif
#endif

	printf("start playback\n");
#ifdef AUDIO7X27A
		snprintf(DeviceSettingString,MAX_STRING,"%d %d %d"
			,SND_DEVICE_FM_HEADSET,SND_MUTE_UNMUTED,
			SND_MUTE_UNMUTED);
		write(afd, &DeviceSettingString,strlen(DeviceSettingString));
		printf("FM device: %s set\n",DeviceSettingString);
#else
	if (ioctl(afd, AUDIO_START, 0) >= 0) {
#endif
		playback_stop = 0;
		while (playback_stop != 1)
			sleep(1);
		printf("end of fm play\n");
#ifndef AUDIO7X27A
	} else {
		printf("fm_play: Unable to start driver\n");
	}
#endif
#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
	if (msm_en_device(fm_devid_rx, 0) < 0) {
		perror("could not disable device\n");
		close(afd);
		return -1;
	}
	if (msm_route_stream(DIR_RX, dec_id, fm_devid_rx, 0) < 0) {
		perror("could not route stream to Device\n");
		close(afd);
		return -1;
	}
	if (devmgr_unregister_session(dec_id, DIR_RX) < 0)
			ret = -1;
exit:
#endif
#ifdef AUDIO7X27A
		snprintf(DeviceSettingString,MAX_STRING,"%d %d %d"
			,SND_DEVICE_HEADSET,SND_MUTE_UNMUTED,SND_MUTE_UNMUTED);
		write(afd, &DeviceSettingString, strlen(DeviceSettingString));
		printf("default device: %s set\n",DeviceSettingString);
#endif
	close(afd);
	return ret;
}

void *fm_play_thread(void *arg)
{
	struct audiotest_thread_context *context =
	(struct audiotest_thread_context *) arg;
	int ret_val;

	ret_val = fm_play(&context->config);
	free_context(context);
	pthread_exit((void *) ret_val);

	return NULL;
}

int fm_play_read_params(void)
{
	struct audiotest_thread_context *context;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
#ifdef AUDIO7X27A
		dev_file_name = "/sys/class/misc/msm_snd/device";
#else
		dev_file_name = "/dev/msm_fm";
#endif

		token = strtok(NULL, " ");

		while (token != NULL) {
			if (!memcmp(token, "-id=", (sizeof("-id=") - 1))) {
				context->cxt_id =
					atoi(&token[sizeof("-id=") - 1]);
				printf("context->cxt_id = %d\n",context->cxt_id);
			}
			token = strtok(NULL, " ");
		}
		context->type = AUDIOTEST_TEST_MOD_FM_DEC;
		pthread_create(&context->thread, NULL,
					fm_play_thread, (void *) context);
	}

	return ret_val;

}

int fm_play_control_handler(void *private_data)
{
	int  drvfd , ret_val = 0;
	char *token;
#if (defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)) || defined(AUDIO7X27A)
	int volume;
#endif

	token = strtok(NULL, " ");
	if (token != NULL) {
		drvfd = (int) private_data;
		if (!memcmp(token, "-cmd=", (sizeof("-cmd=") - 1))) {
#if (defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)) || defined(AUDIO7X27A)
			token = &token[sizeof("-cmd=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
			if (!strcmp(token, "stop")) {
				playback_stop = 1;
				printf("playback_stop = %d\n",playback_stop);
			}
			else if (!strcmp(token, "volume")) {
				int rc;
				unsigned short dec_id;
				token = strtok(NULL, " ");
				if (!memcmp(token, "-value=",
					(sizeof("-value=") - 1))) {
					volume = atoi(&token[sizeof("-value=") - 1]);
#ifdef AUDIO7X27A
					int fd;
					snprintf(DeviceSettingString,MAX_STRING,"%d %d %d"
						,SND_DEVICE_FM_HEADSET,
						SND_METHOD_VOICE, volume);
					fd = open("/sys/class/misc/msm_snd/volume", O_WRONLY);

					if (fd < 0) {
						perror("failed to open device for setting volume");
						return -1;
					}
					printf("Set FM volume  to %d\n",volume);
					write(fd,&DeviceSettingString,
						strlen(DeviceSettingString));
					close(fd);
#else
					if (ioctl(drvfd, AUDIO_GET_SESSION_ID, &dec_id)) {
						perror("could not get decoder session id\n");
					} else {
						printf("session %d - volume %d \n", dec_id, volume);
						rc = msm_set_volume(dec_id, volume);
						printf("session volume result %d\n", rc);
					}
#endif
				}
			}
#else
			token = &token[sizeof("-id=") - 1];
#endif
		}
	} else {
		ret_val = -1;
	}

	return ret_val;
}

const char *fm_play_help_txt =
	"Play FM file: type \n\
echo \"playfm -id=xxx\" > /data/audio_test \n\
Supported control command: stop, volume\n ";

void fm_play_help_menu(void)
{
	printf("%s\n", fm_play_help_txt);
}


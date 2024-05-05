/* snddevtest.c - native snd device test application
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2009-2010, The Linux Foundation. All rights reserved.
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
#include "audiotest_def.h"

#define SND_DEVICE_NODE "/dev/msm_snd"

struct snd_dev_config {
	int	devIndex;
	char*	method;
	int	agc_value;
	int	avc_value;
	struct msm_snd_device_config dev_cfg;
	struct msm_snd_volume_config vol_cfg;
};

static struct snd_dev_config sndConfig;

static int get_method(const char *method) {

#define  SND_METHOD_VOICE  0
#define  SND_METHOD_KEY_BEEP 1
#define  SND_METHOD_MESSAGE 2
#define  SND_METHOD_RING 3
#define  SND_METHOD_MIDI 4
#define  SND_METHOD_AUX 5

#define IS_METHOD(name, id) if (!strcmp(method, name)) return SND_METHOD_##id;

    IS_METHOD("voice", VOICE);
    IS_METHOD("key_beep", KEY_BEEP);
    IS_METHOD("message", MESSAGE);
    IS_METHOD("ring", RING);
    IS_METHOD("midi", MIDI);
    IS_METHOD("aux", AUX);
    printf("unknown method \n");
    return 0;
}

static int get_device(int device) {

#define SND_DEVICE_DEFAULT 0

        if (device < 0) {
            printf("using default SURF device (%d)\n", SND_DEVICE_DEFAULT);
            return SND_DEVICE_DEFAULT;
        }
        printf("using device %d\n", device);
        return device;
}

static int snd_set_device_test(struct snd_dev_config *clnt_config )
{
	int devIndex = 0;
	int methIndex = 0;
        int afd;

	/* Below statement to remove warning for unused variable clnt_config,
	   but to keep function prototype intact */
	(void)clnt_config;
        afd = open(SND_DEVICE_NODE, O_RDWR);
        if (afd < 0) {
                printf("Unable to open audio device = %s\n", SND_DEVICE_NODE);
                goto device_err;
        }
        devIndex = get_device(sndConfig.devIndex);
        methIndex = get_method(sndConfig.method);
        sndConfig.dev_cfg.device = devIndex;
        sndConfig.vol_cfg.device = devIndex;
        sndConfig.vol_cfg.method = methIndex;

	printf(" device = %d, mic_mute = %d, ear_mute = %d \n", 
		sndConfig.dev_cfg.device, sndConfig.dev_cfg.mic_mute, sndConfig.dev_cfg.ear_mute);
	printf(" device = %d, method = %d, volume = %d \n", 
		sndConfig.vol_cfg.device, sndConfig.vol_cfg.method, sndConfig.vol_cfg.volume);

	/* Set device */
        if(ioctl(afd, SND_SET_DEVICE, &sndConfig.dev_cfg)) {
                printf(" Error setting SND_SET_DEVICE \n");
                goto err;
        }

	/* Set Volume */
        if(ioctl(afd, SND_SET_VOLUME, &sndConfig.vol_cfg)) {
                printf(" Error setting SND_SET_VOLUME \n");
                goto err;
        }

        /* Enable/Disable AVC */
	if(ioctl(afd, SND_AVC_CTL, &sndConfig.avc_value)) {
                printf(" Error setting SND_AVC_CTL \n");
                goto err;
        }

        /* Enable/Disable AGC */
	if(ioctl(afd, SND_AGC_CTL, &sndConfig.agc_value)) {
                printf(" Error setting SND_AGC_CTL \n");
                goto err;
        }
	
        close(afd);
        return 0;
err:
        close(afd);
device_err:
	return -1;	
}

void* sndtest_thread(void* arg) {
	struct audiotest_thread_context *context = 
	(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = snd_set_device_test((struct snd_dev_config *)context->config.private_data);
	free_context(context);
	pthread_exit((void*) ret_val);
	return NULL;
}

int sndsetdev_read_params(void) {
	struct audiotest_thread_context *context; 
	char *token;
	int ret_val = 0;
	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		sndConfig.method = "voice";
                sndConfig.vol_cfg.volume =3;
                sndConfig.devIndex = 0;
	
		token = strtok(NULL, " ");
		while (token != NULL) {
			if (!memcmp(token,"-dev=", (sizeof("-dev=" - 1)))) {
				sndConfig.devIndex = atoi(&token[sizeof("-dev=") - 1]);
			} else if (!memcmp(token,"-meth=", (sizeof("-meth=" - 1)))) {
				sndConfig.method = &token[sizeof("-meth=") - 1];
			} else if (!memcmp(token,"-vol=", (sizeof("-vol=" - 1)))) {
				sndConfig.vol_cfg.volume = atoi(&token[sizeof("-vol=") - 1]);
			} else if (!memcmp(token,"-mic=", (sizeof("-mic=" - 1)))) {
				sndConfig.dev_cfg.mic_mute = atoi(&token[sizeof("-mic=") - 1]);
			} else if (!memcmp(token,"-speaker=", (sizeof("-speaker=" - 1)))) {
				sndConfig.dev_cfg.ear_mute = atoi(&token[sizeof("-speaker=") - 1]);
			} else if (!memcmp(token,"-agc=", (sizeof("-agc=" - 1)))) {
				sndConfig.agc_value = atoi(&token[sizeof("-agc=") - 1]);
			} else if(!memcmp(token,"-avc=", (sizeof("-avc=" - 1)))) {
				sndConfig.avc_value = atoi(&token[sizeof("-avc=") - 1]);
			}
			token = strtok(NULL, " ");
		}
			
		printf("%s : snd settings:\n"
                "\tdevice: %d\n"
                "\tmethod: %s\n"
                "\tvolume: %d\n"
                "\tmute mic: %s\n"
                "\tmute speaker: %s\n"
                "\tagc: %d\n"
                "\tavc: %d\n",
                __FUNCTION__,
                sndConfig.devIndex,
                sndConfig.method,
                sndConfig.vol_cfg.volume,
                (sndConfig.dev_cfg.mic_mute ? "yes" : "no"),
                (sndConfig.dev_cfg.ear_mute ? "yes" : "no"),
                sndConfig.agc_value,
                sndConfig.avc_value);
                
		pthread_create( &context->thread, NULL, sndtest_thread, 
						(void*) context);

	}
	return 0;
}

const char *sndsetdev_help_txt = 
        "\t --- snd set device --- \n \
echo \"sndsetdev -dev=devIndex -meth=method -vol=xx -mic=xx -speaker=xx -agc=xx -avc=xx\" > tmp/audio_test \n\
e.g. \n\
echo \"sndsetdev -dev=1 -meth=voice -vol=4\" > tmp/audio_test \n\
devIndex: 0 - 12, defaut: handset \n\
mehtod: one of voice, key_beep, message, ring, midi, aux  default:voice \n\
volume: 1 - 6, default:3 \n\
agc, avc: 0 -1, 0:disable, 1 enable\n\
mic, speaker: 0 - 1, 0:unmuted, 1 muted \n\
by default: unmuted for mic and speaker \n ";


void sndsetdev_help_menu(void) {
	printf("%s\n", sndsetdev_help_txt);
	 printf("\n--- Device List ---\r\n"\
               " \r\n" \
               "  0. Handset \n"\
               "  1. HFK \n"\
               "  2. Mono Headset \n"\
               "  3. Stereo Headset \n"\
               "  4. AHFK           \n"\
               "  5. Stereo DAC     \n"\
               "  6. Speaker phone  \n"\
               "  7. TTY HFK \n"\
               "  8. TTY Headset \n"\
               "  9. TTY VCO  \n"\
               "  10. TTY HCO  \n"\
               "  11. BT INTERCOM \n"\
               "  12. BT Headset  \n"\
               "                  \n\n");
}

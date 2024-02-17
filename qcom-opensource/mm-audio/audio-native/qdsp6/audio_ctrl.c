/* audio_ctrl.c - native audio control test application
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

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/ioctl.h>

#include <linux/msm_audio.h>
#include "audiotest_def.h"

#define HA_MIC                     0x107ac8d
#define HA_SPKR                    0x107ac88
#define HE_MIC                     0x1081510
#define HE_SPKR_MONO               0x1081511
#define HE_SPKR_STEREO             0x107ac8a
#define SP_PHONE_MIC                  0x1081512
#define SP_PHONE_MONO                 0x1081513
#define SP_PHONE_STEREO               0x1081514
#define BT_SCO_MIC_DEVICE                      0x1081518
#define BT_SCO_SPKR_DEVICE                     0x1081519
#define BT_A2DP_SPKR_DEVICE                    0x108151a
#define TTY_HE_MIC                 0x108151b
#define TTY_HE_SPKR                0x108151c
#define HE_MONO_PLUS_SPKR_MONO_RX         0x108c508
#define HE_MONO_PLUS_SPKR_STEREO_RX       0x108c895
#define HE_STEREO_PLUS_SPKR_MONO_RX       0x108c894
#define HE_STEREO_PLUS_SPKR_STEREO_RX     0x108c509
#define I2S_RX_DEVICE                          0x1089bf4
#define I2S_TX_DEVICE                          0x1089bf3

int switch_to_device(int rxdev, int txdev)
{
	int fd;
	int rc = 0;
	unsigned long tx_device;
	unsigned long rx_device;
	int arr[2] = {0, 0};

	fd = open("/dev/msm_audio_ctl", O_RDWR);
	if (fd < 0) {
		perror("audio_ctrl: cannot open audio control device");
		return -1;
	}

	switch (rxdev) {
	case 2:
		printf("switching to HANDSET_SPKR\n");
		rx_device = HA_SPKR;
		break;
	case 4:
		printf("switching to HEADSET_SPKR_MONO\n");
		rx_device = HE_SPKR_MONO;
		break;
	case 5:
		printf("switching to HEADSET_SPKR_STEREO\n");
		rx_device = HE_SPKR_STEREO;
		break;
	case 7:
		printf("switching to SPKR_PHONE_MONO\n");
		rx_device = SP_PHONE_MONO;
		break;
	case 8:
		printf("switching to SPKR_PHONE_STEREO\n");
		rx_device = SP_PHONE_STEREO;
		break;
	case 10:
		printf("switching to BT_SCO_SPKR\n");
		rx_device = BT_SCO_SPKR_DEVICE;
		break;
	case 11:
		printf("switching to BT_A2DP_SPKR\n");
		rx_device = BT_A2DP_SPKR_DEVICE;
		break;
	case 13:
		printf("switching to TTY_HEADSET_SPKR\n");
		rx_device = TTY_HE_SPKR;
		break;
	case 14:
		printf("switching to TTY HCO\n");
		tx_device = TTY_HE_MIC;
		rx_device = HA_SPKR;
		break;
	case 15:
		printf("switching to TTY VCO\n");
		tx_device = HA_MIC;
		rx_device = TTY_HE_SPKR;
		break;
        case 16:
                printf("headset mono plus speaker phone mono\n");
                tx_device = SP_PHONE_MIC;
                rx_device = HE_MONO_PLUS_SPKR_MONO_RX;
                break;
        case 17:
                printf("headset mono plus speaker phone stereo\n");
                tx_device = SP_PHONE_MIC;
                rx_device = HE_MONO_PLUS_SPKR_STEREO_RX;
                break;
        case 18:
                printf("headset stereo plus speaker phone mono\n");
                tx_device = SP_PHONE_MIC;
                rx_device = HE_STEREO_PLUS_SPKR_MONO_RX;
                break;
        case 19:
                printf("headset stereo plus speaker phone stereo\n");
                tx_device = SP_PHONE_MIC;
                rx_device = HE_STEREO_PLUS_SPKR_STEREO_RX;
                break;
	case 32:
		printf("switching to I2S_RX_DEVICE\n");
		rx_device = I2S_RX_DEVICE;
		break;
	default:
		printf("switching to rx device %d\n", rxdev);
		rx_device = rxdev;
		break;
	}

	switch (txdev) {
	case 1:
		printf("switching to HANDSET_MIC\n");
		tx_device = HA_MIC;
		break;
	case 3:
		printf("switching to HEADSET_MIC\n");
		tx_device = HE_MIC;
		break;
	case 6:
		printf("switching to SPKR_PHONE_MIC\n");
		tx_device = SP_PHONE_MIC;
		break;
	case 9:
		printf("switching to BT_SCO_MIC\n");
		tx_device = BT_SCO_MIC_DEVICE;
		break;
	case 12:
		printf("switching to TTY_HEADSET_MIC\n");
		tx_device = TTY_HE_MIC;
		break;
	case 33:
		printf("switching to I2S_TX_DEVICE\n");
		tx_device = I2S_TX_DEVICE;
		break;
	default:
		printf("switching to tx device %d\n", txdev);
		tx_device = txdev;
		break;
	}

	printf("rx device = %d\n",(int)rx_device);
	arr[0] = rx_device;
	if (ioctl(fd, AUDIO_SWITCH_DEVICE, arr)) {
		perror("could not switch device");
		rc = -1;
		goto done;
	}

	printf("tx device = %d\n",(int)tx_device);
	arr[0] = tx_device;
	if (ioctl(fd, AUDIO_SWITCH_DEVICE, arr)) {
		perror("could not switch device");
		rc = -1;
		goto done;
	}

done:
	close(fd);

	return rc;
}

int master_vol(uint32_t volume)
{
	int fd;
	int rc = 0;

	fd = open("/dev/msm_audio_ctl", O_RDWR);
	if (fd < 0) {
		perror("audio_ctrl: cannot open audio control device");
		return -1;
	}

	volume *= 20; //percentage
	printf("Setting in-call volume to %d\n", volume);
	if (ioctl(fd, AUDIO_SET_VOLUME, &volume)) {
		perror("could not set volume");
		rc = -1;
		goto done;
	}

done:
	close(fd);
	return rc;
}

int master_mute(uint32_t mute)
{
	int fd;
	int rc = 0;

	printf("set mute: %d\n", mute);

	fd = open("/dev/msm_audio_ctl", O_RDWR);
	if (fd < 0) {
		perror("audio_ctrl: cannot open audio control device");
		return -1;
	}

	if (ioctl(fd, AUDIO_SET_MUTE, &mute)) {
		perror("could not set mute");
		rc = -1;
		goto done;
	}

done:
	close(fd);
	return rc;
}

int txdev, rxdev;
uint32_t volume, mute;

void* switchdev_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = switch_to_device(rxdev, txdev);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

void* mastervol_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = master_vol(volume);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

void* mastermute_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = master_mute(mute);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int switchdev_read_params(void) {
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
				} else if (!memcmp(token,"-txdev=", (sizeof("-txdev=") - 1))) {
					txdev = atoi(&token[sizeof("-txdev=") - 1]);
				} else if (!memcmp(token,"-rxdev=", (sizeof("-rxdev=") - 1))) {
					rxdev = atoi(&token[sizeof("-rxdev=") - 1]);
				} else {
					context->config.file_name = token;
				}
				token = strtok(NULL, " ");
			}
			context->type = AUDIOTEST_TEST_MOD_SWITCH_DEV;
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL,
					switchdev_thread, (void*) context);
		}
	}
	return ret_val;
}

int mastervol_read_params(void) {
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
				} else if (!memcmp(token,"-volume=", (sizeof("-volume=") - 1))) {
					volume = atoi(&token[sizeof("-volume=") - 1]);
				} else {
					context->config.file_name = token;
				}
				token = strtok(NULL, " ");
			}
			context->type = AUDIOTEST_TEST_MOD_MASTER_VOL;
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL,
					mastervol_thread, (void*) context);
		}
	}
	return ret_val;
}

int mastermute_read_params(void) {
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
				} else if (!memcmp(token,"-mute=", (sizeof("-mute=") - 1))) {
					mute = atoi(&token[sizeof("-mute=") - 1]);
				} else {
					context->config.file_name = token;
				}
				token = strtok(NULL, " ");
			}
			context->type = AUDIOTEST_TEST_MOD_MASTER_MUTE;
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL,
					mastermute_thread, (void*) context);
		}
	}
	return ret_val;
}

/*int switchdev_control_handler(void* private_data) {
	int ret_val = 0;
	char *token;

	token = strtok(NULL, " ");
	if ((private_data != NULL) &&
		(token != NULL)) {
		if(!memcmp(token,"-cmd=", (sizeof("-cmd=") -1))) {
			token = &token[sizeof("-cmd=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
		}
	} else {
		ret_val = -1;
	}

	return ret_val;
}*/

const char *switchdev_help_txt =
"Switch device: txdev rxdev \n\
echo \"switchdev -id=xxx -txdev=x -rxdev=x\" > /data/audio_test \n\
txdev = device no, rxdev = device no \n\
\t\tdevices:\n\
\t\t1  = handset mic\n\
\t\t2  = handset speaker\n\
\t\t3  = headset mic\n\
\t\t4  = headset speaker mono\n\
\t\t5  = headset speaker stereo\n\
\t\t6  = speaker phone mic\n\
\t\t7  = speaker phone mono\n\
\t\t8  = speaker phone stereo\n\
\t\t9  = bt sco mic\n\
\t\t10 = bt sco speaker\n\
\t\t11 = bt a2dp speaker\n\
\t\t12 = tty headset mic\n\
\t\t13 = tty headset speaker\n\
\t\t14 = tty HCO\n\
\t\t15 = tty VCO\n\
\t\t16 = headset mono plus speaker phone mono\n\
\t\t17 = headset mono plus speaker phone stereo\n\
\t\t18 = headset stereo plus speaker phone mono\n\
\t\t19 = headset stereo plus speaker phone stereo\n\
\t\t20 = headset stereo plus speaker stereo rx\n\
\t\t32 = i2s rx\n\
\t\t33 = i2s tx\n";

const char *mastervol_help_txt =
"Master volume: volume \n\
echo \"mastervol -id=xxx -volume=x \" > /data/audio_test \n\
volume = 0 to 5 \n ";

const char *mastermute_help_txt =
"Master mute: mute \n\
echo \"mastermute -id=xxx -mute=x \" > /data/audio_test \n\
mute = 0 or 1 \n ";

void switchdev_help_menu(void) {
	printf("%s\n", switchdev_help_txt);
}

void mastervol_help_menu(void) {
	printf("%s\n", mastervol_help_txt);
}

void mastermute_help_menu(void) {
	printf("%s\n", mastermute_help_txt);
}

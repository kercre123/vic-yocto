/* sbctest.c - native SBC record test application
 *
 * Based on native sbc test application platform/system/extras/sound/playsbc.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2009-2010, 2012 The Linux Foundation. All rights reserved.
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
#include <linux/msm_audio_sbc.h>
#include <pthread.h>
#include <errno.h>
#include "audiotest_def.h"

#ifdef _ANDROID_
static const char *cmdfile = "/data/audio_test";
#else
static const char *cmdfile = "/tmp/audio_test";
#endif

static const char     *dev_file_name;
static int rec_stop = 1;

int sbc_rec(struct audtest_config *config)
{
	struct msm_audio_sbc_enc_config *sbc_data = (struct msm_audio_sbc_enc_config *) config->private_data;
	unsigned char buf[8192];
	struct msm_audio_sbc_enc_config cfg;
	struct msm_audio_stream_config buf_cfg;
	unsigned sz;
	int fd, afd;
	unsigned total = 0;
	unsigned short enc_id;
	const char *device = "a2dp_tx";
	int device_id;
	size_t batch_size;
	int rc = -1;

	if (sbc_data->mode > 3) {
		perror("invalid channel mode \n");
		return -1;
	} else if (sbc_data->sample_rate != 48000) {
		perror("invalid sample rate \n");
		return -1;
	} else if (sbc_data->bit_allocation > 1) {
		perror("Invalid bit allocation\n");
		return -1;
	} else if (sbc_data->number_of_subbands != 1) {
		perror("Invalid number of Bands\n");
		return -1;
	} else if (sbc_data->number_of_blocks > 3) {
		perror("Invalid number of blocks\n");
		return -1;
	}

	fd = open(config->file_name, O_CREAT | O_RDWR, 0666);
	if (fd < 0) {
		perror("cannot open output file");
		return -1;
	}

	afd = open(dev_file_name, O_RDWR);
	if (afd < 0) {
		perror("cannot open msm_sbc_in");
		close(fd);
		return -1;
	}

	if (ioctl(afd, AUDIO_GET_SESSION_ID, &enc_id)) {
		perror("could not get encoder session id\n");
		goto error;
	}
	printf("enc_id = %d\n", enc_id);
#if defined(TARGET_USES_QCOM_MM_AUDIO)
/*	if (devmgr_register_session(enc_id, DIR_TX) < 0) {
		goto error;
    	} */
	device_id = msm_get_device(device);
	printf("Routing to A2DP\n");
	if (devmgr_enable_device(device_id, DIR_TX) < 0){
		perror("could not enable TX device\n");
		return -1;
	}
	if (msm_route_stream(DIR_TX, enc_id, device_id, 1) < 0) {
		perror("could not route stream to Device\n");
		if (devmgr_disable_device(device_id, DIR_RX) < 0)
			perror("could not disable device\n");
		return -1;
	}
#endif
	/* Config param */
	if(ioctl(afd, AUDIO_GET_STREAM_CONFIG, &buf_cfg)) {
		printf("Error getting buf config param\n");
		goto error;
	}

	if(ioctl(afd, AUDIO_SET_STREAM_CONFIG, &buf_cfg)) {
		printf("Error setting buf config param\n");
		goto error;
	}
	printf("Default buffer size = 0x%8x\n", buf_cfg.buffer_size);
	printf("Default buffer count = 0x%8x\n",buf_cfg.buffer_count);

	if (ioctl(afd, AUDIO_GET_SBC_ENC_CONFIG, &cfg)) {
		perror("cannot read audio config");
		goto fail;
	}

	cfg.bit_allocation = sbc_data->bit_allocation;
	cfg.mode = sbc_data->mode;
	cfg.number_of_subbands = sbc_data->number_of_subbands;
	cfg.number_of_blocks = sbc_data->number_of_blocks;
	if (cfg.mode) /* Mono */ {
		if (sbc_data->bit_rate > 512000)
			cfg.bit_rate = 512000;
		else
			cfg.bit_rate = sbc_data->bit_rate;
	} else {
		if (sbc_data->bit_rate > 320000)
			cfg.bit_rate = 320000;
		else
			cfg.bit_rate = sbc_data->bit_rate;
	}
	cfg.sample_rate = sbc_data->sample_rate;

	if (ioctl(afd, AUDIO_SET_SBC_ENC_CONFIG, &cfg)) {
		perror("cannot write audio config");
		goto fail;
	}

	sz = buf_cfg.buffer_size;
	fprintf(stderr, "buffer size %d\n", sz);
	if (sz > sizeof(buf)) {
		fprintf(stderr, "buffer size %d too large\n", sz);
		goto fail;
	}

	if (ioctl(afd, AUDIO_START, 0) < 0) {
		perror("cannot start audio");
		goto fail;
	}
	rec_stop = 0;
	fprintf(stderr,"\n*** RECORDING IN PROGRESS ***\n");

	while(!rec_stop) {
		batch_size = read(afd, buf, sz);
		printf("batch_size = %d\n", batch_size);
		if (write(fd, buf, batch_size) != (ssize_t)batch_size) {
			perror("cannot write buffer");
			goto fail;
		}
		total += batch_size;
	}

	rc = 0;
fail:
#if defined(TARGET_USES_QCOM_MM_AUDIO)
/*	if (devmgr_unregister_session(enc_id, DIR_TX) < 0)
			return -1; */

	printf("Derouting from A2DP\n");
	if (devmgr_disable_device(device_id, DIR_TX) < 0){
		perror("could not enable TX device\n");
		return -1;
	}
	if (msm_route_stream(DIR_TX, enc_id, device_id, 0) < 0) {
		perror("could not route stream to Device\n");
		if (devmgr_disable_device(device_id, DIR_RX) < 0)
			perror("could not disable device\n");
		return -1;
	}
#endif
error:
	close(afd);
	close(fd);
	unlink(config->file_name);
	return rc;
}

void* recsbc_thread(void* arg) {
	struct audiotest_thread_context *context = 
	(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = sbc_rec(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int sbcrec_read_params(void) {
	struct audiotest_thread_context *context;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		struct msm_audio_sbc_enc_config *sbc_data;
		sbc_data = (struct msm_audio_sbc_enc_config *) malloc(sizeof(struct msm_audio_sbc_enc_config));
		if (!sbc_data) {
			printf("Error in allocating sbc structure\n");
			free_context(context);
			ret_val = -1;
		} else {
			context->config.file_name = "/data/record.sbc";
			dev_file_name = "/dev/msm_a2dp_in";
			context->type = AUDIOTEST_TEST_MOD_SBC_ENC;
			sbc_data->bit_allocation = AUDIO_SBC_BA_SNR;
			sbc_data->mode = AUDIO_SBC_MODE_JSTEREO;
			sbc_data->number_of_subbands = AUDIO_SBC_BANDS_8;
			sbc_data->number_of_blocks = AUDIO_SBC_BLOCKS_16;
			sbc_data->bit_rate = 507000;
			sbc_data->sample_rate = 48000;

			token = strtok(NULL, " ");
			while (token != NULL) {
				printf("%s \n", token);
				if (!memcmp(token,"-rate=", (sizeof("-rate=") - 1))) {
					context->config.sample_rate = 
					atoi(&token[sizeof("-rate=") - 1]);
				} else if (!memcmp(token,"-ba=", (sizeof("-ba=") - 1))) {
					sbc_data->bit_allocation = 
					atoi(&token[sizeof("-ba=") - 1]);
				} else if (!memcmp(token,"-cmode=", (sizeof("-cmode=") - 1))) {
					sbc_data->mode = 
					atoi(&token[sizeof("-cmode=") - 1]);
				} else if (!memcmp(token,"-bands=", (sizeof("-bands=") - 1))) {
					sbc_data->number_of_subbands = 
					atoi(&token[sizeof("-bands=") - 1]);
				} else if (!memcmp(token,"-blocks=", (sizeof("-blocks=") - 1))) {
					sbc_data->number_of_blocks = 
					atoi(&token[sizeof("-blocks=") - 1]);
				} else if (!memcmp(token,"-bitrate=", (sizeof("-bitrate=") - 1))) {
					sbc_data->bit_rate = 
					atoi(&token[sizeof("-bitrate=") - 1]);
				} else if (!memcmp(token,"-id=", (sizeof("-id=") - 1))) {
					context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
				} else if (!memcmp(token, "-dev=",
						(sizeof("-dev=") - 1))) {
					dev_file_name = token + (sizeof("-dev=")-1);
				} else {
					context->config.file_name = token;
				}
				token = strtok(NULL, " ");  
			}
			context->config.private_data = (struct msm_audio_sbc_enc_config *) sbc_data;
			pthread_create( &context->thread, NULL, recsbc_thread, (void*) context);  
		}
	}
	return ret_val;
}

int sbc_rec_control_handler(void* private_data) {
	int /* drvfd ,*/ ret_val = 0;
	char *token;

	printf("%s: cmd \n", __FUNCTION__);
	token = strtok(NULL, " ");
	if ((private_data != NULL) && 
		(token != NULL)) {
		/* drvfd = (int) private_data */
		if(!memcmp(token,"-cmd=", (sizeof("-cmd=") -1))) {
			token = &token[sizeof("-cmd=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
			if (!strcmp(token, "stop")) {
				rec_stop = 1;
			}
		}
	} else {
		ret_val = -1;
	}

	return ret_val;
}

const char *sbcrec_help_txt = 
"Record sbc file: type \n\
echo \"recsbc path_of_file -rate=xxx -ba=x -cmode=x -bands=x -blocks=x -bitrate=x -id=xxx -dev=/dev/msm_a2dp_in\" > %s \n\
rate: 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000 \n\
ba: bit allocation 0 or 1 \n\
mode: 0 or 1 or 2 or 3\n\
bands: 1 \n\
blocks: 0 or 1 or 2 or 3 \n\
bitrate: 320000, 512000 \n\
Supported control command: stop\n ";

void sbcrec_help_menu(void) {
	printf(sbcrec_help_txt, cmdfile);
}

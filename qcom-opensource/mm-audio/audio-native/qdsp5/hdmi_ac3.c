/* hdmi_ac3.c - native PCM test application
 *
 * * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2009-2011, The Linux Foundation. All rights reserved.
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
#include "control.h"
#include "iec_60958_61937.h"
#include "audio_parsers.h"


const char  *dev_file_name;
static int quit, repeat;
static int pause_flag = 0;
static int play_state = 0;

static struct config_60958_61937 config_60958_61937;
static struct codec_61937_config codec_61937_config;
static struct audio_parser_codec_info audio_codec_info;

static unsigned char *ac3_61937_burst;
static unsigned char *hdmi_non_l_rep_per;
static unsigned char *ac3_frame;


#define DEV_FILE_NAME   "/dev/msm_lpa_if_out"
#define MAX_FILE_SIZE 128

static int hdmi_ac3_play(struct audtest_config *config)
{
	struct msm_audio_config audio_config;
	unsigned int ac3_file_sz;
	unsigned int actual_read_size;
	unsigned char *ac3_file_data;
	FILE *fp;
	int afd;
	int sz;
	unsigned int i;
	unsigned char *ch;
	int dev_id;
	int max_ac3_frame_sz, cur_ac3_frame_sz;
	char *device_name = "hdmi_pass_through";
	int rc = 0;
	int exit_on_fail = 0;
	unsigned int dma_buf_sz = 0;
	struct msm_audio_config aud_config;
	play_state = 1;

	fprintf(stderr, "%s():\n", __func__);


	/******************** read_file ***************************************/
	while (repeat) {
	fprintf(stderr, "Repeat %d\n", repeat);
	fp = fopen(config->file_name,"rb");
	if (fp == NULL) {
		fprintf(stderr, "hdmi_ac3 : cannot open '%s'\n",
				config->file_name);
		return -1;
	}

	/* Get file size and reset file position */
	fseek(fp, 0, SEEK_END);
	ac3_file_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	fprintf(stderr, "size of AC-3 file %s = %u\n",
			config->file_name, ac3_file_sz);

	ac3_file_data = (unsigned char *)
		(malloc(sizeof(unsigned char) * ac3_file_sz));

	if (ac3_file_data == NULL) {
		fprintf(stderr, "could not allocate memeory for Ac-3 file data."
			" file length %u\n" , ac3_file_sz);
		fclose(fp);
		return -1;
	}

	actual_read_size = fread((void*) ac3_file_data, 1, ac3_file_sz, fp);

	if (actual_read_size != ac3_file_sz) {
		fprintf(stderr, "could not read AC-3 file %s\n",
				config->file_name);
		free(ac3_file_data);
		fclose(fp);
		return -1;
	}

	max_ac3_frame_sz = init_audio_parser(ac3_file_data, ac3_file_sz,
			AUDIO_PARSER_CODEC_AC3);

	ac3_frame = (unsigned char *)
		(malloc(sizeof(unsigned char) * max_ac3_frame_sz));

	if (ac3_frame == NULL) {
		fprintf(stderr, "could not allocate memeory for Ac-3 frame."
			" frame size %u\n" ,max_ac3_frame_sz);
		free(ac3_file_data);
		fclose(fp);
		return -1;
	}
	init_60958_61937_framer();

	/********** end of read_file ******************************************/

	dev_id = msm_get_device(device_name);

	if (msm_en_device(dev_id,1)) {
		fprintf(stderr, "could not enable device %s\n",
				device_name);
		exit_on_fail = 1;
		goto error_en_alsa_dev;
	}

	afd = open(DEV_FILE_NAME, O_WRONLY);
	if (afd < 0) {
		fprintf(stderr, "cannot open audio device\n");
		rc = -1;
		exit_on_fail = 1;
		goto error_open_dev;
	}

	config_60958_61937.codec_type = IEC_61937_CODEC_AC3;

	rc = get_60958_61937_config(&config_60958_61937);

	if(rc == -1) {
		fprintf(stderr, "get_60958_61937_config failed\n");
		rc = -1;
		exit_on_fail = 1;
		goto error_get_60958_61937_config;
	}
	dma_buf_sz = config_60958_61937.dma_buf_sz;
	fprintf(stderr, "Dma buf_sz %d\n", dma_buf_sz);

	ac3_61937_burst =  (unsigned char *)
		(malloc(config_60958_61937.sz_61937_burst));

	if (ac3_61937_burst == NULL) {
		fprintf(stderr, "could not allocate memory for ac3 61937 burst."
			" burst size %u\n" , config_60958_61937.sz_61937_burst);
		rc = -1;
		exit_on_fail = 1;
		goto error_no_mem_ac3_61937_burst;
	}

	hdmi_non_l_rep_per  = (unsigned char *)
		(malloc(config_60958_61937.rep_per_60958));


	if (hdmi_non_l_rep_per == NULL) {
		fprintf(stderr, "could not allocate memeory for ac3 60958 "
			"rep per frames. ac3 60958rep per frame size %u\n" ,
			config_60958_61937.rep_per_60958);
		rc = -1;
		exit_on_fail = 1;
		goto error_no_mem_hdmi_non_l_rep_per;
	}

	fprintf(stderr, "burst length  %d , rep_per_60958 %d \n",
			config_60958_61937.sz_61937_burst,
			config_60958_61937.rep_per_60958);


	audio_codec_info.codec_type = AUDIO_PARSER_CODEC_AC3;
	rc = get_first_frame_info(&audio_codec_info);

	if (rc < 0 ) {
		fprintf(stderr, "Failed to get first frame indfo\n");
		return -1;
	}
	aud_config.buffer_size = dma_buf_sz;
	aud_config.sample_rate = audio_codec_info.codec_config.ac3_fr_info.sample_rate;
	fprintf(stderr, "Sample_rate = %d dma buf size = %d",
			aud_config.sample_rate, dma_buf_sz);

	if (ioctl(afd, AUDIO_SET_CONFIG, &aud_config)) {
		fprintf(stderr, "could not set AUDIO_SET_IEC_CODEC_CONFIG\n");
		rc = -1;
		exit_on_fail = 1;
		goto error_ioctl_audio_get_config;
	}
	if (ioctl(afd, AUDIO_GET_CONFIG, &audio_config)) {
		fprintf(stderr, "could not get audio_config\n");
		rc = -1;
		exit_on_fail = 1;
		goto error_ioctl_audio_get_config;
	}

	fprintf(stderr, "initiate_play: buffer_size=%d, buffer_count=%d\n",
			audio_config.buffer_size, audio_config.buffer_count);

	fprintf(stderr, "prefill: send Pause Fram\n");

	get_60958_61937_pause_burst(hdmi_non_l_rep_per,
			config_60958_61937.rep_per_60958, &config_60958_61937);


	sz = config_60958_61937.rep_per_60958;

	/* Driver requires two buffers to be prefilled before AUDIO_START */
	for (i = 0; i < 2; i++) {

		if (write(afd, hdmi_non_l_rep_per, sz) != sz) {
			fprintf(stderr, "could not write pause frame %d\n", i);
			rc = -1;
			exit_on_fail = 1;
			goto error_dev_write;
		}
	}

	fprintf(stderr, "start playback\n");

	rc = ioctl(afd, AUDIO_START, 0);

	if (rc < 0 ) {
		fprintf(stderr, "%s: Unable to start driver\n", __func__);
		rc = 1;
		exit_on_fail = 1;
		goto error_ioctl_audio_start;
	}

	for (i = 0; i < 12; i++) {

		if (write(afd, hdmi_non_l_rep_per, sz) != sz) {
			fprintf(stderr, "could not write pause frame %d\n", i);
			rc = -1;
			exit_on_fail = 1;
			goto error_dev_write;
		}
	}

	sz = config_60958_61937.rep_per_60958;

	audio_codec_info.codec_type = AUDIO_PARSER_CODEC_AC3;

	memset(&codec_61937_config, sizeof(struct codec_61937_config), 0);
	for (;;) {
		if (!pause_flag) {
			rc = get_audio_frame(ac3_frame, max_ac3_frame_sz,
						&audio_codec_info);

			if (rc < 0 ) {
				fprintf(stderr, "%s: no more aduio frames\n",
					__func__);
				rc = 0;
				break;
			}

			codec_61937_config.codec_type = IEC_61937_CODEC_AC3;

			codec_61937_config.codec_config.ac3_fr_config.ac3_fr_sz_16bit =
				audio_codec_info.codec_config.ac3_fr_info.ac3_fr_sz_16bit;

			codec_61937_config.codec_config.ac3_fr_config.bsmod =
				audio_codec_info.codec_config.ac3_fr_info.bsmod;

			codec_61937_config.codec_config.ac3_fr_config.sample_rate =
				audio_codec_info.codec_config.ac3_fr_info.sample_rate;

			codec_61937_config.codec_config.ac3_fr_config.reverse_bytes =
				audio_codec_info.codec_config.ac3_fr_info.reverse_bytes;

			cur_ac3_frame_sz =  2 *
				codec_61937_config.codec_config.ac3_fr_config.ac3_fr_sz_16bit;

			get_61937_burst(ac3_61937_burst,
					config_60958_61937.sz_61937_burst,
					ac3_frame, cur_ac3_frame_sz,
					&codec_61937_config);

			get_60958_frame(hdmi_non_l_rep_per,
					config_60958_61937.rep_per_60958,
					ac3_61937_burst,
					config_60958_61937.sz_61937_burst,
					&codec_61937_config);
		} else {
			get_60958_61937_pause_burst(hdmi_non_l_rep_per,
					config_60958_61937.rep_per_60958,
					&config_60958_61937);
		}
		if (write(afd, hdmi_non_l_rep_per, sz) != sz) {
			fprintf(stderr, "could not write %s\n", DEV_FILE_NAME);
			exit_on_fail = 1;
			break;
		}
	}


error_ioctl_audio_start:
error_dev_write:

error_ioctl_audio_get_config:
	free(hdmi_non_l_rep_per);

error_no_mem_hdmi_non_l_rep_per:
	free(ac3_61937_burst);

error_no_mem_ac3_61937_burst:

error_get_60958_61937_config:
	close(afd);

error_open_dev:
	if (msm_en_device(dev_id, 0) < 0)
		fprintf(stderr, "ERROR: could not disable %s\n",
				device_name);

error_en_alsa_dev:
	free(ac3_frame);
	fclose(fp);
	deinit_60958_61937_framer();
	free(ac3_file_data);

	repeat--;
	if (repeat > 0) {
		if(exit_on_fail == 1)
			goto exit;
		sleep(5);
	}
	}
	fprintf(stderr, "End of playback\n");
exit:
	play_state = 0;
	return rc;
}


static void* hdmi_ac3_thread(void* arg) {
	struct audiotest_thread_context *context =
	(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = hdmi_ac3_play(&context->config);
	free(context->config.file_name);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int hdmi_ac3_read_params(void) {
	struct audiotest_thread_context *context;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		if (!play_state) {
			context->config.file_name = "/data/data.ac3";
			dev_file_name = "/dev/msm_lpa_if_out";
			repeat = 1;
			quit = 0;
			pause_flag = 0;
		}

		token = strtok(NULL, " ");

		while (token != NULL) {
			if (!memcmp(token,"-id=", (sizeof("-id=")-1))) {
				context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
			} else if (!memcmp(token, "-dev=",
					(sizeof("-dev=") - 1))) {
				dev_file_name = token + (sizeof("-dev=")-1);
			} else if (!memcmp(token, "-repeat=",
					(sizeof("-repeat=") - 1))) {
				repeat = atoi(&token[sizeof("-repeat=") - 1]);
			} else if (!memcmp(token, "-pause=",
					(sizeof("-pause=")-1))) {
				pause_flag = atoi(&token[sizeof("-pause=") - 1]);
				free_context(context);
				return ret_val;
                        } else {
				context->config.file_name = (char*)malloc(MAX_FILE_SIZE);
				if (!context->config.file_name)
					return -1;
				strlcpy(context->config.file_name, token, MAX_FILE_SIZE);
			}
			token = strtok(NULL, " ");
		}
		context->type = AUDIOTEST_TEST_MOD_PCM_DEC;
		pthread_create( &context->thread, NULL,
				hdmi_ac3_thread, (void*) context);
	}

	return ret_val;

}

const char *hdmi_ac3_help_txt =
	"To Play ac3 file on hdmi: type \n"
"echo \"hdmi_ac3 path_of_file -id=xxx  -dev=/dev/msm_lpa_if_out \" > tmp/audio_test \n"
	 "To Pause ac3 file on hdmi: type \n"
"echo \"hdmi_ac3 -pause=0/1 \n";


void hdmi_ac3_help_menu(void) {
	printf("%s\n", hdmi_ac3_help_txt);
}


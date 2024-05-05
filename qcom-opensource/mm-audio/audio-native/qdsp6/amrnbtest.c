/* amrnbtest.c - native AMRNB test application
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>
#include <linux/msm_audio_amrnb.h>
#include "audiotest_def.h"

/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */
struct wav_header {		/* Simple wave header */
	char Chunk_ID[4];	/* Store "RIFF" */
	unsigned int Chunk_size;
	char Riff_type[4];	/* Store "WAVE" */
	char Chunk_ID1[4];	/* Store "fmt " */
	unsigned int Chunk_fmt_size;
	unsigned short Compression_code;	/*1 - 65,535,  1 - pcm */
	unsigned short Number_Channels;	/* 1 - 65,535 */
	unsigned int Sample_rate;	/*  1 - 0xFFFFFFFF */
	unsigned int Bytes_Sec;	/*1 - 0xFFFFFFFF */
	unsigned short Block_align;	/* 1 - 65,535 */
	unsigned short Significant_Bits_sample;	/* 1 - 65,535 */
	char Chunk_ID2[4];	/* Store "data" */
	unsigned int Chunk_data_size;
} __attribute__ ((packed));

static char *next;
static unsigned avail;

static int do_amr_play(struct audtest_config *clnt_config, unsigned rate, unsigned channels,
                    int (*fill)(void *buf, unsigned sz, void *cookie),
                    void *cookie)
{
	struct msm_audio_config config;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
	//  struct msm_audio_stats stats;
	unsigned sz;
	char buf[32768];
	int afd;
	int cntW=0;
	int volume = 1200;

	afd = open("/dev/msm_amr", O_RDWR);
	if (afd < 0) {
		perror("amr_play: cannot open audio device");
		return -1;
	}

	if (ioctl(afd, AUDIO_GET_CONFIG, &config)) {
		perror("could not get config");
		return -1;
	}

	config.channel_count = channels;
	config.sample_rate = rate;
/*<peter>
	if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
		perror("could not set config");
		return -1;
	}
*/
	sz = 4096;//config.buffer_size;
	printf("amr_play: sz=%d, buffer_count=%d\n",sz,config.buffer_count);

	if (sz > sizeof(buf)) {
		fprintf(stderr,"too big\n");
		return -1;
	}

	fprintf(stderr,"start\n");
	ioctl(afd, AUDIO_START, 0);

	for (;;) {
		int cnt = 0;
		if (fill(buf, sz, cookie)) {
			printf(" fill return NON NULL, exit loop \n");
			break;
		}

		if( audio_data->g_test_volume ) {
			printf("volume = %d\n",volume);
			if (ioctl(afd, AUDIO_SET_VOLUME, &volume)) {
				perror("could not set volume");
			}
			volume -= 40;
		}

		cnt = write(afd, buf, sz);
                printf("write %d bytes, %d bytes reported\n",sz,cnt);
		cntW++;
		printf("amr_play: cntW=%d\n",cntW);
	}
	printf("end of play\n");
        /* let audio finish playing before close */
        sleep(3);
	close(afd);
	return 0;
}

static int rec_stop;
static char *next;
static unsigned avail;

static int fill_buffer(void *buf, unsigned sz, void *cookie)
{
	unsigned cpy_size = (sz < avail?sz:avail);

	printf("fill_buffer: avail=%d, next=%p\n",avail,next);
	if (avail == 0) {
		return -1;
	}
	memcpy(buf, next, cpy_size);
	next += cpy_size;
	avail -= cpy_size;
	printf("fill_buffer: avail=%d, next=%p\n",avail,next);
	return 0;
}

static void play_amr_file(struct audtest_config *config, unsigned rate, unsigned channels,
                      int fd, unsigned count)
{
	next = (char*)malloc(count);
	printf(" play_file: count=%d\n", count);
	if (!next) {
		fprintf(stderr,"could not allocate %d bytes\n", count);
		return;
	}
	if (read(fd, next, count) != count) {
		fprintf(stderr,"could not read %d bytes\n", count);
		//return;
	}
	avail = count;
	do_amr_play(config, rate, channels, fill_buffer, 0);
}

int amrnb_play(struct audtest_config *config)
{
	struct stat stat_buf;
	int fd;

	if (config == NULL) {
		return -1;
	}

	fd = open(config->file_name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "playamr: cannot open '%s'\n", config->file_name);
		return -1;
	}

	(void)fstat(fd, &stat_buf);

	play_amr_file(config, 48000, 1, fd, stat_buf.st_size);

	return 0;
}

int amrnb_rec(struct audtest_config *config)
{
	unsigned char buf[2048*10];
        struct msm_audio_stream_config amrnb_str_cfg;
        struct msm_audio_amrnb_enc_config_v2 amrnb_cfg;
	struct msm_voicerec_mode voicerec_mode;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config->private_data;
	unsigned sz ,framesize = 0;
	int fd, afd;
	unsigned char tmp;
	static unsigned int cnt = 0;

	fd = open(config->file_name, O_CREAT | O_RDWR, 0666);
	if (fd < 0) {
		perror("cannot open output file");
		return -1;
	}
	afd = open("/dev/msm_amr_in", O_RDWR);
	if (afd < 0) {
		perror("cannot open msm_amr_in");
		close(fd);
		return -1;
	}


	sz = 768;
	fprintf(stderr,"buffer size %d\n", sz);
	if (sz > sizeof(buf)) {
		fprintf(stderr,"buffer size %d too large\n", sz);
		goto fail;
	}

	amrnb_str_cfg.buffer_size = sz;
	amrnb_str_cfg.buffer_count=2;
	if (ioctl(afd, AUDIO_SET_STREAM_CONFIG, &amrnb_str_cfg)) {
		perror("cannot write audio config");
		goto fail;
	}

	amrnb_cfg.band_mode =  7;
        amrnb_cfg.dtx_enable = 0;

	if (ioctl(afd, AUDIO_SET_AMRNB_ENC_CONFIG, &amrnb_cfg)) {
		perror("cannot write audio config");
		goto fail;
	}

	voicerec_mode.rec_mode=2;
        if (ioctl(afd, AUDIO_SET_INCALL, &voicerec_mode)) {
                perror("cannot set incall mode");
                goto fail;
        }

	if (ioctl(afd, AUDIO_START, 0)) {
		perror("cannot start audio");
		goto fail;
	}

	fcntl(0, F_SETFL, O_NONBLOCK);
	fprintf(stderr,"\n*** RECORDING * USE 'STOP' CONTROL COMMAND TO STOP***\n");
	if(write(fd, "#!AMR\n", 6) != 6) {
		perror("cannot write AMR header");
		goto fail;
	}

	for (;(rec_stop!=1);) {
		framesize = read(afd, buf, sz);

		printf("AMR decoded frame num = %d , size = %d \n",++cnt,framesize);
		if(write(fd, buf, framesize) != framesize) {
			perror("cannot write buffer");
			goto fail;
		}
	}
done:
	close(afd);


	close(fd);
	return 0;

fail:
	close(afd);
	close(fd);
	unlink(config->file_name);
	return -1;
}

static void audiotest_alarm_handler(int sig)
{
	g_terminate_early = 1;
	sleep(1);
}

void* playamrnb_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = amrnb_play(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int amrnbplay_read_params(void) {
	struct audiotest_thread_context *context;
	struct itimerspec ts;
	char *token;
	int ret_val = 0;

	printf("This option is not supported currently\n");
	return -1;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/data.amr";
		g_terminate_early = 0;
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
					audio_data->g_test_volume = atoi(&token[sizeof("-volume=") - 1]);
				} else if (!memcmp(token,"-timeout=", (sizeof("-timeout=") - 1))) {
					audio_data->g_rec_timeout = atoi(&token[sizeof("-timeout=") - 1]);
					memset(&ts, 0, sizeof(struct itimerspec));
					printf("setting rec timeout to %d secs\n", audio_data->g_rec_timeout);
					ts.it_value.tv_sec = audio_data->g_rec_timeout;
					signal(SIGALRM, audiotest_alarm_handler);
					setitimer(ITIMER_REAL, &ts, NULL);
				} else {
					context->config.file_name = token;
				}
				token = strtok(NULL, " ");
			}
			context->type = AUDIOTEST_TEST_MOD_AMRNB_DEC;
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL,
					playamrnb_thread, (void*) context);
		}
	}
	return ret_val;
}

void* recamrnb_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = amrnb_rec(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int amrnbrec_read_params(void) {
	struct audiotest_thread_context *context;
	struct itimerspec ts;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/record.amr";
		context->type = AUDIOTEST_TEST_MOD_AMRNB_ENC;
		g_terminate_early = 0;
		struct audio_pvt_data *audio_data;
		audio_data = (struct audio_pvt_data *) malloc(sizeof(struct audio_pvt_data));
		if(!audio_data) {
			printf("error allocating audio instance structure \n");
			free_context(context);
			ret_val = -1;
		} else {
			token = strtok(NULL, " ");
			while (token != NULL) {
				printf("%s \n", token);
				if (!memcmp(token,"-id=", (sizeof("-id=") - 1))) {
					context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
				} else if (!memcmp(token,"-timeout=", (sizeof("-timeout=") - 1))) {
					audio_data->g_rec_timeout = atoi(&token[sizeof("-timeout=") - 1]);
					memset(&ts, 0, sizeof(struct itimerspec));
					printf("setting rec timeout to %d secs\n", audio_data->g_rec_timeout);
					ts.it_value.tv_sec = audio_data->g_rec_timeout;
					signal(SIGALRM, audiotest_alarm_handler);
					setitimer(ITIMER_REAL, &ts, NULL);
				} else {
					context->config.file_name = token;
					}
				token = strtok(NULL, " ");
			}
			printf("%s \n", __FUNCTION__);
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL, recamrnb_thread, (void*) context);
		}
	}
	return ret_val;
}

int amrnb_play_control_handler(void* private_data) {
	int /* drvfd ,*/ ret_val = 0;
	char *token;

	token = strtok(NULL, " ");
	if ((private_data != NULL) &&
			(token != NULL)) {
		/* drvfd = (int) private_data */
		if(!memcmp(token,"-cmd=", (sizeof("-cmd=") -1))) {
			token = &token[sizeof("-id=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
		}
	} else {
		ret_val = -1;
	}

	return ret_val;
}

int amrnb_rec_control_handler(void* private_data) {
	int /* drvfd ,*/ ret_val = 0;
	char *token;

	token = strtok(NULL, " ");
	if ((private_data != NULL) &&
		(token != NULL)) {
		/* drvfd = (int) private_data */
		if(!memcmp(token,"-cmd=", (sizeof("-cmd=") -1))) {
			token = &token[sizeof("-cmd=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
			if (!strcmp(token, "stop"))
				printf("Rec stop command\n");
				rec_stop = 1;
		}
	} else {
		ret_val = -1;
	}

	return ret_val;
}

const char *amrnbplay_help_txt =
"Play AMR file: type \n\
echo \"playamrnb path_of_file -id=xxx timeout=x -volume=x\" > /data/audio_test \n\
timeout = x (value in seconds) \n\
Volume = 0 (default, volume=100), 1 (test different volumes continuously) \n\
Bits per sample = 16 bits \n ";

void amrnbplay_help_menu(void) {
	printf("%s\n", amrnbplay_help_txt);
}

const char *amrnbrec_help_txt =
"Record amrnb file: type \n\
echo \"recamrnb path_of_file -id=xxx -timeout=x\" > /data/audio_test \n\
timeout = x (value in seconds) \n ";

void amrnbrec_help_menu(void) {
	printf("%s\n", amrnbrec_help_txt);
}

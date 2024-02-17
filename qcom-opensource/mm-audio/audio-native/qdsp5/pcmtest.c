/* pcmtest.c - native PCM test application
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2009-2012, The Linux Foundation. All rights reserved.
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
#include "equalizer.h"

#define DUAL_MIC_SOURCE		4
#define FRAME_NUM	128

const char     *dev_file_name;
static char *next, *org_next;
static unsigned avail, org_avail;
static int quit, repeat;
static int rec_source;
int rec_stop = 1;

struct audiot_buf_config {
	char    	*buf;
	uint32_t	head; /* next buffer app will write */
	uint32_t	tail; /* next buffer read() will read */
	int      	fd;
	int      	frame_available;
	int      	buffer_size; /* pcm min buffer size */
	uint32_t 	frame_count; /* total number of frames read */
	pthread_mutex_t cond_lock;
	pthread_mutex_t cmpl_lock;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
	pthread_t 	thread;
};

void wait_for_frame(struct audiot_buf_config *buf_cfg)
{
	pthread_mutex_lock(&buf_cfg->cond_lock);
	while (!buf_cfg->frame_available && !rec_stop ) {
		pthread_cond_wait(&buf_cfg->cond, &buf_cfg->cond_lock);
	}
	buf_cfg->frame_count--;
	if(!buf_cfg->frame_count)
		buf_cfg->frame_available = 0;
	pthread_mutex_unlock(&buf_cfg->cond_lock);
}

void frame_available(struct audiot_buf_config *buf_cfg)
{
	pthread_mutex_lock(&buf_cfg->cond_lock);
	if(buf_cfg->frame_available == 0) {
		buf_cfg->frame_available = 1;
	}
	buf_cfg->frame_count++;
	pthread_cond_signal(&buf_cfg->cond);
	pthread_mutex_unlock(&buf_cfg->cond_lock);
}

static int pcm_play(struct audtest_config *cfg, unsigned rate, 
					unsigned channels, int (*fill)(void *buf, 
												   unsigned sz, void *cookie), void *cookie)
{
	struct msm_audio_config config;
	// struct msm_audio_stats stats;
	unsigned n;
	int sz;
	char *buf; 
	int afd;
	int cntW=0;
	int ret = 0;
#ifdef AUDIOV2
	unsigned short dec_id;
#endif

	afd = open(dev_file_name, O_WRONLY);

	if (afd < 0) {
		perror("pcm_play: cannot open audio device");
		return -1;
	}

	cfg->private_data = (void*) afd;

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
#endif
#endif

	if (ioctl(afd, AUDIO_GET_CONFIG, &config)) {
		perror("could not get config");
		ret = -1;
		goto err_state;
	}

	config.channel_count = channels;
	config.sample_rate = rate;
	if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
		perror("could not set config");
		ret = -1;
		goto err_state;
	}

	buf = (char*) malloc(sizeof(char) * config.buffer_size);
	if (buf == NULL) {
		perror("fail to allocate buffer\n");
		ret = -1;
		goto err_state;
	}

	printf("initiate_play: buffer_size=%d, buffer_count=%d\n", config.buffer_size,
		   config.buffer_count);

	fprintf(stderr,"prefill\n");
	for (n = 0; n < config.buffer_count; n++) {
		if ((sz = fill(buf, config.buffer_size, cookie)) < 0)
			break;
		if (write(afd, buf, sz) != sz)
			break;
	}
	cntW=cntW+config.buffer_count; 

	fprintf(stderr,"start playback\n");
	if (ioctl(afd, AUDIO_START, 0) >= 0) {
		for (;;) {
#if 0
		if (ioctl(afd, AUDIO_GET_STATS, &stats) == 0)
			fprintf(stderr,"%10d\n", stats.out_bytes);
#endif
			if (((sz = fill(buf, config.buffer_size, cookie)) < 0) || (quit == 1)) {
				if ((repeat == 0) || (quit == 1)) {
					printf(" fill return NON NULL, exit loop \n");
					break;
				} else {
					printf("\nRepeat playback\n");
					avail = org_avail;
					next  = org_next;
					cntW = 0;
					if(repeat > 0)
						repeat--;
					sleep(1);
					continue;
				}
			}
			if (write(afd, buf, sz) != sz) {
				printf(" write return not equal to sz, exit loop\n");
				break;
			} else {
				cntW++;
				printf(" pcm_play: repeat_count=%d cntW=%d\n", repeat, cntW);
			}
		}
		printf("end of pcm play\n");
		sleep(5); 
	} else {
		printf("pcm_play: Unable to start driver\n");
	}
	free(buf);
err_state:
#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
	if (devmgr_unregister_session(dec_id, DIR_RX) < 0)
			ret = -1;
exit:
#endif
	close(afd);
	return ret;
}

/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

struct wav_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t riff_fmt;
	uint32_t fmt_id;
	uint32_t fmt_sz;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;		  /* sample_rate * num_channels * bps / 8 */
	uint16_t block_align;	  /* num_channels * bps / 8 */
	uint16_t bits_per_sample;
	uint32_t data_id;
	uint32_t data_sz;
};

static int fill_buffer(void *buf, unsigned sz,  void *cookie)
{
	unsigned cpy_size = (sz < avail?sz:avail);

	/* Below statement to remove warning for unused variable cookie,
	   but to keep function prototype intact */
	(void) cookie;

	if (avail == 0)
		return -1;

	memcpy(buf, next, cpy_size);
	next += cpy_size; 
	avail -= cpy_size;

	return cpy_size;
}

static int play_file(struct audtest_config *config, 
					 unsigned rate, unsigned channels,
					 int fd, size_t count)
{
	int ret = 0;
	next = (char*)malloc(count);
	org_next = next;
	printf(" play_file: count=%d,next=%p\n", count,next);
	if (!next) {
		fprintf(stderr,"could not allocate %d bytes\n", count);
		return -1;
	}
	if (read(fd, next, count) != (ssize_t)count) {
		fprintf(stderr,"could not read %d bytes\n", count);
		return -1;
	}
	avail = count;
	org_avail = avail;
	ret = pcm_play(config, rate, channels, fill_buffer, 0);
	free(org_next);
	next = NULL;
	org_next = NULL;
	return ret;
}

int wav_play(struct audtest_config *config)
{

	struct wav_header hdr;
	//  unsigned rate, channels;
	int fd;

	if (config == NULL) {
		return -1;
	}

	fd = open(config->file_name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "playwav: cannot open '%s'\n", config->file_name);
		return -1;
	}
	if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		fprintf(stderr, "playwav: cannot read header\n");
		return -1;
	}
	fprintf(stderr,"playwav: %d ch, %d hz, %d bit, %s\n",
			hdr.num_channels, hdr.sample_rate, hdr.bits_per_sample,
			hdr.audio_format == FORMAT_PCM ? "PCM" : "unknown");

	if ((hdr.riff_id != ID_RIFF) ||
		(hdr.riff_fmt != ID_WAVE) ||
		(hdr.fmt_id != ID_FMT)) {
		fprintf(stderr, "playwav: '%s' is not a riff/wave file\n", 
				config->file_name);
		return -1;
	}
	if ((hdr.audio_format != FORMAT_PCM) ||
		(hdr.fmt_sz != 16)) {
		fprintf(stderr, "playwav: '%s' is not pcm format\n", config->file_name);
		return -1;
	}
	if (hdr.bits_per_sample != 16) {
		fprintf(stderr, "playwav: '%s' is not 16bit per sample\n", config->file_name);
		return -1;
	}

	return play_file(config, hdr.sample_rate, hdr.num_channels,
					 fd, hdr.data_sz);
}


void* pcmwrite_thread(void *arg)
{
	struct audiot_buf_config *buf_cfg = (struct audiot_buf_config *)arg;
	while(!rec_stop)
	{
		wait_for_frame(buf_cfg);
		if (write(buf_cfg->fd,
			&buf_cfg->buf[buf_cfg->tail * buf_cfg->buffer_size],
			buf_cfg->buffer_size) != (ssize_t)buf_cfg->buffer_size) {
			perror("cannot write buffer");
			pthread_mutex_unlock(&buf_cfg->cmpl_lock);
			return NULL;
		}
		pthread_mutex_lock(&buf_cfg->lock);
		buf_cfg->tail = (buf_cfg->tail + 1) & (FRAME_NUM - 1);
		pthread_mutex_unlock(&buf_cfg->lock);

	}
	pthread_mutex_unlock(&buf_cfg->cmpl_lock);
	return NULL;

}

int wav_rec(struct audtest_config *config)
{

	struct wav_header hdr;
	char *buf;
	struct msm_audio_config cfg;
	struct audiot_buf_config *buf_cfg;
	unsigned sz; //n;
	int fd, afd;
	unsigned total = 0;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *)
						config->private_data;
#ifdef AUDIOV2
	unsigned short enc_id;
	int device_id=-1;
	const char *device = "a2dp_tx";
	int a2dp_set = 0;
#endif

	if ((config->channel_mode != 2) && (config->channel_mode != 1)) {
		perror("invalid channel mode \n");
		return -1;
	} else {
		switch (config->sample_rate) {
		case 48000:    
		case 44100:  
		case 32000:  
		case 24000:  
		case 22050:  
		case 16000: 
		case 12000: 
		case 11025: 
		case 8000:  
			break;
		default:    
			perror("invalid sample rate \n");
			return -1;
			break;
		}
	}

	hdr.riff_id = ID_RIFF;
	hdr.riff_sz = 0;
	hdr.riff_fmt = ID_WAVE;
	hdr.fmt_id = ID_FMT;
	hdr.fmt_sz = 16;
	hdr.audio_format = FORMAT_PCM;
	hdr.num_channels = config->channel_mode;
	hdr.sample_rate = config->sample_rate;
	hdr.byte_rate = hdr.sample_rate * hdr.num_channels * 2;
	hdr.block_align = hdr.num_channels * 2;
	hdr.bits_per_sample = 16;
	hdr.data_id = ID_DATA;
	hdr.data_sz = 0;

	fd = open(config->file_name, O_CREAT | O_RDWR, 0666);
	if (fd < 0) {
		perror("cannot open output file");
		return -1;
	}
	write(fd, &hdr, sizeof(hdr));

#if defined(AUDIOV2) || defined(AUDIO7X27A)
	afd = open(dev_file_name, O_RDONLY);
	if (afd < 0) {
		perror("cannot open msm_pcm_in");
		close(fd);
		return -1;
	}
#else
        afd = open("/dev/msm_pcm_in", O_RDONLY);
        if (afd < 0) {
                perror("cannot open msm_pcm_in");
                close(fd);
                return -1;
        }
#endif

#ifdef AUDIOV2
	if (ioctl(afd, AUDIO_GET_SESSION_ID, &enc_id)) {
		perror("could not get encoder session id\n");
		close(fd);
		close(afd);
		return -1;
	}
	if(!strcmp(dev_file_name, "/dev/msm_a2dp_in"))
		a2dp_set = 1;
	if(!a2dp_set) {
		printf("Routing to non A2DP\n");
		if (devmgr_register_session(enc_id, DIR_TX) < 0) {
			close(fd);
			close(afd);
			return -1;
		}
	} else {
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
	}
	if (rec_source == DUAL_MIC_SOURCE) {
		if (msm_set_dual_mic_config(enc_id, 1) < 0) {
			perror("could not set dual mic config\n");
			if (devmgr_disable_device(device_id, DIR_RX) < 0)
				perror("could not disable device\n");
			return -1;
		}
	}
#endif

	if (ioctl(afd, AUDIO_GET_CONFIG, &cfg)) {
		perror("cannot read audio config");
		goto fail;
	}
	fprintf(stderr,"rec_buf_size = %d\n", audio_data->recsize);
	cfg.buffer_size = audio_data->recsize;
	cfg.channel_count = hdr.num_channels;
	cfg.sample_rate = hdr.sample_rate;

	sz = cfg.buffer_size;
	if (sz > (cfg.buffer_size * FRAME_NUM)) {
		fprintf(stderr, "buffer size %d too large\n", sz);
		goto fail;
	}
	if (sz < 160) {
		fprintf(stderr, "buffer size %d is small\n", sz);
		goto fail;
	}

	if (ioctl(afd, AUDIO_SET_CONFIG, &cfg)) {
		perror("cannot write audio config");
		goto fail;
	}
	buf_cfg = (struct audiot_buf_config *)malloc(sizeof(struct audiot_buf_config));
	if(buf_cfg == NULL)	{
		fprintf(stderr,"\n buffer allocation failed \n");
		goto fail;
	}

	memset(buf_cfg,0,sizeof(struct audiot_buf_config));

	buf_cfg->buf =(char *)malloc(cfg.buffer_size * FRAME_NUM);
	if(buf_cfg->buf == NULL) {

		fprintf(stderr,"\n buffer allocation failed \n");
		goto fail;
	}
	rec_stop = 0;
	buf_cfg->buffer_size = sz;
	buf_cfg->fd = fd;
	buf = buf_cfg->buf;
	pthread_cond_init(&buf_cfg->cond, 0);
	pthread_mutex_init(&buf_cfg->cond_lock, 0);
	pthread_mutex_init(&buf_cfg->cmpl_lock, 0);
	pthread_mutex_init(&buf_cfg->lock, 0);
	pthread_create( &buf_cfg->thread, NULL,
			pcmwrite_thread, (void*) buf_cfg);

#ifdef AUDIOV2
	/* Record form voice link */
	if (rec_source <= VOC_REC_BOTH ) {

		if (ioctl(afd, AUDIO_SET_INCALL, &rec_source)) {
			perror("Error: AUDIO_SET_INCALL failed");
			goto fail;
		}
		printf("rec source = 0x%8x\n", rec_source);
	}
#endif

	if (ioctl(afd, AUDIO_START, 0) < 0) {
		perror("cannot start audio");
		goto fail;
	}
	fprintf(stderr,"\n*** RECORDING IN PROGRESS ***\n");
	while(!rec_stop) {
		if (read(afd, &buf[buf_cfg->head * sz], sz) != (ssize_t)sz) {
			perror("cannot read buffer");
			rec_stop = 1;
			frame_available(buf_cfg);
			goto done;
		}

		pthread_mutex_lock(&buf_cfg->lock);
		buf_cfg->head = (buf_cfg->head + 1) & (FRAME_NUM - 1);

		/* If overflow, move the tail index foward. */
		if (buf_cfg->head == buf_cfg->tail)
		{
			buf_cfg->tail = (buf_cfg->tail + 1) & (FRAME_NUM - 1);
			fprintf(stderr,"\n***BUFFER OVERFLOW***");
			rec_stop = 1;
			pthread_mutex_unlock(&buf_cfg->lock);

			frame_available(buf_cfg);
			goto done;
		}
		pthread_mutex_unlock(&buf_cfg->lock);

		frame_available(buf_cfg);
		total += sz;
	}
	done:
	close(afd);
	/* wait for pcm writer thread to exit */
	pthread_mutex_lock(&buf_cfg->cmpl_lock);

	/* update lengths in header */
	hdr.data_sz = total;
	hdr.riff_sz = total + 8 + 16 + 8;
	lseek(fd, 0, SEEK_SET);
	write(fd, &hdr, sizeof(hdr));
	close(fd);
	pthread_cond_destroy(&buf_cfg->cond);
	pthread_mutex_destroy(&buf_cfg->cond_lock);
	pthread_mutex_destroy(&buf_cfg->lock);
	pthread_mutex_destroy(&buf_cfg->cmpl_lock);
#ifdef AUDIOV2
	if(!a2dp_set) {
		printf("Derouting from non A2DP\n");
		if (devmgr_unregister_session(enc_id, DIR_TX) < 0){
			perror("could not unregister encode session\n");
		}
	} else {
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
	}
#endif
	return 0;

	fail:
	close(afd);
	close(fd);
#ifdef AUDIOV2
	if(!a2dp_set) {
		printf("Derouting from non A2DP\n");
		if (devmgr_unregister_session(enc_id, DIR_TX) < 0){
			perror("could not unregister encode session\n");
		}
	} else {
		device_id = msm_get_device(device);
		printf("De Routing to A2DP\n");
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
	}
#endif
	unlink(config->file_name);
	return -1;
}

void* playpcm_thread(void* arg) {
	struct audiotest_thread_context *context = 
	(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = wav_play(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int pcmplay_read_params(void) {
	struct audiotest_thread_context *context; 
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/data.wav";
		dev_file_name = "/dev/msm_pcm_out";
		repeat = 0;
		quit = 0;

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
				if (repeat == 0)
					repeat = -1;
				else
					repeat--;
			} else {
				context->config.file_name = token;
			} 
			token = strtok(NULL, " ");
		}
		context->type = AUDIOTEST_TEST_MOD_PCM_DEC;
		pthread_create( &context->thread, NULL, 
						playpcm_thread, (void*) context);
	}

	return ret_val;

}

void* recpcm_thread(void* arg) {
	struct audiotest_thread_context *context = 
	(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = wav_rec(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int pcmrec_read_params(void) {
	struct audiotest_thread_context *context;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.sample_rate = 8000;
		context->config.file_name = "/data/record.wav";
		dev_file_name = "/dev/msm_pcm_in";
		context->config.channel_mode = 1;  
		context->type = AUDIOTEST_TEST_MOD_PCM_ENC;
		rec_source = 3;
		struct audio_pvt_data *audio_data;
		audio_data = (struct audio_pvt_data *)
				malloc(sizeof(struct audio_pvt_data));

		if(!audio_data) {
			printf("error allocating audio instance structure \n");
			free_context(context);
			ret_val = -1;
		} else {

			audio_data->recsize = 2048;
			token = strtok(NULL, " ");
			while (token != NULL) {
				printf("%s \n", token);
				if (!memcmp(token,"-rate=", (sizeof("-rate=") - 1))) {
					context->config.sample_rate =
						atoi(&token[sizeof("-rate=") - 1]);
				} else if (!memcmp(token,"-cmode=", (sizeof("-cmode=") - 1))) {
					context->config.channel_mode =
						atoi(&token[sizeof("-cmode=") - 1]);
				} else if (!memcmp(token,"-recbufsize=", (sizeof("-recbufsize=") - 1))) {
					audio_data->recsize = atoi(&token[sizeof("-recbufsize=") - 1]);
				} else if (!memcmp(token,"-id=", (sizeof("-id=") - 1))) {
					context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
				} else if (!memcmp(token, "-dev=",
								   (sizeof("-dev=") - 1))) {
					dev_file_name = token + (sizeof("-dev=")-1);
				} else if (!memcmp(token, "-src=", (sizeof("-src=") - 1))) {
					rec_source = atoi(&token[sizeof("-src=") - 1]);
				} else {
					context->config.file_name = token;
				}
				token = strtok(NULL, " ");
			}
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			printf("%s:sample_rate=%d channel_mode=%d, recbufsize = %d\n", __FUNCTION__,
				   context->config.sample_rate, context->config.channel_mode,
				  audio_data->recsize);
		pthread_create( &context->thread, NULL, recpcm_thread, (void*) context);  
	}
	}

	return ret_val;
}

int pcm_play_control_handler(void* private_data) {
	int  drvfd , ret_val = 0;
	int volume;
	char *token;
#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
	int eq_preset;
#endif

	token = strtok(NULL, " ");
	if ((private_data != NULL) && 
		(token != NULL)) {
		drvfd = (int) private_data;
		if(!memcmp(token,"-cmd=", (sizeof("-cmd=") -1))) {
                       token = &token[sizeof("-cmd=") - 1];
                       printf("%s: cmd %s\n", __FUNCTION__, token);
#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
                       if (!strcmp(token, "volume")) {
                               int rc;
                               unsigned short dec_id;
                               token = strtok(NULL, " ");
                               if (!memcmp(token, "-value=",
                                       (sizeof("-value=") - 1))) {
                                       volume = atoi(&token[sizeof("-value=") - 1]);
                                       if (ioctl(drvfd, AUDIO_GET_SESSION_ID, &dec_id)) {
                                               perror("could not get decoder session id\n");
                                       } else {
                                               printf("session %d - volume %d \n", dec_id, volume);
                                               if ((volume >= 0) && (volume <= 100)) {
                                                  rc = msm_set_volume(dec_id, volume);
                                                  printf("session volume result %d\n", rc);
                                               } else
                                                  printf("session volume out of range\n");
                                       }
                               }
                       } else if (!strcmp(token, "eq")) {
                               token = strtok(NULL, " ");
                               if (!memcmp(token, "-preset=",
                                       (sizeof("-preset=") - 1))) {
                                        eq_preset = atoi(&token[sizeof("-preset=") - 1]);
					if ((eq_preset >= 0) && (eq_preset < MAX_PRESETS))
						#ifdef QDSP6V2
                                		set_pcm_default_eq_values(drvfd, eq_preset);
						#else
						printf("not supported\n");
						#endif
					else {
                                                printf("Wrong preset:%d Check command: \
                                                for supported preset values\n", eq_preset);
                                                printf("mm-audio-native-test -format playpcm\n");
                                        }
                               }
			} else if (!strcmp(token, "stop")) {
			       quit = 1;
			       printf("quit session\n");
		       }
#else
			if (!strcmp(token, "pause")) {
				ioctl(drvfd, AUDIO_PAUSE, 1);
			} else if (!strcmp(token, "resume")) {
				ioctl(drvfd, AUDIO_PAUSE, 0);
			} else if (!strcmp(token, "volume")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-value=",
					(sizeof("-value=") - 1))) {
					volume =
					atoi(&token[sizeof("-value=") - 1]);
					ioctl(drvfd, AUDIO_SET_VOLUME, volume);
					printf("volume:%d\n", volume);
				}
			} else if (!strcmp(token, "stop")) {
			       quit = 1;
			       printf("quit session\n");
		       }
#endif
		}
	} else {
		ret_val = -1;
	}

	return ret_val;
}

int pcm_rec_control_handler(void* private_data) {
	int /* drvfd ,*/ ret_val = 0;
	char *token;

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

const char *pcmplay_help_txt = 
	"Play PCM file: type \n\
echo \"playpcm path_of_file -id=xxx -repeat=x -dev=/dev/msm_pcm_dec or msm_pcm_out\" > tmp/audio_test \n\
Repeat 'x' no. of times, repeat infinitely if repeat = 0\n\
Sample rate of PCM file <= 48000 \n\
Bits per sample = 16 bits \n\
Supported control command: pause, flush, volume, stop, equalizer\n\
examples: \n\
echo \"control_cmd -id=xxx -cmd=eq -preset=yyyy\" > %s \n\
                                   0 - BLANK \n\
                                   1 - CLUB \n\
                                   2 - DANCE \n\
                                   3 - FULLBASS \n\
                                   4 - FULLBASSTREBLE \n\
                                   5 - FULLTREBLE \n\
                                   6 - LAPTOP \n\
                                   7 - LARGEHALL \n\
                                   8 - LIVE \n\
                                   9 - PARTY \n\
                                  10 - POP \n\
                                  11 - REGGAE \n\
                                  12 - ROCK \n\
                                  13 - SKA \n\
                                  14 - SOFT \n\
                                  15 - SOFTROCK \n\
                                  16 - TECHNO \n";


void pcmplay_help_menu(void) {
	printf("%s\n", pcmplay_help_txt);
}

const char *pcmrec_help_txt = 
"Record pcm file: type \n\
echo \"recpcm path_of_file -rate=xxx -cmode=x -recbufsize=x -id=xxx -src=x -dev=/dev/msm_pcm_in or msm_a2dp_in\" > tmp/audio_test \n\
sample rate: 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000 \n\
src: 0 - Uplink 1 - Downlink, 2 - UL/DL, 3 - Mic 4 - Dual MIC\n\
channel mode: 1 or 2 \n\
recbufsize(recording buffer size): value greater than 160 and a multiple of 4 for 8k, default is 2048 \n\
                                   (512,1024 or 2048) * channel mode for 7x30, default is 2048 \n\
Supported control command: stop\n ";

void pcmrec_help_menu(void) {
	printf("%s\n", pcmrec_help_txt);
}

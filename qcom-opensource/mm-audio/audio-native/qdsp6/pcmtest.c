/* pcmtest.c - native PCM test application
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
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include <linux/ioctl.h>
#include <linux/msm_audio.h>

#include "audiotest_def.h"
#include "equalizer.h"

static int pcm_play(struct audtest_config *clnt_config, unsigned rate, unsigned channels,
                    int (*fill)(void *buf, unsigned sz, void *cookie),
                    void *cookie)
{
	struct msm_audio_config config;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
	int sz;
	char * buf;
	int afd;
	int cntW=0;
	int volume = 100;

	afd = open("/dev/msm_pcm_out", O_RDWR);
	if (afd < 0) {
		perror("pcm_play: cannot open audio device");
		return -1;
	}

	if (ioctl(afd, AUDIO_GET_CONFIG, &config)) {
		perror("could not get config");
		return -1;
	}

//	config.buffer_size = g_play_buf_size;
	config.channel_count = channels;
	config.sample_rate = rate;
	if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
		perror("could not set config");
		close(afd);
		return -1;
	}

	buf = (char*) malloc(sizeof(char) * config.buffer_size);
	if (buf == NULL) {
		perror("fail to allocate buffer\n");
		close(afd);
		return -1;
        }

	sz = config.buffer_size;
	printf("pcm_play: sz=%d, buffer_count=%d\n",sz,config.buffer_count);

	ioctl(afd, AUDIO_START, 0);

	for (;;) {
#if 0
		if (ioctl(afd, AUDIO_GET_STATS, &stats) == 0)
			fprintf(stderr,"%10d\n", stats.out_bytes);
#endif
		if ((sz = fill(buf, config.buffer_size, cookie)) < 0) {
			printf(" fill return NON NULL, exit loop \n");
			//fsync(afd);
			break;
		}

		if (g_terminate_early) {
			printf("cancelling...\n");
			break;
		}

		if( audio_data->g_test_volume ) {
			printf("volume = %d\n",volume);
			if (ioctl(afd, AUDIO_SET_VOLUME, &volume)) {
				perror("could not set volume");
			}
			volume -= 1;
			if( volume <= 1 ) {
				// Loop around to max stream volume so volume change is audible.
				volume = 100;
			}
		}
		if( audio_data->g_test_equalizer ) {
			if (cntW == 150) {
				printf("********** club ***********\n");
				set_pcm_default_eq_values(afd, BAND_CLUB, 0);
			}
			else if (cntW == 350)  {
				printf("********** dance ***********\n");
				set_pcm_default_eq_values(afd, BAND_DANCE, 1);
			}
			else if (cntW == 575) {
				printf("********** techno ***********\n");
				set_pcm_default_eq_values(afd, BAND_TECHNO, 2);
			}
			else if (cntW == 700) {
				printf("********** live ***********\n");
				set_pcm_default_eq_values(afd, BAND_LIVE, 3);
			}
			else if (cntW == 825) {
				printf("********** reggae ***********\n");
				set_pcm_default_eq_values(afd, BAND_REGGAE, 4);
			}
		}

		if (write(afd, buf, sz) != sz) {
			printf(" write return not equal to sz, exit loop\n");
			break;
		} else {
			cntW++;
			printf(" pcm_play: cntW=%d\n",cntW);
		}

	}
	printf("end of play\n");
	/* let audio finish playing before close */
        sleep(3);
	close(afd);
	return 0;
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
	uint32_t byte_rate;       /* sample_rate * num_channels * bps / 8 */
	uint16_t block_align;     /* num_channels * bps / 8 */
	uint16_t bits_per_sample;
	uint32_t data_id;
	uint32_t data_sz;
};

static int rec_stop;
static char *next;
static unsigned avail;

static int fill_buffer(void *buf, unsigned sz, void *cookie)
{
	unsigned cpy_size = (sz < avail?sz:avail);

	if (avail == 0)
		return -1;

	memcpy(buf, next, cpy_size);
	next += cpy_size;
	avail -= cpy_size;

	return cpy_size;
}

static void play_file(struct audtest_config *config, unsigned rate, unsigned channels,
                      int fd, unsigned count)
{
	next = (char*)malloc(count);
	printf(" play_file: count=%d,next=%p\n", count,next);
	if (!next) {
		fprintf(stderr,"could not allocate %d bytes\n", count);
		return;
	}
	if (read(fd, next, count) != count) {
		fprintf(stderr,"could not read %d bytes\n", count);
		return;
	}
	avail = count;
	pcm_play(config, rate, channels, fill_buffer, 0);
}

int wav_play(struct audtest_config *config)
{

	struct wav_header hdr;
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
		fprintf(stderr, "playwav: '%s' is not a riff/wave file\n", config->file_name);
		return -1;
	}
	if ((hdr.audio_format != FORMAT_PCM) /*||
			(hdr.fmt_sz != 16)*/) {
		fprintf(stderr, "playwav: '%s' is not pcm format\n", config->file_name);
		return -1;
	}
	if (hdr.bits_per_sample != 16) {
		fprintf(stderr, "playwav: '%s' is not 16bit per sample\n", config->file_name);
		return -1;
	}

	play_file(config, hdr.sample_rate, hdr.num_channels,
	fd, hdr.data_sz);

	return 0;
}

int wav_rec(struct audtest_config *config)
{

	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config->private_data;
	struct wav_header hdr;
	unsigned char buf[10240];
	struct msm_audio_config cfg;
	struct msm_voicerec_mode voicerec_mode;
	unsigned sz;
	int fd, afd;
	unsigned total = 0;
	int read_size = 0;

	hdr.riff_id = ID_RIFF;
	hdr.riff_sz = 0;
	hdr.riff_fmt = ID_WAVE;
	hdr.fmt_id = ID_FMT;
	hdr.fmt_sz = 16;
	hdr.audio_format = FORMAT_PCM;
	hdr.num_channels = 1;
	hdr.sample_rate = config->sample_rate;
	hdr.byte_rate = hdr.sample_rate * hdr.num_channels * 2;
	hdr.block_align = hdr.num_channels * 2;
	hdr.bits_per_sample = 16;
	hdr.data_id = ID_DATA;
	hdr.data_sz = 0;
	voicerec_mode.rec_mode = config->rec_mode;

	fd = open(config->file_name, O_CREAT | O_RDWR, 0666);
	if (fd < 0) {
		perror("cannot open output file");
		return -1;
	}
	write(fd, &hdr, sizeof(hdr));

	afd = open("/dev/msm_pcm_in", O_RDWR);
	if (afd < 0) {
		perror("cannot open msm_pcm_in");
		close(fd);
		return -1;
	}

	if (ioctl(afd, AUDIO_GET_CONFIG, &cfg)) {
		perror("cannot read audio config");
		goto fail;
	}

	cfg.buffer_size = audio_data->g_rec_buf_size;

	sz = cfg.buffer_size;
	fprintf(stderr,"buffer size %d\n", sz);
	if (sz > sizeof(buf)) {
		fprintf(stderr,"buffer size %d too large\n", sz);
		goto fail;
	}
	else if (sz <= 0) {
		fprintf(stderr,"buffer size %d too small\n", sz);
		goto fail;
	}


	cfg.channel_count = hdr.num_channels;
	cfg.sample_rate = hdr.sample_rate;
	if (ioctl(afd, AUDIO_SET_CONFIG, &cfg)) {
		perror("cannot write audio config");
		goto fail;
	}

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
	while (rec_stop!=1){
		read_size = read(afd, buf, sz);
		if (write(fd, buf, read_size) != read_size) {
			perror("cannot write buffer");
			goto fail;
		}
		total += read_size;
		if (g_terminate_early) {
			printf("done...\n");
			break;
		}

	}
	close(afd);

	/* update lengths in header */
	hdr.data_sz = total;
	hdr.riff_sz = total + 8 + 16 + 8;
	lseek(fd, 0, SEEK_SET);
	write(fd, &hdr, sizeof(hdr));
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
	struct itimerspec ts;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/test.wav";
		g_terminate_early = 0;
		struct audio_pvt_data *audio_data;
		audio_data = (struct audio_pvt_data *) malloc(sizeof(struct audio_pvt_data));
		if(!audio_data) {
			printf("error allocating audio instance structure \n");
			free_context(context);
			ret_val = -1;
		} else {
			audio_data->g_play_buf_size = 4096;
			token = strtok(NULL, " ");
			while (token != NULL) {
				if (!memcmp(token,"-id=", (sizeof("-id=")-1))) {
					context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
				} else if (!memcmp(token,"-eq=", (sizeof("-eq=") - 1))) {
					audio_data->g_test_equalizer = atoi(&token[sizeof("-eq=") - 1]);
				} else if (!memcmp(token,"-volume=", (sizeof("-volume=") - 1))) {
					audio_data->g_test_volume = atoi(&token[sizeof("-volume=") - 1]);
				} else if (!memcmp(token,"-playbufsize=", (sizeof("-playbufsize=") - 1))) {
					audio_data->g_play_buf_size = atoi(&token[sizeof("-playbufsize=") - 1]);
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
			context->type = AUDIOTEST_TEST_MOD_PCM_DEC;
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL,
					playpcm_thread, (void*) context);
		}
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
	struct itimerspec ts;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.sample_rate = 8000;
		context->config.file_name = "/data/rec.wav";
		context->config.rec_mode = 1;
		context->type = AUDIOTEST_TEST_MOD_PCM_ENC;
		g_terminate_early = 0;
		struct audio_pvt_data *audio_data;
		audio_data = (struct audio_pvt_data *) malloc(sizeof(struct audio_pvt_data));
		if(!audio_data) {
			printf("error allocating audio instance structure \n");
			free_context(context);
			ret_val = -1;
		} else {
			audio_data->g_rec_buf_size = 4096;
			token = strtok(NULL, " ");
			while (token != NULL) {
				printf("%s \n", token);
				if (!memcmp(token,"-rate=", (sizeof("-rate=") - 1))) {
					context->config.sample_rate =
					atoi(&token[sizeof("-rate=") - 1]);
				} else if (!memcmp(token,"-rec_mode=", (sizeof("-rec_mode=") - 1))) {
					context->config.rec_mode =
					atoi(&token[sizeof("-rec_mode=") - 1]);
				} else if (!memcmp(token,"-recbufsize=", (sizeof("-recbufsize=") - 1))) {
					audio_data->g_rec_buf_size = atoi(&token[sizeof("-recbufsize=") - 1]);
				} else if (!memcmp(token,"-id=", (sizeof("-id=") - 1))) {
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
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			printf("%s : sample_rate=%d rec_mode=%d\n",
				__FUNCTION__, context->config.sample_rate,
				context->config.rec_mode);
			pthread_create( &context->thread, NULL, recpcm_thread, (void*) context);
		}
	}

	return ret_val;
}

int pcm_play_control_handler(void* private_data) {
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
			if (!strcmp(token, "stop"))
				printf("Rec stop command\n");
				rec_stop = 1;
		}
	} else {
		ret_val = -1;
	}

	return ret_val;
}

const char *pcmplay_help_txt =
"Play PCM file: type \n\
echo \"playpcm path_of_file -id=xxx -eq=x -timeout=x -playbufsize=xxx -volume=x\" > /data/audio_test \n\
sample rate: 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000 \n\
Equalizer (-eq) = 1 (enable) \n\
Volume = 0 (default, volume=100), 1 (test different volumes continuously) \n\
timeout = x (value in seconds) \n\
playbufsize = xxx (play buffer size value) \n\
Bits per sample = 16 bits \n ";

void pcmplay_help_menu(void) {
	printf("%s\n", pcmplay_help_txt);
}

const char *pcmrec_help_txt =
"Record pcm file: type \n\
echo \"recpcm path_of_file -rate=xxx -rec_mode=x -id=xxx -timeout=x -recbufsize=xxx\" > tmp/audio_test \n\
sample rate: 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000 \n\
recbufsize = xxx (record buffer size value) \n\
timeout = x (value in seconds) \n\
record mode: 0=txonly 1=rxonly 2=mixed)\n ";

void pcmrec_help_menu(void) {
	printf("%s\n", pcmrec_help_txt);
}

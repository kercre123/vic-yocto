/* aactest.c - native AAC test application
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
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <linux/ioctl.h>
#include <linux/msm_audio.h>
#include <linux/msm_audio_aac.h>
#include "audiotest_def.h"

typedef unsigned char uint8;
typedef unsigned char byte;
typedef unsigned int  uint32;
typedef unsigned int  uint16;

#define AUDAAC_MAX_ADIF_HEADER_LENGTH 64
/* ADTS variable frame header, frame length field  */
#define AUDAAC_ADTS_FRAME_LENGTH_SIZE    13

/* maximum ADTS frame header length                */
#define AUDAAC_MAX_ADTS_HEADER_LENGTH 7


#define AACHDR_LAYER_SIZE             2
#define AACHDR_CRC_SIZE               1
#define AAC_PROFILE_SIZE              2
#define AAC_SAMPLING_FREQ_INDEX_SIZE  4
#define AAC_ORIGINAL_COPY_SIZE        1
#define AAC_HOME_SIZE                 1

#define MIN(A,B)	(((A) < (B))?(A):(B))

uint8   audaac_header[AUDAAC_MAX_ADTS_HEADER_LENGTH];
unsigned int audaac_hdr_bit_index;

void audaac_rec_install_bits
(
  uint8   *input,
  byte    num_bits_reqd,
  uint32  value,
  uint16  *hdr_bit_index
)
{
  uint32 byte_index;
  byte   bit_index;
  byte   bits_avail_in_byte;
  byte   num_to_copy;
  byte   byte_to_copy;

  byte   num_remaining = num_bits_reqd;
  uint8  bit_mask;

  bit_mask = 0xFF;

  while (num_remaining) {

    byte_index = (*hdr_bit_index) >> 3;
    bit_index  = (*hdr_bit_index) &  0x07;

    bits_avail_in_byte = 8 - bit_index;

    num_to_copy = MIN(bits_avail_in_byte, num_remaining);

    byte_to_copy = ((uint8)((value >> (num_remaining - num_to_copy)) & 0xFF) <<
                    (bits_avail_in_byte - num_to_copy));

    input[byte_index] &= ((uint8)(bit_mask << bits_avail_in_byte));
    input[byte_index] |= byte_to_copy;

    *hdr_bit_index += num_to_copy;

    num_remaining -= num_to_copy;
  } /* while (num_remaining) */
} /* audaac_rec_install_bits */


void audaac_rec_install_adts_header_variable (uint16  byte_num)
{
  //uint16  bit_index=0;

  uint32  value;

  uint32   sample_index = 44100;
  uint8   channel_config = 1;

  /* Store Sync word first */
  audaac_header[0] = 0xFF;
  audaac_header[1] = 0xF0;

  audaac_hdr_bit_index = 12;

  /* ID field, 1 bit */
  value = 1;
  audaac_rec_install_bits(audaac_header,
                          1,
                          value,
                          &(audaac_hdr_bit_index));

  /* Layer field, 2 bits */
  value = 0;
  audaac_rec_install_bits(audaac_header,
                          AACHDR_LAYER_SIZE,
                          value,
                          &(audaac_hdr_bit_index));

  /* Protection_absent field, 1 bit */
  value = 1;
  audaac_rec_install_bits(audaac_header,
                          AACHDR_CRC_SIZE,
                          value,
                          &(audaac_hdr_bit_index));

  /* profile_ObjectType field, 2 bit */
  value = 1;
  audaac_rec_install_bits(audaac_header,
                          AAC_PROFILE_SIZE,
                          value,
                          &(audaac_hdr_bit_index));

  /* sampling_frequency_index field, 4 bits */
  audaac_rec_install_bits(audaac_header,
                          AAC_SAMPLING_FREQ_INDEX_SIZE,
                          (uint32)sample_index,
                          &(audaac_hdr_bit_index));

  /* pravate_bit field, 1 bits */
  audaac_rec_install_bits(audaac_header,
                          1,
                          0,
                          &(audaac_hdr_bit_index));

  /* channel_configuration field, 3 bits */
  audaac_rec_install_bits(audaac_header,
                          3,
                          channel_config,
                          &(audaac_hdr_bit_index));


  /* original/copy field, 1 bits */
  audaac_rec_install_bits(audaac_header,
                          AAC_ORIGINAL_COPY_SIZE,
                          0,
                          &(audaac_hdr_bit_index));


  /* home field, 1 bits */
  audaac_rec_install_bits(audaac_header,
                          AAC_HOME_SIZE,
                          0,
                          &(audaac_hdr_bit_index));

 // bit_index = audaac_hdr_bit_index;
 // bit_index += 2;

	/* copyr. id. bit, 1 bits */
  audaac_rec_install_bits(audaac_header,
                          1,
                          0,
                          &(audaac_hdr_bit_index));

	/* copyr. id. start, 1 bits */
  audaac_rec_install_bits(audaac_header,
                          1,
                          0,
                          &(audaac_hdr_bit_index));

  /* aac_frame_length field, 13 bits */
  audaac_rec_install_bits(audaac_header,
                          AUDAAC_ADTS_FRAME_LENGTH_SIZE,
                          byte_num,
                          &audaac_hdr_bit_index);

  /* adts_buffer_fullness field, 11 bits */
  audaac_rec_install_bits(audaac_header,
                          11,
                          0x7FF,
                          &audaac_hdr_bit_index);

  /* number_of_raw_data_blocks_in_frame, 2 bits */
  audaac_rec_install_bits(audaac_header,
                          2,
                          0,
                          &audaac_hdr_bit_index);

} /* audaac_rec_install_adts_header_variable */


static int do_aac_play(struct audtest_config *clnt_config, unsigned rate, unsigned channels,
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
	int volume = 100;

	afd = open("/dev/msm_aac", O_RDWR);
	if (afd < 0) {
		perror("aac_play: cannot open audio device");
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
	sz = config.buffer_size;
	printf("aac_play: sz=%d, buffer_count=%d\n",sz,config.buffer_count);

	if (sz > sizeof(buf)) {
		fprintf(stderr,"too big\n");
		return -1;
	}

	fprintf(stderr,"start\n");
	ioctl(afd, AUDIO_START, 0);

	for (;;) {
		int cnt = 0;
		if (fill(buf, sz, cookie) < 0) {
			printf(" fill return NON NULL, exit loop \n");
			if(fsync(afd) < 0)
                                printf("fsync failed\n");
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
				/* Loop around to max stream volume so volume change is audible. */
				volume = 100;
			}
		}

		cnt = write(afd, buf, sz);
                printf("write %d bytes, %d bytes reported\n",sz,cnt);
		cntW++;
		printf(" aac_play: cntW=%d\n",cntW);
	}
	printf("end of play\n");
	/* let audio finish playing before close */
        sleep(3);
	close(afd);
	return 0;
}

/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */

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

static void play_aac_file(struct audtest_config *config, unsigned rate, unsigned channels,
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
	do_aac_play(config, rate, channels, fill_buffer, 0);
}

int aac_play(struct audtest_config *config)
{
	struct stat stat_buf;
	int fd;

	if (config == NULL) {
		return -1;
	}

	fd = open(config->file_name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "playaac: cannot open '%s'\n", config->file_name);
		return -1;
	}

	(void)fstat(fd, &stat_buf);

	play_aac_file(config, 44100, 2, fd, stat_buf.st_size);

	return 0;
}

int aac_rec(struct audtest_config *config)
{
  //unsigned char buf[8192];
  unsigned char buf[2048*10];
  struct msm_audio_stream_config str_cfg;
  struct msm_audio_aac_enc_config aac_cfg;
  struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config->private_data;
  unsigned sz ,framesize = 0; //n;
  int fd, afd;
  unsigned total = 0;
  unsigned char tmp;
  static unsigned int cnt = 0;

  fd = open(config->file_name, O_CREAT | O_RDWR, 0666);
  if (fd < 0) {
    perror("cannot open output file");
    return -1;
  }
  afd = open("/dev/msm_aac_in", O_RDWR);
  if (afd < 0) {
    perror("cannot open msm_aac_in");
    close(fd);
    return -1;
  }

  if (ioctl(afd, AUDIO_GET_STREAM_CONFIG, &str_cfg)) {
    perror("cannot read audio stream config");
    goto fail;
  }

  sz = 1543;
  fprintf(stderr,"buffer size %d\n", sz);
  if (sz > sizeof(buf)) {
    fprintf(stderr,"buffer size %d too large\n", sz);
    goto fail;
  }

  str_cfg.buffer_size = sz;
  str_cfg.buffer_count = 2;
  if (ioctl(afd, AUDIO_SET_STREAM_CONFIG, &str_cfg)) {
    perror("cannot write audio stream config");
    goto fail;
  }

  if (ioctl(afd, AUDIO_GET_AAC_ENC_CONFIG, &aac_cfg)) {
    perror("cannot read AAC config");
    goto fail;
  }

  aac_cfg.channels = 1;
  aac_cfg.bit_rate = 192000;
  aac_cfg.stream_format = AUDIO_AAC_FORMAT_ADTS;
  aac_cfg.sample_rate = 48000;

  if (ioctl(afd, AUDIO_SET_AAC_ENC_CONFIG, &aac_cfg)) {
    perror("cannot set AAC config");
    goto fail;
  }


  if (ioctl(afd, AUDIO_START, 0)) {
    perror("cannot start audio");
    goto fail;
  }

  fcntl(0, F_SETFL, O_NONBLOCK);
  fprintf(stderr,"\n*** RECORDING * USE 'STOP' CONTROL COMMAND TO STOP***\n");

  rec_stop = 0;

  for (;(rec_stop!=1);) {
    if (g_terminate_early || (rec_stop==1)) {
        printf("cancelling...\n");
        break;
    }

    framesize = read(afd, buf, sz);

    if(write(fd, buf, framesize) != framesize) {
      perror("cannot write buffer");
      goto fail;
    }
    total += framesize;
  }
  done:
  printf("end of recording\n");
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
	printf("Alarm handler\n");
	g_terminate_early = 1;
	sleep(1);
}

void* playaac_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = aac_play(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int aacplay_read_params(void) {
	struct audiotest_thread_context *context;
	struct itimerspec ts;
	char *token;
	int ret_val = 0;

	printf("This option is not supported currently\n");
	return -1;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/test.aac";
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
			context->type = AUDIOTEST_TEST_MOD_AAC_DEC;
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL,
					playaac_thread, (void*) context);
		}
	}

	return ret_val;
}

void* recaac_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = aac_rec(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int aacrec_read_params(void) {
	struct audiotest_thread_context *context;
	struct itimerspec ts;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/rec.aac";
		context->type = AUDIOTEST_TEST_MOD_AAC_ENC;
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
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL, recaac_thread, (void*) context);
		}
	}
	return ret_val;
}

int aac_play_control_handler(void* private_data) {
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

int aac_rec_control_handler(void* private_data) {
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

const char *aacplay_help_txt =
"Play AAC file: type \n\
echo \"playaac path_of_file -id=xxx timeout=x -volume=x\" > /data/audio_test \n\
timeout = x (value in seconds) \n\
Volume = 0 (default, volume=100), 1 (test different volumes continuously) \n\
Bits per sample = 16 bits \n ";

void aacplay_help_menu(void) {
	printf("%s\n", aacplay_help_txt);
}

const char *aacrec_help_txt =
"Record aac file: type \n\
echo \"recaac path_of_file -id=xxx -timeout=x\" > tmp/audio_test \n\
sample rate: 48000 constant\n\
timeout = x (value in seconds) \n\
channel mode: 1 constant \n ";

void aacrec_help_menu(void) {
	printf("%s\n", aacrec_help_txt);
}

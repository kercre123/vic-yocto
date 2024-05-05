/* mp3test.c - native MP3 test application
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
#include "audiotest_def.h"

static int initiate_play(struct audtest_config *clnt_config, unsigned rate, unsigned channels,
                         int (*fill)(void *buf, unsigned sz, void *cookie),
                         void *cookie)
{
  struct msm_audio_config config;
  struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
  // struct msm_audio_stats stats;
  unsigned sz;
  char buf[32768];
  int afd;
  int cntW=0;
  int volume = 100;

  afd = open("/dev/msm_mp3", O_RDWR);
  if (afd < 0) {
    perror("mp3_play: cannot open MP3 device");
    return -1;
  }

  if (ioctl(afd, AUDIO_GET_CONFIG, &config)) {
    perror("could not get config");
    return -1;
  }

  config.channel_count = channels;
  config.sample_rate = rate;
  if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
    perror("could not set config");
    return -1;
  }
  sz = config.buffer_size;
  printf("initiate_play: sz=%d, buffer_count=%d\n",sz,config.buffer_count);

  if (sz > sizeof(buf)) {
    fprintf(stderr,"too big\n");
    return -1;
  }

  fprintf(stderr,"start\n");
  ioctl(afd, AUDIO_START, 0);

  for (;;) {
	if (g_terminate_early) {
	    printf("cancelling...\n");
	    break;
	}

#if 0
    if (ioctl(afd, AUDIO_GET_STATS, &stats) == 0)
      fprintf(stderr,"%10d\n", stats.out_bytes);
#endif
    if (fill(buf, sz, cookie) < 0) {
	printf(" fill return NON NULL, exit loop \n");
	if(fsync(afd) < 0)
			printf("fsync failed\n");
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
    if (write(afd, buf, sz) != sz) {
      printf(" write return not equal to sz, exit loop\n");
      break;
    } else {
      cntW++;
      printf(" mp3_play: cntW=%d\n",cntW);
    }

  }
  printf("end of play\n");
//done:
  /* let audio finish playing before close */
  sleep(3);
  close(afd);
  return 0;
}

/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */

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

static void play_file(struct audtest_config *config, unsigned rate, unsigned channels,
                      int fd, unsigned count)
{

  next = (char*)malloc(count);
  printf(" play_file: count=%d,next=%s\n", count,next);
  if (!next) {
    fprintf(stderr,"could not allocate %d bytes\n", count);
    return;
  }

  if (read(fd, next, count) != count) {
    fprintf(stderr,"could not read %d bytes\n", count);
    return;
  }
  avail = count;
  initiate_play(config, rate, channels, fill_buffer, 0);
}

int mp3_play(struct audtest_config *config)
{
  struct stat stat_buf;
  int fd;

  fd = open(config->file_name, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "playmp3: cannot open '%s'\n", config->file_name);
    return -1;
  }

  (void) fstat(fd, &stat_buf);


  play_file(config, 44100, 2, fd, stat_buf.st_size);

  return 0;
}

static void audiotest_alarm_handler(int sig)
{
	g_terminate_early = 1;
	sleep(1);
}

void* playmp3_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = mp3_play(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int mp3play_read_params(void) {
	struct audiotest_thread_context *context;
	struct itimerspec ts;
	char *token;
	int ret_val = 0;

	printf("This option is not supported currently\n");
	return -1;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/test.mp3";
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
			context->type = AUDIOTEST_TEST_MOD_MP3_DEC;
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL,
					playmp3_thread, (void*) context);
		}
	}

	return ret_val;

}

int mp3_play_control_handler(void* private_data) {
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

const char *mp3play_help_txt =
"Play MP3 file: type \n\
echo \"playmp3 path_of_file -id=xxx -timeout=x -volume=x\" > /data/audio_test \n\
Volume = 0 (default, volume=100), 1 (test different volumes continuously) \n\
timeout = x (value in seconds) \n ";

void mp3play_help_menu(void) {
	printf("%s\n", mp3play_help_txt);
}

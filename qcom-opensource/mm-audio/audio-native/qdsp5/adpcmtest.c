/* adpcmtest.c - native ADPCM test application
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
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
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/msm_audio.h>
#include <pthread.h>
#include <errno.h>
#include "audiotest_def.h"

/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */

#define EOS 1

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164
#define ID_FACT 0x74636166

#define FORMAT_ADPCM 17 /* Microsoft ADPCM - 2 \
			  IMA ADPCM - 17 \
			  ITU G.723 - 20 (Yamaha) \
			  ITU G.721 - 64 */
struct wav_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t riff_fmt;
	uint32_t fmt_id;
	uint32_t fmt_sz;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;	  /* sample_rate * num_channels * bps / 8 */
	uint16_t block_align;	  /* num_channels * bps / 8 */
	uint16_t bits_per_sample;
	uint16_t data_id1;
	uint16_t block_sz;
	uint32_t chunk_id; /*fact or data*/
	uint32_t chunk_sz;
} __attribute__ ((packed));

struct pcm_header {
	char riff_id[4];
	uint32_t riff_sz;
	char riff_fmt[4];
	char fmt_id[4];
	uint32_t fmt_sz;
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;	  /* sample_rate * num_channels * bps / 8 */
	uint16_t block_align;	  /* num_channels * bps / 8 */
	uint16_t bits_per_sample;
	char data_id[4];
	uint32_t data_sz;
} __attribute__ ((packed));

static struct pcm_header append_header = {
	{'R', 'I', 'F', 'F'}, 0, {'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '}, 16, 1, 1, 8000, 16000, 2,
	16, {'d', 'a', 't', 'a'}, 0
	};

typedef struct TIMESTAMP{
	unsigned long LowPart;
	unsigned long HighPart;
} __attribute__ ((packed)) TIMESTAMP;

struct meta_in{
	unsigned short offset;
	TIMESTAMP ntimestamp;
	unsigned int nflags;
} __attribute__ ((packed));

struct meta_out{
	unsigned short offset;
	TIMESTAMP ntimestamp;
	unsigned int nflags;
	unsigned short errflag;
	unsigned short sample_frequency;
	unsigned short channel;
	unsigned int tick_count;
} __attribute__ ((packed));

#ifdef _ANDROID_
static const char *cmdfile = "/data/audio_test";
#else
static const char *cmdfile = "/tmp/audio_test";
#endif

static void create_wav_header(int Datasize)
{
	append_header.riff_sz = Datasize + 8 + 16 + 12;
	append_header.data_sz = Datasize;
}

static void *adpcm_dec(void *arg)
{
	int fd, ret_val = 0;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd;
	unsigned long long *time;
	struct meta_out meta;
	int len, total_len;
	len = 0;
	total_len = 0;

	fd = open(audio_data->outfile, O_RDWR | O_CREAT,
		  S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd < 0) {
		printf("Err while opening file decoder \
			output file :%s\n", audio_data->outfile);
		pthread_exit((void *)ret_val);
	}

	printf(" adpcm_read Thread \n");

	lseek(fd, 44, SEEK_SET);	/* Set Space for Wave Header */
	do {
		if (audio_data->suspend == 1) {
			printf("enter suspend mode\n");
			ioctl(afd, AUDIO_STOP, 0);
			while (audio_data->suspend == 1)
				sleep(1);
			ioctl(afd, AUDIO_START, 0);
			printf("exit suspend mode\n");
		}
		printf("Before read\n");
		len = read(afd, audio_data->recbuf, audio_data->recsize);
		printf(" Read = %d PCM samples\n", len/2);
		if (len < 0) {
			printf("error reading the PCM samples \n");
			goto fail;
		} else if (len != 0) {
			memcpy(&meta, audio_data->recbuf, sizeof(struct meta_out));
			time = (unsigned long long *)(audio_data->recbuf + 2);
			meta.ntimestamp.LowPart = (*time & 0xFFFFFFFF);
			meta.ntimestamp.HighPart = ((*time >> 32) & 0xFFFFFFFF);
			#ifdef DEBUG_LOCAL
			printf("Meta_out High part is %lu\n",
					meta.ntimestamp.HighPart);
			printf("Meta_out Low part is %lu\n",
					meta.ntimestamp.LowPart);
			printf("Meta Out Timestamp: %llu\n",
					(((unsigned long long)meta.ntimestamp.HighPart << 32)
					 + meta.ntimestamp.LowPart));
			#endif
			if (meta.nflags == EOS) {
				printf("Reached end of file\n");
				break;
			}
			len = (len - sizeof(struct meta_out));
			if (len > 0) {
				if (write(fd, (audio_data->recbuf +
				sizeof(struct meta_out)), len) != len) {
					printf(" error writing the PCM \
							samples to file \n");
					goto fail;
				}
			}
		} else if (len == 0)
			printf("Unexpected case: read count zero\n");
		total_len += len;
	} while (1);
	create_wav_header(total_len);
	lseek(fd, 0, SEEK_SET);
	write(fd, (char *)&append_header, 44);
	close(fd);
	printf("Done writing output file\n");
	free(audio_data->recbuf);
	pthread_exit((void *)ret_val);

fail:
	close(fd);
	free(audio_data->recbuf);
	pthread_exit((void *)ret_val);
    return NULL;
}

static void *event_notify(void *arg)
{
	long ret_drv;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd;
	struct msm_audio_event suspend_event;
	do {
		printf("event_notify thread started\n");
		suspend_event.timeout_ms = 0;
		ret_drv = ioctl(afd, AUDIO_GET_EVENT, &suspend_event);
		if (ret_drv < 0) {
			printf("event_notify thread exiting: \
				Got Abort event or timedout\n");
			break;
		} else {
			if (suspend_event.event_type == AUDIO_EVENT_SUSPEND) {
				printf("event_notify: RECEIVED EVENT FROM \
					DRIVER OF TYPE: AUDIO_EVENT_SUSPEND: \
					%d\n", suspend_event.event_type);
				audio_data->suspend = 1;
				sleep(1);
			} else if
			(suspend_event.event_type == AUDIO_EVENT_RESUME) {
				printf("event_notify: RECEIVED EVENT FROM \
					DRIVER OF TYPE: AUDIO_EVENT_RESUME : \
					%d\n", suspend_event.event_type);
				audio_data->suspend = 0;
			}
		}
	} while (1);
	return NULL;
}

static int adpcm_play(struct audtest_config *cfg, unsigned rate,
		unsigned channels, int (*fill)(void *buf,
		unsigned sz, void *cookie), void *cookie)
{
	static struct msm_audio_config config;
	/* struct msm_audio_stats stats; */
	pthread_t thread, event_th;
	unsigned n;
	int sz;
	char *buf;
	int afd;
	int cntW = 0, ret = 0;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) cfg->private_data;
#ifdef AUDIOV2
	unsigned short dec_id;
#endif

	if (audio_data->mode) {
		printf("non tunnel mode open\n");
		afd = open("/dev/msm_adpcm", O_RDWR);
	} else {
		printf("tunnel mode open\n");
		afd = open("/dev/msm_adpcm", O_WRONLY);
	}
	if (afd < 0) {
		perror("adpcm_play: cannot open audio device");
		return -1;
	}

	audio_data->afd = afd; /* Store */

#ifdef AUDIOV2
	if (!audio_data->mode) {
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
	}
#endif

	pthread_create(&event_th, NULL, event_notify, (void *) audio_data);

	if (ioctl(afd, AUDIO_GET_CONFIG, &config)) {
		perror("could not get config");
		ret = -1;
		goto err_state;
	}

	config.channel_count = channels;
	config.sample_rate = rate;
	config.bits = audio_data->bits;
	if (audio_data->mode)
		config.meta_field = 1;
	if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
		perror("could not set config");
		ret = -1;
		goto err_state;
	}

	buf = (char *) malloc(sizeof(char) * config.buffer_size);
	if (buf == NULL) {
		perror("fail to allocate buffer\n");
		ret = -1;
		goto err_state;
	}

	printf("initiate_play: buffer_size=%d, buffer_count=%d\n", config.buffer_size,
		   config.buffer_count);

	if (audio_data->mode)
		config.buffer_size -= sizeof(struct meta_in);

	fprintf(stderr, "prefill\n");

	if (audio_data->mode) {
		/* non - tunnel portion */
		struct msm_audio_pcm_config config_rec;
		printf(" selected non-tunnel part\n");
		append_header.sample_rate = config.sample_rate;
		append_header.num_channels = config.channel_count;
		append_header.byte_rate = append_header.sample_rate *
			append_header.num_channels * 2;
		append_header.block_align = append_header.num_channels * 2;
		if (ioctl(afd, AUDIO_GET_PCM_CONFIG, &config_rec)) {
			printf("could not get PCM config\n");
			free(buf);
			ret = -1;
			goto err_state;
		}
		printf(" config_rec.pcm_feedback = %d, \
			config_rec.buffer_count = %d , \
			config_rec.buffer_size=%d \n", \
			config_rec.pcm_feedback, \
			config_rec.buffer_count, config_rec.buffer_size);
		config_rec.pcm_feedback = 1;
		audio_data->recsize = config_rec.buffer_size;
		audio_data->recbuf = (char *)malloc(config_rec.buffer_size);
		if (!audio_data->recbuf) {
			printf("could not allocate memory for decoding\n");
			free(buf);
			ret = -1;
			goto err_state;
		}
		memset(audio_data->recbuf, 0, config_rec.buffer_size);
		if (ioctl(afd, AUDIO_SET_PCM_CONFIG, &config_rec)) {
			printf("could not set PCM config\n");
			free(audio_data->recbuf);
			ret = -1;
			goto err_state;
		}
		pthread_create(&thread, NULL, adpcm_dec, (void *) audio_data);

	}
	printf("Before fill loop\n");
	for (n = 0; n < config.buffer_count; n++) {
		if ((sz = fill(buf, config.buffer_size, cookie)) < 0)
			break;
		if (write(afd, buf, sz) != sz)
			break;
	}
	cntW = cntW+config.buffer_count;

	fprintf(stderr, "start playback\n");
	if (ioctl(afd, AUDIO_START, 0) >= 0) {
		for (;;) {
#if 0
			if (ioctl(afd, AUDIO_GET_STATS, &stats) == 0)
				fprintf(stderr, "%10d\n", stats.out_bytes);
#endif
			if (((sz = fill(buf, config.buffer_size,
				cookie)) < 0) || (audio_data->quit == 1)) {
				if ((audio_data->repeat == 0) || (audio_data->quit == 1)) {
					printf(" File reached end or quit cmd issued, exit loop \n");
					if (audio_data->mode) {
						struct meta_in meta;
						meta.offset =
							sizeof(struct meta_in);
						meta.ntimestamp.LowPart =
						((audio_data->frame_count * 20000) & 0xFFFFFFFF);
						meta.ntimestamp.HighPart =
						(((unsigned long long)(audio_data->frame_count
						* 20000) >> 32) & 0xFFFFFFFF);
						meta.nflags = EOS;
						#ifdef DEBUG_LOCAL
						printf("Meta In High part is %lu\n",
							meta.ntimestamp.HighPart);
						printf("Meta In Low part is %lu\n",
							meta.ntimestamp.LowPart);
						printf("Meta In ntimestamp: %llu\n",
						(((unsigned long long)meta.ntimestamp.HighPart << 32)
						 + meta.ntimestamp.LowPart));
						#endif
						memset(buf, 0,
						sizeof(config.buffer_size));
						memcpy(buf, &meta,
							sizeof(struct meta_in));
						if (write(afd, buf,
						sizeof(struct meta_in)) < 0)
							printf(" writing buffer\
							for EOS failed\n");
					} else {
						printf("FSYNC: Reached end of \
							file, calling fsync\n");
						while (fsync(afd) < 0)
							printf("fsync \
								failed\n");
					}
					printf(" fill return NON NULL, exit loop \n");
					break;
				} else {
					printf("\nRepeat playback\n");
					audio_data->avail = audio_data->org_avail;
					audio_data->next  = audio_data->org_next;
					cntW = 0;
					if(audio_data->repeat > 0)
						audio_data->repeat--;
					sleep(1);
					continue;
				}
			}
			if (write(afd, buf, sz) != sz) {
				if (audio_data->flush_enable == 1 && errno == EBUSY) {
					printf("Flush in progress\n");
					while (write(afd, buf, sz) < 0)
						usleep(10000);
					audio_data->avail = audio_data->org_avail;
					audio_data->next  = audio_data->org_next;
					audio_data->flush_enable = 0;
					printf("Flush done");
					continue;
				}
				printf(" write return not equal to sz, exit loop\n");
				break;
			} else {
				cntW++;
				printf(" adpcm_play: instance=%d repeat_cont=%d cntW=%d\n",
						(int) audio_data, audio_data->repeat, cntW);
			}
		}
		printf("end of adpcm play\n");
		sleep(3);
		ioctl(afd, AUDIO_ABORT_GET_EVENT, 0);
		ioctl(afd, AUDIO_STOP, 0);
	} else {
		printf("adpcm_play: Unable to start driver\n");
	}
err_state:
#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
	if (!audio_data->mode) {
		if (devmgr_unregister_session(dec_id, DIR_RX) < 0)
			ret = -1;
	}
exit:
#endif
	close(afd);
	return ret;
}


static int fill_buffer(void *buf, unsigned sz, void *cookie)
{
	struct meta_in meta;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) cookie;
	unsigned cpy_size = (sz < audio_data->avail ? sz : audio_data->avail);

	if (audio_data->avail == 0)
		return -1;

	if (audio_data->mode) {
		meta.ntimestamp.LowPart = ((audio_data->frame_count * 20000) & 0xFFFFFFFF);
		meta.ntimestamp.HighPart = (((unsigned long long)(audio_data->frame_count
						* 20000) >> 32) & 0xFFFFFFFF);
		meta.offset = sizeof(struct meta_in);
		meta.nflags = 0;
		audio_data->frame_count++;
		#ifdef DEBUG_LOCAL
		printf("Meta In High part is %lu\n",
				meta.ntimestamp.HighPart);
		printf("Meta In Low part is %lu\n",
				meta.ntimestamp.LowPart);
		printf("Meta In ntimestamp: %llu\n", (((unsigned long long)
					meta.ntimestamp.HighPart << 32) +
					meta.ntimestamp.LowPart));
		#endif
		memcpy(buf, &meta, sizeof(struct meta_in));
		memcpy(((char *)buf + sizeof(struct meta_in)), audio_data->next, cpy_size);
	} else
		memcpy(buf, audio_data->next, cpy_size);

	audio_data->next += cpy_size;
	audio_data->avail -= cpy_size;

	if (audio_data->mode)
		return cpy_size + sizeof(struct meta_in);
	else
		return cpy_size;
}

static int play_file(struct audtest_config *config, unsigned rate,
		unsigned channels, int fd, size_t count)
{
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config->private_data;
	int ret_val = 0;
	char *content_buf;

	audio_data->next = (char *)malloc(count);
	printf(" play_file: count=%d,next=%p\n", count, audio_data->next);
	if (!audio_data->next) {
		fprintf(stderr, "could not allocate %d bytes\n", count);
		return -1;
	}
	content_buf = audio_data->next;
	audio_data->org_next = audio_data->next;

	if (read(fd, audio_data->next, count) != (ssize_t) count) {
		fprintf(stderr, "could not read %d bytes\n", count);
		free(content_buf);
		return -1;
	}
	audio_data->avail = count;
	audio_data->org_avail = audio_data->avail;
	ret_val = adpcm_play(config, rate, channels, fill_buffer, audio_data);
	free(content_buf);
	return ret_val;
}

int adpcm_wav_play(struct audtest_config *config)
{
	struct wav_header hdr;
	uint32_t data_size, id_data;
	int fd;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config->private_data;

	if (config == NULL)
		return -1;

	fd = open(config->file_name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "playwav: cannot open '%s'\n", config->file_name);
		return -1;
	}
	if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		fprintf(stderr, "playwav: cannot read header\n");
		return -1;
	}

	if ((hdr.riff_id != ID_RIFF) ||
		(hdr.riff_fmt != ID_WAVE) ||
		(hdr.fmt_id != ID_FMT)) {
		fprintf(stderr, "playwav: '%s' is not a riff/wave file\n",
				config->file_name);
		return -1;
	}
	if ((hdr.audio_format != FORMAT_ADPCM)) {
		fprintf(stderr, "playwav: '%s' is not adpcm format %d and fmt size is %d\n", config->file_name, hdr.audio_format, hdr.fmt_sz);
		return -1;
	}

	audio_data->bits = hdr.block_sz;

	if (hdr.chunk_id == ID_FACT) {
		printf("Fact chunk id is %d and size is %d\n", hdr.chunk_id, hdr.chunk_sz);
		lseek(fd, hdr.chunk_sz, SEEK_CUR);
		read(fd, &id_data, sizeof(id_data));
		if (id_data == ID_DATA) {
			read(fd, &data_size, sizeof(data_size));
			printf("Data size is %d\n", data_size);
		} else {
			printf("Unable to parse header data id_data is %d\n", id_data);
			return -1;
		}
	} else if (hdr.chunk_id == ID_DATA) {
		printf("Data size is %d\n", hdr.chunk_sz);
		data_size = hdr.chunk_sz;
	} else {
			printf("Unable to parse header data id_data is %d\n", id_data);
			return -1;
	}

	return play_file(config, hdr.sample_rate, hdr.num_channels,
					 fd, data_size);
}

void *playadpcm_thread(void *arg)
{
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context *) arg;
	int ret_val;

	ret_val = adpcm_wav_play(&context->config);
	printf(" Free audio instance 0x%8x \n", (unsigned int) context->config.private_data);
	free(context->config.private_data);
	free_context(context);
	pthread_exit((void *) ret_val);

    return NULL;
}

int adpcmplay_read_params(void)
{
	struct audiotest_thread_context *context;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/data.wav";
		struct audio_pvt_data *audio_data;
		audio_data = (struct audio_pvt_data *) malloc(sizeof(struct audio_pvt_data));
		if (!audio_data) {
			printf("error allocating audio instance structure \n");
			free_context(context);
			ret_val = -1;
		} else {
			printf("Created audio instance 0x%8x\n", (unsigned int) audio_data);
			memset(audio_data, 0, sizeof(struct audio_pvt_data));
			#ifdef _ANDROID_
				audio_data->outfile = "/data/pcm.wav";
			#else
				audio_data->outfile = "/tmp/pcm.wav";
			#endif
			audio_data->repeat = 0;
			audio_data->quit = 0;
			context->config.file_name = "/data/data.wav";
			context->type = AUDIOTEST_TEST_MOD_ADPCM_DEC;
			audio_data->mode = 0;

			token = strtok(NULL, " ");

			while (token != NULL) {
				if (!memcmp(token, "-id=", (sizeof("-id=")-1))) {
					context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
				} else if
				(!memcmp(token, "-mode=", (sizeof("-mode=") - 1))) {
					audio_data->mode = atoi(&token[sizeof("-mode=") - 1]);
				} else if (!memcmp(token, "-rate=", (sizeof("-rate=" - 1)))) {
					context->config.sample_rate = atoi(&token[sizeof("-rate=") - 1]);
				} else if (!memcmp(token, "-channels=", (sizeof("-channels=" - 1)))) {
					context->config.channel_mode = atoi(&token[sizeof("-channels=") - 1]);
				} else if (!memcmp(token, "-block=", (sizeof("-block=" - 1)))) {
					audio_data->bits = atoi(&token[sizeof("-block=") - 1]);
				} else if (!memcmp(token, "-out=",
						(sizeof("-out=") - 1))) {
					audio_data->outfile = token +
							(sizeof("-out=")-1);
				} else if (!memcmp(token, "-repeat=",
					(sizeof("-repeat=") - 1))) {
					audio_data->repeat = atoi(&token[sizeof("-repeat=") - 1]);
					if (audio_data->repeat == 0)
						audio_data->repeat = -1;
					else
						audio_data->repeat--;
				} else {
					context->config.file_name = token;
				}
				token = strtok(NULL, " ");
			}
		}
		context->config.private_data = (struct audio_pvt_data *) audio_data;
		pthread_create(&context->thread, NULL,
				playadpcm_thread, (void *) context);
	}

	return ret_val;
}

int adpcm_play_control_handler(void *private_data)
{
	int drvfd, ret_val = 0;
	char *token;
	int volume;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) private_data;

	token = strtok(NULL, " ");
	if ((private_data != NULL) &&
			(token != NULL)) {
		drvfd = audio_data->afd;
		if (!memcmp(token, "-cmd=", (sizeof("-cmd=")-1))) {
			token = &token[sizeof("-cmd=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
			if (!strcmp(token, "pause")) {
				ioctl(drvfd, AUDIO_PAUSE, 1);
			} else if (!strcmp(token, "resume")) {
				ioctl(drvfd, AUDIO_PAUSE, 0);
			} else if (!strcmp(token, "volume")) {
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
						rc = msm_set_volume(dec_id, volume);
						printf("session volume result %d\n", rc);
					}
				}
			} else if (!strcmp(token, "flush")) {
				audio_data->flush_enable = 1;
				ioctl(drvfd, AUDIO_FLUSH, 0);
			}  else if (!strcmp(token, "quit")) {
				audio_data->quit = 1;
				printf("quit session\n");
			}
		}
	} else
		ret_val = -1;
	return ret_val;
}

const char *adpcmplay_help_txt =
"Play ADPCM file: type \n\
echo \"playadpcm path_of_file -id=xxx -repeat=x -out=<filename>\" > %s \n\
Sample rate of ADPCM file <= 48000 \n\
Bits per sample = 16 bits \n\
Repeat 'x' no. of times, repeat infinitely if repeat = 0\n\
Supported control command: pause, resume, volume, flush, quit\n ";

void adpcmplay_help_menu(void)
{
	printf(adpcmplay_help_txt, cmdfile);
}


/* wmaprotest.c - native WMAPRO10 test application
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
#include <errno.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <linux/msm_audio.h>
#include <linux/msm_audio_wmapro.h>
#include "audiotest_def.h"

#define WMAPRO_DEVICE_NODE "/dev/msm_wmapro"
#define EOS 1

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

static struct wav_header append_header = {
	{'R', 'I', 'F', 'F'}, 0, {'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '}, 16, 1, 2, 48000, 8001, 2,
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
	append_header.Chunk_size = Datasize + 8 + 16 + 12;
	append_header.Chunk_data_size = Datasize;
	return;
}

static void *wmapro_dec(void *arg)
{
	int fd, ret_val = 0;
	struct meta_out meta;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd;
	unsigned long long *time;
	int len, total_len = 0;
	len = 0;
	total_len = 0;

	fd = open(audio_data->outfile, O_RDWR | O_CREAT,
			S_IRWXU | S_IRWXG | S_IRWXO);

	if (fd < 0) {
		printf("Err while opening file decoder output file \n");
		pthread_exit((void *)ret_val);
	}

	printf(" qcp_read Thread \n");

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
		len = read(afd, audio_data->recbuf, audio_data->recsize);
#ifdef DEBUG_LOCAL
		printf(" Read %d Bytes of data \n", len);
#endif
		if (len < 0) {
			if (audio_data->flush_enable == 1 && errno == EBUSY) {
				printf("Flush in progress\n");
				usleep(5000);
				continue;
			} else {
				printf(" error reading the PCM samples \n");
				goto fail;
			}
		} else if (len != 0) {
			memcpy(&meta, audio_data->recbuf,
					sizeof(struct meta_out));
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
	pthread_exit((void *)ret_val);

fail:
	printf("error:Reached fail\n");
	close(fd);
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

static int wmapro_start(struct audtest_config *clnt_config)
{
	int fd, afd, ii, count, ret, pkts_size, size;
	int retval = 0;
	pthread_t thread, event_th;
	static struct msm_audio_config config;
	static struct msm_audio_wmapro_config wmapro_config;
	int wmapro_opflg = 0;
	char *transcodebuf = NULL;
	struct audio_pvt_data *audio_data =
		(struct audio_pvt_data *) clnt_config->private_data;
#ifdef AUDIOV2
	unsigned short dec_id;
#endif

	/* Open the file for operation */
	fd = open(clnt_config->file_name, O_RDONLY);
	if (fd < 0) {
		printf("unable to open wmapro  file =%s\n",
		       clnt_config->file_name);
		return -1;
	}

	if (audio_data->mode) {
		printf("non-tunnel mode\n");
		afd = open(WMAPRO_DEVICE_NODE, O_RDWR);
	} else {
		printf("tunnel mode\n");
		afd = open(WMAPRO_DEVICE_NODE, O_WRONLY);
	}

	wmapro_opflg = 0;
	if (afd < 0) {
		printf("Unable to open audio device = %s\n",
				WMAPRO_DEVICE_NODE);
		goto file_err;
	}
	/* Store handle for commands */
	audio_data->afd = afd;

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
		retval = -1;
		goto err_state;
	}

	if (audio_data->mode)
		config.meta_field = 1;
	config.sample_rate = audio_data->freq;
	config.channel_count = audio_data->channels;
	if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
		perror("could not set config");
		retval = -1;
		goto err_state;
	}

	wmapro_config.armdatareqthr = audio_data->datareqthr;
	wmapro_config.numchannels = audio_data->channels;
	wmapro_config.validbitspersample = audio_data->bitspersample;
	wmapro_config.formattag = audio_data->formattag;
	wmapro_config.samplingrate = audio_data->freq;
	wmapro_config.avgbytespersecond = audio_data->bps;
	wmapro_config.asfpacketlength = audio_data->asfpacketlength;
	wmapro_config.channelmask = audio_data->channelmask;
	wmapro_config.encodeopt = audio_data->encopt;
	wmapro_config.advancedencodeopt = audio_data->advancedencodeopt;
	wmapro_config.advancedencodeopt2 = audio_data->advancedencodeopt2;

	printf("wmapro_config.armdatareqthr=%d\n", wmapro_config.armdatareqthr);
	printf("wmapro_config.numchannels=%d\n", wmapro_config.numchannels);
	printf("wmapro_config.samplingrate=%d\n", wmapro_config.samplingrate);
	printf("wmapro_config.avgbytespersec=%d\n",
			wmapro_config.avgbytespersecond);
	printf("wmapro_config.validbitspersample=%d\n",
			wmapro_config.validbitspersample);
	printf("wmapro_config.formattag=%d\n", wmapro_config.formattag);
	printf("wmapro_config.channelmask=%d\n", wmapro_config.channelmask);
	printf("wmapro_config.asfpacketlength=%d\n",
			wmapro_config.asfpacketlength);
	printf("wmapro_config.encodeopts=%d\n", wmapro_config.encodeopt);
	printf("wmapro_config.advancedencodeopt=%d\n",
			wmapro_config.advancedencodeopt);
	printf("wmapro_config.advancedencodeopt2=%d\n",
			wmapro_config.advancedencodeopt2);
	if (ioctl(afd, AUDIO_SET_WMAPRO_CONFIG, &wmapro_config)) {
		perror("could not set WMAPRO config");
		retval = -1;
		goto err_state;
	}

	transcodebuf = (char *)malloc(config.buffer_size);
	if (!transcodebuf) {
		printf("could not allocate memory for store transcoded data\n");
		retval = -1;
		goto err_state;
	}
	memset(transcodebuf, 0, config.buffer_size);
	printf("transcodebuf = %d\n", (int) transcodebuf);

	if (!audio_data->mode)
		config.buffer_size =
			(config.buffer_size - sizeof(struct meta_in));

	if (audio_data->mode) {
		/* non - tunnel portion */
		struct msm_audio_pcm_config config_rec;
		printf(" selected non-tunnel part\n");
		append_header.Sample_rate = wmapro_config.samplingrate;
		append_header.Number_Channels = wmapro_config.numchannels;
		append_header.Bytes_Sec = append_header.Sample_rate *
			append_header.Number_Channels * 2;
		append_header.Block_align = append_header.Number_Channels * 2;
		if (ioctl(afd, AUDIO_GET_PCM_CONFIG, &config_rec)) {
			printf("could not get PCM config\n");
			retval = -1;
			goto err_state;
		}
		printf("config_rec.pcm_feedback=%d config_rec.buffer_count=%d \
			       config_rec.buffer_size=%d \n",
			       config_rec.pcm_feedback, config_rec.buffer_count,
			       config_rec.buffer_size);
		config_rec.pcm_feedback = 1;
		audio_data->recsize = config_rec.buffer_size;
		audio_data->recbuf = (char *)malloc(config_rec.buffer_size);
		if (!audio_data->recbuf) {
			printf("could not allocate memory for decoding\n");
			retval = -1;
			goto err_state;
		}
		memset(audio_data->recbuf, 0, config_rec.buffer_size);
		if (ioctl(afd, AUDIO_SET_PCM_CONFIG, &config_rec)) {
			printf("could not set PCM config\n");
			retval = -1;
			goto err_state;
		}
		pthread_create(&thread, NULL, wmapro_dec, (void *)audio_data);

	}
	audio_data->start_ptr = 0x0;
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, audio_data->start_ptr, SEEK_SET);

	if (audio_data->mode)
		pkts_size = (config.buffer_size - sizeof(struct meta_in));
	else
		pkts_size = config.buffer_size;

	for (ii = 0, count = 0; ; ii = ii + pkts_size) {
		if ((ii < size) && (audio_data->quit != 1)) {
			if (audio_data->mode) {
				struct meta_in meta;
				meta.ntimestamp.LowPart = ((audio_data->frame_count *
							20000) & 0xFFFFFFFF);
				meta.ntimestamp.HighPart =
					(((unsigned long long)(audio_data->frame_count
						       * 20000) >> 32) & 0xFFFFFFFF);
				meta.offset = sizeof(struct meta_in);
				meta.nflags = 0;
				audio_data->frame_count++;
				#ifdef DEBUG_LOCAL
				printf("Meta In High part is %lu\n",
						meta.ntimestamp.HighPart);
				printf("Meta In Low part is %lu\n",
						meta.ntimestamp.LowPart);
				printf("Meta In ntimestamp: %llu\n",
					(((unsigned long long)meta.ntimestamp.HighPart
						<< 32) + meta.ntimestamp.LowPart));
				#endif
				memcpy(transcodebuf, &meta, sizeof(struct meta_in));
				read(fd, (transcodebuf + sizeof(struct meta_in)),
						pkts_size);
			} else
				read(fd, transcodebuf, pkts_size);
			count++;
			printf("writing %d no of packets\n", count);
			if (audio_data->suspend == 1) {
				printf("enter suspend mode\n");
				ioctl(afd, AUDIO_STOP, 0);
				while (audio_data->suspend == 1)
					sleep(1);
				ioctl(afd, AUDIO_START, 0);
				printf("exit suspend mode\n");
			}
			ret = write(afd, transcodebuf, config.buffer_size);
			printf("ret = %d\n", ret);
			if ((ret < 0) && (audio_data->flush_enable == 1)) {
				printf("Flush in progress \n");
				usleep(5000);
				ii = 0;
				/* Set to start of data portion */
				lseek(fd, audio_data->start_ptr, SEEK_SET);
			}
			if (count == 2)
				ioctl(afd, AUDIO_START, 0);
		} else if ((ii >= size) && (audio_data->repeat != 0)
				&& (audio_data->quit != 1)) {
			printf("\nRepeat playback\n");
			ii = 0;
			count = 2;
			/* Set to start of data portion */
			lseek(fd, audio_data->start_ptr, SEEK_SET);
			if(audio_data->repeat > 0)
				audio_data->repeat--;
			sleep(1);
			continue;
		} else if (((ii >= size) && (audio_data->repeat == 0))
				|| (audio_data->quit == 1))
			break;
	}

	printf(" File reached end or quit cmd issued, exit loop \n");
	if (audio_data->mode) {
		struct meta_in meta;
		meta.offset = sizeof(struct meta_in);
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
		memset(transcodebuf, 0,
			sizeof(config.buffer_size));
		memcpy(transcodebuf, &meta,
			sizeof(struct meta_in));
		if (write(afd, transcodebuf,
			sizeof(struct meta_in)) < 0)
			printf(" writing buffer\
				for EOS failed\n");
	} else {
		printf("FSYNC: Reached end of file, calling fsync\n");
		if (fsync(afd) < 0)
			printf(" fsync failed\n");
	}
	sleep(2);
	ioctl(afd, AUDIO_STOP, 0);
	printf("count = %d\n", count);
	ioctl(afd, AUDIO_ABORT_GET_EVENT, 0);
err_state:
#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
	if (!audio_data->mode) {
		if (devmgr_unregister_session(dec_id, DIR_RX) < 0)
			ret = -1;
	}
exit:
#endif
	if (transcodebuf)
		free(transcodebuf);
	if (audio_data->recbuf)
		free(audio_data->recbuf);
	close(afd);
	return retval;
file_err:
	close(fd);
	return -1;
}

void *wmapro_thread(void *arg)
{
	struct audiotest_thread_context *context =
	    (struct audiotest_thread_context *)arg;
	int ret_val;

	ret_val = wmapro_start(&context->config);
	printf("Free audio instance 0x%8x\n",
			(unsigned int)context->config.private_data);
	free(context->config.private_data);
	free_context(context);
	pthread_exit((void *)ret_val);
	return NULL;
}

int wmaproplay_read_params(void)
{
	struct audiotest_thread_context *context;
	char *token = NULL;
	int ret_val = 0;

	context = get_free_context();
	if (context == NULL) {
		ret_val = -1;
	} else {
		struct audio_pvt_data *audio_data;
		audio_data = (struct audio_pvt_data *)
			malloc(sizeof(struct audio_pvt_data));
		if (!audio_data) {
			printf("error allocating audio instance structure\n");
			free_context(context);
			ret_val = -1;
		} else {
			printf("Created audio instance 0x%8x\n",
					(unsigned int) audio_data);
			/* Set complete zero */
			memset(audio_data, 0, sizeof(struct audio_pvt_data));
			context->config.file_name = "/data/sample.strm";
			context->type = AUDIOTEST_TEST_MOD_WMAPRO_DEC;
			#ifdef _ANDROID_
			audio_data->outfile = "/data/pcm.wav";
			#else
			audio_data->outfile = "/tmp/pcm.wav";
			#endif
			audio_data->repeat = 0;
			audio_data->quit = 0;
			audio_data->mode = 0;
			token = strtok(NULL, " ");

			while (token != NULL) {
				if (!memcmp(token, "-id=",
						(sizeof("-id=") - 1))) {
					context->cxt_id =
					    atoi(&token[sizeof("-id=") - 1]);
					printf("Context id =%d\n",
							context->cxt_id);
				} else if (!memcmp(token, "-mode=",
						   (sizeof("-mode=") - 1))) {
					audio_data->mode =
						atoi(&token[sizeof("-mode=") - 1]);
				} else if (!memcmp(token, "-out=",
						   (sizeof("-out=") - 1))) {
					audio_data->outfile = token + (sizeof("-out=") - 1);
				} else if (!memcmp(token, "-datareqthr=",
					   (sizeof("-datareqthr=") - 1))) {
					audio_data->datareqthr =
						atoi(&token[sizeof("-datareqthr=") - 1]);
				} else if (!memcmp(token, "-channels=",
						   (sizeof("-channels=") - 1))) {
					audio_data->channels =
						atoi(&token[sizeof("-channels=") - 1]);
				} else if (!memcmp(token, "-bps=",
						   (sizeof("-bps=") - 1))) {
					audio_data->bps =
						atoi(&token[sizeof("-bps=") - 1]);
				} else if (!memcmp(token, "-freq=",
						   (sizeof("-freq=") - 1))) {
					audio_data->freq =
						atoi(&token[sizeof("-freq=") - 1]);
				} else if (!memcmp(token, "-encopt=",
						   (sizeof("-encopt=") - 1))) {
					audio_data->encopt =
						atoi(&token[sizeof("-encopt=") - 1]);
				} else if (!memcmp(token, "-formattag=",
					(sizeof("-formattag=") - 1))) {
					audio_data->formattag =
						atoi(&token[sizeof("-formattag=") - 1]);
				} else if (!memcmp(token, "-asfpacketlength=",
					   (sizeof("-asfpacketlength=") - 1))) {
					audio_data->asfpacketlength =
						atoi(&token[sizeof("-asfpacketlength=") - 1]);
				} else if (!memcmp(token,
					"-advancedencodeopt=",
					(sizeof("-advancedencodeopt=") - 1))) {
					audio_data->advancedencodeopt =
						atoi(&token[sizeof("-advancedencodeopt=") - 1]);
				} else if (!memcmp(token,
					"-advancedencodeopt2=",
					(sizeof("-advancedencodeopt2=") - 1))) {
					audio_data->advancedencodeopt2 =
						atoi(&token[sizeof("-advancedencodeopt2=") - 1]);
				} else if (!memcmp(token, "-channelmask=",
					   (sizeof("-channelmask=") - 1))) {
					audio_data->channelmask =
						atoi(&token[sizeof("-channelmask=") - 1]);
				} else if (!memcmp(token, "-bitspersample=",
					   (sizeof("-bitspersample=") - 1))) {
					audio_data->bitspersample =
						atoi(&token[sizeof("-bitspersample=") - 1]);
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
			if (audio_data->mode == 1) {	/* non-tunnel */
				context->config.sample_rate = 8000;
				context->config.channel_mode = 1;
			}
			printf("reading parameters\n\n");
			context->config.private_data =
				(struct audio_pvt_data *) audio_data;
			pthread_create(&context->thread, NULL,
				       wmapro_thread, (void *)context);
		}

	}
	return ret_val;

}

int wmapro_play_control_handler(void *private_data)
{
	int drvfd, ret_val = 0;
	int volume;
	char *token;
	struct audio_pvt_data *audio_data =
		(struct audio_pvt_data *) private_data;

	printf("reached here\n");
	token = strtok(NULL, " ");
	if ((private_data != NULL) && (token != NULL)) {
		drvfd = audio_data->afd;
		if (!memcmp(token, "-cmd=", (sizeof("-cmd=") - 1))) {
			token = &token[sizeof("-cmd=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
			if (!strcmp(token, "pause")) {
				printf("reached here\n");
				ioctl(drvfd, AUDIO_PAUSE, 1);
			} else if (!strcmp(token, "resume")) {
				ioctl(drvfd, AUDIO_PAUSE, 0);
#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
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
#else
			} else if (!strcmp(token, "volume")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-value=",
					(sizeof("-value=") - 1))) {
					volume =
					atoi(&token[sizeof("-value=") - 1]);
					ioctl(drvfd, AUDIO_SET_VOLUME, volume);
					printf("volume:%d\n", volume);
				}
#endif
			} else if (!strcmp(token, "flush")) {
				audio_data->flush_enable = 1;
				ioctl(drvfd, AUDIO_FLUSH, 0);
				printf("flush\n");
			} else if (!strcmp(token, "quit")) {
				audio_data->quit = 1;
				printf("quit session\n");
			}
		}
	} else {
		ret_val = -1;
	}

	return ret_val;
}

const char *wmaproplay_help_txt = "Play WMAPRO stream file: \n\
echo \"playwmapro path_of_file -id=xxx -mode=x -datareqthr=x -channels=x\
-bps=x -freq=x -encopt=x -bitspersample=x -formattag=x  -asfpacketlength=x\
-channelmask=x -advancedencodeopt=x -advancedencodeopt2=x\
-out=path_of_outfile -repeat=x\" > %s \n Codec type of WMAPRO file\n\
mode= 0(tunnel mode) or 1 (non-tunnel mode) \n\
Repeat 'x' no. of times, repeat infinitely if repeat = 0\n\
Supported control command: pause, resume, volume, flush, quit \n\
examples: \n\
echo \"playqcp path_of_file -id=xxx -mode=<0 or 1>\" > %s \n\
echo \"control_cmd -id=xxx -cmd=pause\" > %s \n\
echo \"control_cmd -id=xxx -cmd=resume\" > %s \n\
echo \"control_cmd -id=xxx -cmd=flush\" > %s \n\
echo \"control_cmd -id=xxx -cmd=volume -value=yyyy\" > %s \n";

void wmaproplay_help_menu(void)
{
	printf(wmaproplay_help_txt, cmdfile, cmdfile, cmdfile,
			cmdfile, cmdfile, cmdfile);
}

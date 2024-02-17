/* wmatest.c - native WMA test application
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
#include <linux/msm_audio_wma.h>
#include <linux/msm_audio_wmapro.h>
#include "audiotest_def.h"

#define WMA_DEVICE_NODE "/dev/msm_wma"
#define EOS 0x00000001

static int in_size =0;
static int out_size =0;
static int file_write=0;
static int eos_ack=0;

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
static pthread_mutex_t avail_lock;
static pthread_cond_t avail_cond;
static pthread_mutex_t consumed_lock;
static pthread_cond_t consumed_cond;
static int data_is_available = 0;
static int data_is_consumed = 0;
static int in_free_indx;
static int in_data_indx;
static int out_free_indx;
static int out_data_indx;

typedef struct TIMESTAMP{
	unsigned long LowPart;
	unsigned long HighPart;
} __attribute__ ((packed)) TIMESTAMP;

struct meta_in{
	unsigned char rsv[18];
	unsigned short offset;
	TIMESTAMP ntimestamp;
	unsigned int nflags;
} __attribute__ ((packed));

struct meta_out_dsp{
	unsigned int offset_to_frame;
	unsigned int frame_size;
	unsigned int encoded_pcm_samples;
	unsigned int msw_ts;
	unsigned int lsw_ts;
	unsigned int nflags;
} __attribute__ ((packed));

struct dec_meta_out{
	unsigned int rsv[7];
	unsigned int num_of_frames;
	struct meta_out_dsp meta_out_dsp[];
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

#define WMATEST_IBUFSZ (32*1024)
#define WMATEST_NUM_IBUF 2
#define WMATEST_IPMEM_SZ (WMATEST_IBUFSZ * WMATEST_NUM_IBUF)

#define WMATEST_OBUFSZ (32*1024)
#define WMATEST_NUM_OBUF 2
#define WMATEST_OPMEM_SZ (WMATEST_OBUFSZ * WMATEST_NUM_OBUF)

#ifdef _ANDROID_
static const char *cmdfile = "/data/audio_test";
#else
static const char *cmdfile = "/tmp/audio_test";
#endif


struct msm_audio_aio_buf aio_ip_buf[WMATEST_NUM_IBUF];
struct msm_audio_aio_buf aio_op_buf[WMATEST_NUM_OBUF];

static void wait_for_data(void)
{
	pthread_mutex_lock(&avail_lock);

	while (data_is_available == 0) {
		pthread_cond_wait(&avail_cond, &avail_lock);
	}
	data_is_available = 0;
	pthread_mutex_unlock(&avail_lock);
}

static void data_available(void)
{
	pthread_mutex_lock(&avail_lock);
	if (data_is_available == 0) {
		data_is_available = 1;
		pthread_cond_broadcast(&avail_cond);
	}
	pthread_mutex_unlock(&avail_lock);
}

static void wait_for_data_consumed(void)
{
	pthread_mutex_lock(&consumed_lock);

	while (data_is_consumed == 0) {
		pthread_cond_wait(&consumed_cond, &consumed_lock);
	}
	data_is_consumed = 0;
	pthread_mutex_unlock(&consumed_lock);
}

static void data_consumed(void )
{
	pthread_mutex_lock(&consumed_lock);
	if (data_is_consumed == 0) {
		data_is_consumed = 1;
		pthread_cond_broadcast(&consumed_cond);
	}
	pthread_mutex_unlock(&consumed_lock);
}

static void create_wav_header(int Datasize)
{
	append_header.Chunk_size = Datasize + 8 + 16 + 12;
	append_header.Chunk_data_size = Datasize;
	return;
}

static void *wma_dec(void *arg)
{
	int fd, ret_val = 0;
	struct meta_out meta;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd;
	unsigned long long *time;
	int len, total_len = 0;
	len = 0;
	total_len = 0;

	fd = open(audio_data->outfile, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);

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

static int wma_start(struct audtest_config *clnt_config)
{
	int fd, afd, ii, count, ret, pkts_size, size;
	pthread_t thread, event_th;
	static struct msm_audio_config config;
	static struct msm_audio_wma_config wma_config;
	int wma_opflg = 0;
	char *transcodebuf = NULL;
	int ret_val = 0;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
#ifdef AUDIOV2
	unsigned short dec_id = 0;
#endif

	/* Open the file for operation */
	fd = open(clnt_config->file_name, O_RDONLY);
	if (fd < 0) {
		printf("unable to open wma  file =%s\n",
		       clnt_config->file_name);
		return -1;
	}

	if (audio_data->mode) {
		printf("non-tunnel mode\n");
		afd = open(WMA_DEVICE_NODE, O_RDWR);
	} else {
		printf("tunnel mode\n");
		afd = open(WMA_DEVICE_NODE, O_WRONLY);
	}
	
	wma_opflg = 0;
	if (afd < 0) {
		printf("Unable to open audio device = %s\n", WMA_DEVICE_NODE);
		ret_val = -1;
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
		ret_val = -1;
		goto err_state;
	}

	if (audio_data->mode)
		config.meta_field = 1;
	config.sample_rate = audio_data->freq;
	config.channel_count = audio_data->channels;
	if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
		perror("could not set config");
		ret_val = -1;
		goto err_state;
	}


	wma_config.armdatareqthr =  audio_data->datareqthr;
	wma_config.channelsdecoded = audio_data->channels;
	wma_config.wmabytespersec = audio_data->bps;
	wma_config.wmasamplingfreq = audio_data->freq;
	wma_config.wmaencoderopts = audio_data->encopt;


	printf("wma_config.armdatareqthr=%d\n",wma_config.armdatareqthr);
	printf("wma_config.channelsdecoded=%d\n",wma_config.channelsdecoded);
	printf("wma_config.wmabytespersec=%d\n",wma_config.wmabytespersec);
	printf("wma_config.wmasamplingfreq=%d\n",wma_config.wmasamplingfreq);
	printf("wma_config.wmaencoderopts=%d\n",wma_config.wmaencoderopts);
	if (ioctl(afd, AUDIO_SET_WMA_CONFIG, &wma_config)) {
		perror("could not set WMA config");
		ret_val = -1;
		goto err_state;
	}

	transcodebuf = (char *)malloc(config.buffer_size);
	if (!transcodebuf) {
		printf("could not allocate memory for store transcoded data\n");
		ret_val = -1;
		goto err_state;
	}
	memset(transcodebuf, 0, config.buffer_size);
	printf("transcodebuf = %d\n",(int) transcodebuf);

	if (!audio_data->mode)
		config.buffer_size =
			(config.buffer_size - sizeof(struct meta_in));

	if (audio_data->mode) {
		/* non - tunnel portion */
		struct msm_audio_pcm_config config_rec;
		printf(" selected non-tunnel part\n");
		append_header.Sample_rate = wma_config.wmasamplingfreq;
		append_header.Number_Channels = wma_config.channelsdecoded;
		append_header.Bytes_Sec = append_header.Sample_rate *
			append_header.Number_Channels * 2;
		append_header.Block_align = append_header.Number_Channels * 2;
		if (ioctl(afd, AUDIO_GET_PCM_CONFIG, &config_rec)) {
			printf("could not get PCM config\n");
			ret_val = -1;
			goto err_state;
		}
		printf(" config_rec.pcm_feedback = %d,\
			config_rec.buffer_count = %d , config_rec.buffer_size=%d \n",\
			config_rec.pcm_feedback, config_rec.buffer_count,
			config_rec.buffer_size);
		config_rec.pcm_feedback = 1;
		audio_data->recsize = config_rec.buffer_size;
		audio_data->recbuf = (char *)malloc(config_rec.buffer_size);
		if (!audio_data->recbuf) {
			printf("could not allocate memory for decoding\n");
			ret_val = -1;
			goto err_state;
		}
		memset(audio_data->recbuf, 0, config_rec.buffer_size);
		if (ioctl(afd, AUDIO_SET_PCM_CONFIG, &config_rec)) {
			printf("could not set PCM config\n");
			ret_val = -1;
			goto err_state;
		}
		pthread_create(&thread, NULL, wma_dec, (void *)audio_data);

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
				memcpy(transcodebuf, &meta, sizeof(struct meta_in));
				read(fd, (transcodebuf + sizeof(struct meta_in)), pkts_size);
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
	close(afd);
	if (transcodebuf)
		free(transcodebuf);
	if (audio_data->recbuf)
		free(audio_data->recbuf);

file_err:
	close(fd);
	return ret_val;
}

/* Get File content and create meta */
static int fill_buffer(void *buf, unsigned sz, void *cookie)
{
        struct meta_in meta;
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) cookie;
        unsigned cpy_size = (sz < audio_data->avail?sz:audio_data->avail);
	#ifdef DEBUG_LOCAL
	char *temp;
	printf("%s:frame count %d\n", __func__, audio_data->frame_count);
	#endif
	if (audio_data->mode) {
	        meta.ntimestamp.HighPart = 0;
	        meta.ntimestamp.LowPart = (unsigned long long)(audio_data->frame_count * 0x10000);
		meta.offset = sizeof(struct meta_in);
	        audio_data->frame_count++;
	#ifdef DEBUG_LOCAL
                printf("Meta In High part is %lu\n",
                                meta.ntimestamp.HighPart);
                printf("Meta In Low part is %lu\n",
                                meta.ntimestamp.LowPart);
                printf("Meta In ntimestamp: %llu\n", (((unsigned long long)
                                        meta.ntimestamp.HighPart << 32) +
                                        meta.ntimestamp.LowPart));
                printf("meta in size %d\n", sizeof(struct meta_in));
	#endif
		if (audio_data->avail == 0) {
			/* End of file, send EOS */
			meta.nflags = EOS;
	                memcpy(buf, &meta, sizeof(struct meta_in));
	                return (sizeof(struct meta_in));
		}
	        meta.nflags = 0;
		memcpy(buf, &meta, sizeof(struct meta_in));
	        memcpy(((char *)buf + sizeof(struct meta_in)), audio_data->next, cpy_size);
		#ifdef DEBUG_LOCAL
		temp = ((char*)buf + sizeof(struct meta_in));
		printf("\nFirst three bytes 0x%2x:0x%2x:0x%2x\n", *temp, *(temp+1), *(temp+2));
		#endif
	} else {
	        if (audio_data->avail == 0) {
	                return 0;
        	}
	        audio_data->frame_count++;
	        memcpy((char *)buf, audio_data->next, cpy_size);
		#ifdef DEBUG_LOCAL
		temp = (buf);
		printf("\nFirst three bytes 0x%2x:0x%2x:0x%2x\n", *temp, *(temp+1), *(temp+2));
		#endif
	}
        audio_data->next += cpy_size;
        audio_data->avail -= cpy_size;
	if (audio_data->mode)
		return cpy_size + sizeof(struct meta_in);
	else
		return cpy_size;
}

/* Expect on raw file, which is  only wma data file */
static void *setup_wma_file(struct audtest_config *clnt_config)
{
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
        struct stat stat_buf;
        char *content_buf;
        int fd;
        size_t buffer_size;

        printf("setup wma file\n");
        fd = open(clnt_config->file_name, O_RDONLY);
        if (fd < 0) {
                printf("Err while opening WMA bin file for decoder \
                        : %s\n", clnt_config->in_file_name);
		return((void *)-1);
        }
        (void) fstat(fd, &stat_buf);
        buffer_size = stat_buf.st_size;
	/* memory set for file */
        audio_data->next = (char*)malloc(buffer_size);
        printf("Total WMA bin file len: %d, buffer start addr=0x%p\n", buffer_size, audio_data->next);

        if (!audio_data->next) {
                fprintf(stderr,"could not allocate %d bytes\n", buffer_size);
                close(fd);
		return ((void*)-1);
        }
        audio_data->org_next = audio_data->next;
        content_buf = audio_data->org_next;

        if ((read(fd, audio_data->next, buffer_size)) != (ssize_t)buffer_size) {
                fprintf(stderr,"could not read %d bytes\n", buffer_size);
                goto fail;
        }
        audio_data->avail = buffer_size;
        audio_data->org_avail = audio_data->avail;
        printf("Total available wma len: %d, buffer start addr=%p\n", audio_data->avail, audio_data->org_next);
	return 0;
fail:
        close(fd);
        free(content_buf);
        return ((void*)-1);
}

static void *wma_read_thread_8660(void *arg)
{
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd;
	int total_len;
	int fd = 0;
	struct dec_meta_out *meta_out_ptr;
	struct meta_out_dsp *meta_out_dsp;
	struct msm_audio_aio_buf aio_buf;
	struct msm_audio_config config;
#ifdef AUDIOV2
	unsigned short dec_id;
#endif
	unsigned int first_frame_offset, idx;
	unsigned int total_frame_size;

	total_len = 0;
	if(file_write) {
		// Log PCM samples to a file
		fd = open(audio_data->outfile, O_RDWR | O_CREAT,
		  S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd < 0) {
			perror("Cannot open audio sink device");
			return ((void*)-1);
		}
		lseek(fd, 44, SEEK_SET);  /* Set Space for Wave Header */
	} else {
		// LOg PCM samples to pcm out driver
		fd = open(audio_data->outfile, O_WRONLY);
		if (fd < 0) {
			perror("Cannot open audio sink device");
			return ((void*)-1);
		}
#ifdef AUDIOV2
		if (ioctl(fd, AUDIO_GET_SESSION_ID, &dec_id)) {
			perror("could not get pcm decoder session id\n");
			goto err_state;
		}
		printf("pcm decoder session id %d\n", dec_id);
#if defined(TARGET_USES_QCOM_MM_AUDIO)
		if (devmgr_register_session(dec_id, DIR_RX) < 0) {
			perror("could not route pcm decoder stream\n");
			goto err_state;
		}
#endif
#endif
		if (ioctl(fd, AUDIO_GET_CONFIG, &config)) {
			perror("could not get pcm config");
			goto err_state;
		}
		config.channel_count = audio_data->channels;
		config.sample_rate = audio_data->freq;
		if (ioctl(fd, AUDIO_SET_CONFIG, &config)) {
			perror("could not set pcm config");
			goto err_state;
		}
		if (ioctl(fd, AUDIO_START, 0) < 0) {
			perror("could not start pcm playback node");
			goto err_state;
		}
	}
	while(1) {
		// Send free Read buffer
		aio_buf.buf_addr = aio_op_buf[out_free_indx].buf_addr;
		aio_buf.buf_len =  aio_op_buf[out_free_indx].buf_len;   
		aio_buf.data_len = 0; // Driver will notify actual size     
		aio_buf.private_data =  aio_op_buf[out_free_indx].private_data;
		wait_for_data();
#ifdef DEBUG_LOCAL
		printf("%s:free_idx %d, data_idx %d\n", __func__, out_free_indx, out_data_indx);
#endif
		out_free_indx = out_data_indx;
		printf("%s:ASYNC_READ addr %p len %d\n", __func__, aio_buf.buf_addr, aio_buf.buf_len);
		if (ioctl(afd, AUDIO_ASYNC_READ, &aio_buf) < 0) {
			printf("error on async read\n");
			break;
		}
		meta_out_ptr = (struct dec_meta_out *)aio_op_buf[out_free_indx].buf_addr;
		meta_out_dsp = (struct meta_out_dsp *)(((char *)meta_out_ptr + sizeof(struct dec_meta_out)));
		printf("nr of frames %d\n", meta_out_ptr->num_of_frames);
#ifdef DEBUG_LOCAL
		printf("%s:msw ts 0x%8x, lsw_ts 0x%8x, nflags 0x%8x\n", __func__,
			meta_out_dsp->msw_ts,
			meta_out_dsp->lsw_ts,
			meta_out_dsp->nflags);
#endif
		first_frame_offset = meta_out_dsp->offset_to_frame + sizeof(struct dec_meta_out);
		total_frame_size = 0;
		if(meta_out_ptr->num_of_frames != 0xFFFFFFFF) {
			// Go over all meta data field to find exact frame size
			for(idx=0; idx < meta_out_ptr->num_of_frames; idx++) { 
				total_frame_size +=  meta_out_dsp->frame_size;
				meta_out_dsp++;
			}
			printf("total size %d\n", total_frame_size);
		} else {
			//OutPut EOS reached
			if (meta_out_dsp->nflags == EOS) {
				printf("%s:Received EOS at output port 0x%8x\n", __func__,
				meta_out_dsp->nflags);
				break;
			}
		}
		printf("%s: Read Size %d offset %d\n", __func__,
			total_frame_size, first_frame_offset);
		write(fd, ((char *)aio_op_buf[out_free_indx].buf_addr + first_frame_offset),
								total_frame_size);
		total_len +=  total_frame_size;
	}
	if(file_write) {
		append_header.Sample_rate = audio_data->freq;
		append_header.Number_Channels = audio_data->channels;
		append_header.Bytes_Sec = append_header.Sample_rate *
			append_header.Number_Channels * 2;
		append_header.Block_align = append_header.Number_Channels * 2;
		create_wav_header(total_len);
		lseek(fd, 0, SEEK_SET);
		write(fd, (char *)&append_header, 44);
	} else {
		sleep(1); // All buffers drained
#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
		if (devmgr_unregister_session(dec_id, DIR_RX) < 0) {
			perror("could not deroute pcm decoder stream\n");
		}
#endif
	}
err_state:
	close(fd);
	printf("%s:exit\n", __func__);
	pthread_exit(NULL);
	return NULL;
}

static void *wma_write_thread_8660(void *arg)
{
	struct msm_audio_aio_buf aio_buf;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd, sz;
	struct meta_in *meta_in_ptr;
	int eos=0;

	while(1) {
		if(!eos) {
			// Copy write buffer
	 		aio_buf.buf_addr = aio_ip_buf[in_free_indx].buf_addr;
			aio_buf.buf_len =  aio_ip_buf[in_free_indx].buf_len;   
			aio_buf.private_data =  aio_ip_buf[in_free_indx].private_data;
			sz = fill_buffer(aio_buf.buf_addr, in_size, audio_data);
			if (sz == sizeof(struct meta_in)) { //NT mode EOS
				printf("%s:Done reading file\n", __func__);
				printf("%s:Send EOS on I/N Put\n", __func__);
				aio_buf.data_len = sz;
				aio_ip_buf[in_free_indx].data_len = sz;
				eos = 1;
			} else if (sz == 0){ // Tunnel mode EOS
				eos = 1;
				break;
			} else {
				aio_buf.data_len = sz;
				aio_ip_buf[in_free_indx].data_len = sz;
			}
			printf("%s:ASYNC_WRITE addr %p len %d\n", __func__, aio_buf.buf_addr,aio_buf.data_len);
			ioctl(afd, AUDIO_ASYNC_WRITE, &aio_buf);
		}
		wait_for_data_consumed();
#ifdef DEBUG_LOCAL
		printf("%s:free_idx %d, data_idx %d\n", __func__, in_free_indx, in_data_indx);
#endif
		in_free_indx = in_data_indx;
		meta_in_ptr = (struct meta_in *)aio_ip_buf[in_data_indx].buf_addr;
		//Input EOS reached
		if (meta_in_ptr->nflags == EOS) {
			printf("%s:Received EOS buffer back at i/p 0x%8x\n", __func__, meta_in_ptr->nflags);
			break;
		}
	}
	if(!audio_data->mode && eos) {
		printf("%s:Wait for data to drain out\n", __func__); 
		fsync(afd);
		eos_ack = 1;
		sleep(1);
		ioctl(afd, AUDIO_ABORT_GET_EVENT, 0);
	}
	printf("%s:exit\n", __func__);
	// Free memory created during wma bin file 
        free(audio_data->org_next);
	pthread_exit(NULL);
	return NULL;
}

static void *wma_dec_event_8660(void *arg)
{
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd, rc;
	struct msm_audio_event event;
	int eof = 0;
	struct dec_meta_out *meta_out_ptr;
	struct meta_out_dsp *meta_out_dsp;
	struct meta_in *meta_in_ptr;
	pthread_t evt_read_thread;
	pthread_t evt_write_thread;

	eos_ack = 0;
	if (audio_data->mode) // Non Tunnel mode
		pthread_create(&evt_read_thread, NULL, wma_read_thread_8660, (void *) audio_data);
	pthread_create(&evt_write_thread, NULL, wma_write_thread_8660, (void *) audio_data);
	// Till EOF not reached in NT or till eos not reached in tunnel
	while((!eof && audio_data->mode) || (!eos_ack && !audio_data->mode)) {
		// Wait till timeout
		event.timeout_ms = 0;
		rc = ioctl(afd, AUDIO_GET_EVENT, &event);
		if (rc < 0) {
	  		printf("%s: errno #%d", __func__, errno);
	  		continue;
		}
#ifdef DEBUG_LOCAL
		printf("%s:AUDIO_GET_EVENT event %d \n", __func__, event.event_type);
#endif
		switch(event.event_type) {
			case AUDIO_EVENT_READ_DONE:
				if(event.event_payload.aio_buf.buf_len == 0)
					printf("Warning buf_len Zero\n");
				if (event.event_payload.aio_buf.data_len >= sizeof(struct dec_meta_out)) {
		  			printf("%s: READ_DONE: addr %p len %d\n", __func__,
						event.event_payload.aio_buf.buf_addr,
						event.event_payload.aio_buf.data_len);
					meta_out_ptr = (struct dec_meta_out *)event.event_payload.aio_buf.buf_addr;
					out_data_indx =(int) event.event_payload.aio_buf.private_data;
					meta_out_dsp = (struct meta_out_dsp *)(((char *)meta_out_ptr + sizeof(struct dec_meta_out)));
					//OutPut EOS reached
					if (meta_out_dsp->nflags == EOS) {
			  			eof = 1;
						printf("%s:Received EOS event at output 0x%8x\n", __func__,
						meta_out_dsp->nflags);
					}
					data_available();
				} else {
					printf("%s:AUDIO_EVENT_READ_DONE:unexpected length\n", __func__);
				}
		 		break;
			case AUDIO_EVENT_WRITE_DONE:
				if (event.event_payload.aio_buf.data_len >= sizeof(struct meta_in)) {
					printf("%s:WRITE_DONE: addr %p len %d\n", __func__,
						event.event_payload.aio_buf.buf_addr,
						event.event_payload.aio_buf.data_len);
					meta_in_ptr = (struct meta_in *)event.event_payload.aio_buf.buf_addr;
					in_data_indx =(int) event.event_payload.aio_buf.private_data;
					//Input EOS reached
					if (meta_in_ptr->nflags == EOS) {
						printf("%s:Received EOS at input 0x%8x\n", __func__, meta_in_ptr->nflags);
					}
					data_consumed();
				} else {
					printf("%s:AUDIO_EVENT_WRITE_DONE:unexpected length\n", __func__);
				}
				break;
			default:
				printf("%s: -Unknown event- %d\n", __func__, event.event_type);
				break;
		}
	}
	if(audio_data->mode)
		pthread_join(evt_read_thread, NULL);
	else
		pthread_join(evt_write_thread, NULL);
	printf("%s:exit\n", __func__);
	pthread_exit(NULL);
	return NULL;
}

static int wma_start_8660(struct audtest_config *clnt_config)
{
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
	unsigned n = 0;
	pthread_t evt_thread;
	int sz;
	int rc = -1;
#ifdef AUDIOV2
	int dec_id;
#endif
	int afd, ipmem_fd[WMATEST_NUM_IBUF], opmem_fd[WMATEST_NUM_OBUF];
	void *ipmem_ptr[WMATEST_NUM_IBUF], *opmem_ptr[WMATEST_NUM_OBUF];
	struct msm_audio_pmem_info pmem_info;
	struct msm_audio_aio_buf aio_buf;
	struct msm_audio_buf_cfg buf_cfg;
	struct msm_audio_config config;
	unsigned int open_flags;

	memset(ipmem_fd, 0, (sizeof(int) * WMATEST_NUM_IBUF));
	memset(opmem_fd, 0, (sizeof(int) * WMATEST_NUM_OBUF));
	memset(ipmem_ptr, 0, (sizeof(void *) * WMATEST_NUM_IBUF));
	memset(opmem_ptr, 0, (sizeof(void *) * WMATEST_NUM_OBUF));

	if(((in_size + sizeof(struct meta_in)) > WMATEST_IBUFSZ) ||
		(out_size > WMATEST_OBUFSZ)) {
			perror("configured input / output size more"\
			"than pmem allocation");
			return -1; 
	}

	if (audio_data->mode)
		open_flags = O_RDWR | O_NONBLOCK;
	else
		open_flags = O_WRONLY | O_NONBLOCK;

	if (audio_data->formattag == 0x161)
		afd = open("/dev/msm_wma", open_flags);
	else
		afd = open("/dev/msm_wmapro", open_flags);

	if (afd < 0) {
		perror("Cannot open WMA device");
		return -1;
	}

	audio_data->afd = afd; /* Store */

	setup_wma_file(clnt_config);
	if (audio_data->mode) {
		/* PCM config */
		if (ioctl(afd, AUDIO_GET_CONFIG, &config)) {
			perror("could not get config");
			goto err_state1;
		}
		config.sample_rate = audio_data->freq;
		config.channel_count = audio_data->channels;
		config.bits = audio_data->bitspersample;

		if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
			perror("could not set config");
			goto err_state1;
		}
		printf("pcm config sample_rate=%d channels=%d bitspersample=%d \n",
			config.sample_rate, config.channel_count, config.bits);
	} else {
#ifdef AUDIOV2
		if (ioctl(afd, AUDIO_GET_SESSION_ID, &dec_id)) {
			perror("could not get decoder session id\n");
			goto err_state1;
		}
#if defined(TARGET_USES_QCOM_MM_AUDIO)
		if (devmgr_register_session(dec_id, DIR_RX) < 0) {
			goto err_state1;
		}
#endif
#endif
	}
	audio_data->frame_count	= 0;
	if(audio_data->formattag == 0x161) {
		struct msm_audio_wma_config_v2 wma_config;
		wma_config.numchannels = audio_data->channels;
		wma_config.avgbytespersecond = audio_data->bps/8;
		wma_config.samplingrate = audio_data->freq;
		wma_config.encodeopt = audio_data->encopt;
		wma_config.format_tag = audio_data->formattag;
		wma_config.channelmask =  audio_data->channelmask;
		wma_config.validbitspersample = audio_data->bitspersample;
		wma_config.block_align = audio_data->asfpacketlength;
		if (ioctl(afd, AUDIO_SET_WMA_CONFIG_V2, &wma_config)) {
			perror("could not set AUDIO_SET_WMA_CONFIG_V2");
			goto err_state2;
		}
	} else {
		struct msm_audio_wmapro_config wmapro_config;
		wmapro_config.numchannels = audio_data->channels;
		wmapro_config.avgbytespersecond = audio_data->bps/8;
		wmapro_config.samplingrate = audio_data->freq;
		wmapro_config.encodeopt = audio_data->encopt;
		wmapro_config.formattag = audio_data->formattag;
		wmapro_config.channelmask =  audio_data->channelmask;
		wmapro_config.validbitspersample = audio_data->bitspersample;
		wmapro_config.asfpacketlength = audio_data->asfpacketlength;
		wmapro_config.advancedencodeopt = audio_data->advancedencodeopt;
		wmapro_config.advancedencodeopt2 =  audio_data->advancedencodeopt2;
		if (ioctl(afd, AUDIO_SET_WMAPRO_CONFIG, &wmapro_config)) {
			perror("could not set AUDIO_SET_WMAPRO_CONFIG");
			goto err_state2;
		}
	}
	if(ioctl(afd, AUDIO_GET_BUF_CFG, &buf_cfg)) {
		printf("Error getting AUDIO_GET_BUF_CONFIG\n");
		goto err_state2;
	}
	printf("Default meta_info_enable = 0x%8x\n", buf_cfg.meta_info_enable);
	printf("Default frames_per_buf = 0x%8x\n", buf_cfg.frames_per_buf);
	if (audio_data->mode) {
		// NT mode support meta info
		buf_cfg.meta_info_enable = 1;
		if(ioctl(afd, AUDIO_SET_BUF_CFG, &buf_cfg)) {
			printf("Error setting AUDIO_SET_BUF_CONFIG\n");
			goto err_state2;
		}
	}
	pthread_cond_init(&avail_cond, 0);
	pthread_mutex_init(&avail_lock, 0);
	pthread_cond_init(&consumed_cond, 0);
	pthread_mutex_init(&consumed_lock, 0);
	data_is_available = 0;
	data_is_consumed = 0;
	in_free_indx=0;
	out_free_indx=0;
	if ((ioctl(afd, AUDIO_START, 0))< 0 ) {
	   		printf("wmatest: unable to start driver\n");
			goto err_state2;
	}
	if (audio_data->mode) {
		/* non - tunnel portion */
		printf(" selected non-tunnel part\n");
		// Register read buffers
		for (n = 0; n < WMATEST_NUM_OBUF; n++) {
			opmem_fd[n] = open("/dev/pmem_audio", O_RDWR);
			printf("%s: opmem_fd %x\n",  __func__, opmem_fd[n]);
			opmem_ptr[n] = mmap(0, WMATEST_OBUFSZ,
				PROT_READ | PROT_WRITE, MAP_SHARED, opmem_fd[n], 0);
			printf("%s:opmem_ptr[%d] %x\n", __func__, n, (unsigned int) opmem_ptr[n]);
			pmem_info.fd = opmem_fd[n];
			pmem_info.vaddr = opmem_ptr[n];
			rc = ioctl(afd, AUDIO_REGISTER_PMEM, &pmem_info);
			if(rc < 0) {
                                printf( "error on register opmem=%d\n",rc);
				goto err_state2;
                        }
			// Read buffers local structure
		 	aio_op_buf[n].buf_addr = opmem_ptr[n];
			aio_op_buf[n].buf_len = out_size + sizeof(struct dec_meta_out); 
			aio_op_buf[n].data_len = 0; // Driver will notify actual size 
			aio_op_buf[n].private_data = (void *)n; //Index
		}
		// Send n-1 Read buffer
		for (n = 0; n < (WMATEST_NUM_OBUF-1); n++) {
		 	aio_buf.buf_addr = aio_op_buf[n].buf_addr;
			aio_buf.buf_len = aio_op_buf[n].buf_len;
			aio_buf.data_len = aio_op_buf[n].data_len; 
			aio_buf.private_data = aio_op_buf[n].private_data;
			printf("ASYNC_READ addr %p len %d\n", aio_buf.buf_addr,
				aio_buf.buf_len);
			if (ioctl(afd, AUDIO_ASYNC_READ, &aio_buf) < 0) {
				printf("error on async read\n");
				goto err_state2;
			}
		}
		//Indicate available free buffer as (n-1)
		out_free_indx = WMATEST_NUM_OBUF-1;
	}
	//Register Write  buffer
	for (n = 0; n < WMATEST_NUM_IBUF; n++) {
		ipmem_fd[n] = open("/dev/pmem_audio", O_RDWR);
		printf("%s: ipmem_fd %x\n",  __func__, ipmem_fd[n]);
		ipmem_ptr[n] = mmap(0, WMATEST_IBUFSZ,
			PROT_READ | PROT_WRITE, MAP_SHARED, ipmem_fd[n], 0);
		printf("%s:ipmem_ptr[%d] %x\n", __func__, n, (unsigned int )ipmem_ptr[n]);
		pmem_info.fd = ipmem_fd[n];
		pmem_info.vaddr = ipmem_ptr[n];
		rc = ioctl(afd, AUDIO_REGISTER_PMEM, &pmem_info);
		if(rc < 0) {
                        printf( "error on register ipmem=%d\n",rc);
			goto err_state2;
                }
		// Write buffers local structure
	 	aio_ip_buf[n].buf_addr = ipmem_ptr[n];
		aio_ip_buf[n].buf_len = WMATEST_IBUFSZ;
		aio_ip_buf[n].data_len = 0; // Driver will notify actual size 
		aio_ip_buf[n].private_data = (void *)n; //Index
	}
	// Send n-1 write buffer
	for (n = 0; n < (WMATEST_NUM_IBUF-1); n++) {
	 	aio_buf.buf_addr = aio_ip_buf[n].buf_addr;
		aio_buf.buf_len = aio_ip_buf[n].buf_len;
		if ((sz = fill_buffer(aio_buf.buf_addr, in_size, audio_data)) < 0)
			goto err_state2;
		aio_buf.data_len = sz;
		aio_ip_buf[n].data_len = sz; 
		aio_buf.private_data = aio_ip_buf[n].private_data;
		printf("ASYNC_WRITE addr %p len %d\n", aio_buf.buf_addr,
			aio_buf.data_len);
		rc = ioctl(afd, AUDIO_ASYNC_WRITE, &aio_buf);
		if(rc < 0) {
			printf( "error on async write=%d\n",rc);
			goto err_state2;
		}
	}
	//Indicate available free buffer as (n-1)
	in_free_indx = WMATEST_NUM_IBUF-1;
	pthread_create(&evt_thread, NULL, wma_dec_event_8660, (void *) audio_data);
	pthread_join(evt_thread, NULL);
	printf("AUDIO_STOP as event thread completed\n");
done:
	rc = 0;
	ioctl(afd, AUDIO_STOP, 0);
err_state2:
	if (audio_data->mode) {
		for (n = 0; n < WMATEST_NUM_OBUF; n++) {
			munmap(opmem_ptr[n], WMATEST_OBUFSZ);
			close(opmem_fd[n]);
		}
	}
	for (n = 0; n < WMATEST_NUM_IBUF; n++) {
		munmap(ipmem_ptr[n], WMATEST_IBUFSZ);
		close(ipmem_fd[n]);
	}
	if (!audio_data->mode) {
#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
		if (devmgr_unregister_session(dec_id, DIR_RX) < 0)
			printf("error closing stream\n");
#endif
	}
err_state1:
	close(afd);
	return rc;
}

void *wma_thread(void *arg)
{
	struct audiotest_thread_context *context =
	    (struct audiotest_thread_context *)arg;
	int ret_val;

 	if (context->config.tgt == 0x07)
		ret_val = wma_start(&context->config);
	else
		ret_val = wma_start_8660(&context->config);

	printf(" Free audio instance 0x%8x \n", (unsigned int) context->config.private_data);
	free(context->config.private_data);
	free_context(context);
	pthread_exit((void *)ret_val);
	return NULL;
}

int wmaplay_read_params(void)
{
	struct audiotest_thread_context *context;
	char *token = NULL;
	int ret_val = 0;

	context = get_free_context();
	if (context == NULL) {
		ret_val = -1;
	} else {
                struct audio_pvt_data *audio_data;
                audio_data = (struct audio_pvt_data *) malloc(sizeof(struct audio_pvt_data));
                if(!audio_data) {
                        printf("error allocating audio instance structure \n");
                        free_context(context);
                        ret_val = -1;
                } else {
                        printf("Created audio instance 0x%8x wma\n",(unsigned int) audio_data);
                        memset(audio_data, 0, sizeof(struct audio_pvt_data)); /* Set complete zero */
			context->config.file_name = "/data/sample.strm";
			context->type = AUDIOTEST_TEST_MOD_WMA_DEC;
			#ifdef _ANDROID_
			audio_data->outfile = "/data/pcm.wav";
			#else
			audio_data->outfile = "/tmp/pcm.wav";
			#endif
			audio_data->mode = 0;
			audio_data->repeat = 0;
			audio_data->quit = 0;
			context->config.tgt = 0x07;
			token = strtok(NULL, " ");
			out_size = 8192 + sizeof(struct dec_meta_out);
			in_size = 8192;
			file_write = 1;

			while (token != NULL) {
				if (!memcmp(token, "-id=", (sizeof("-id=") - 1))) {
					context->cxt_id =
					    atoi(&token[sizeof("-id=") - 1]);
					printf("Context id =%d\n", context->cxt_id);
				} else if (!memcmp(token, "-mode=",
						   (sizeof("-mode=") - 1))) {
					audio_data->mode = atoi(&token[sizeof("-mode=") - 1]);
				} else if (!memcmp(token, "-out=",
						   (sizeof("-out=") - 1))) {
					audio_data->outfile = token + (sizeof("-out=") - 1);
				} else if (!memcmp(token, "-datareqthr=",
						   (sizeof("-datareqthr=") - 1))) {
					audio_data->datareqthr = atoi(&token[sizeof("-datareqthr=") - 1]);
				} else if (!memcmp(token, "-channels=",
						   (sizeof("-channels=") - 1))) {
					audio_data->channels = atoi(&token[sizeof("-channels=") - 1]);
				} else if (!memcmp(token, "-bps=",
						   (sizeof("-bps=") - 1))) {
					audio_data->bps = atoi(&token[sizeof("-bps=") - 1]);
				} else if (!memcmp(token, "-freq=",
						   (sizeof("-freq=") - 1))) {
					audio_data->freq = atoi(&token[sizeof("-freq=") - 1]);
				} else if (!memcmp(token, "-encopt=",
						   (sizeof("-encopt=") - 1))) {
					audio_data->encopt = atoi(&token[sizeof("-encopt=") - 1]);
				} else if (!memcmp(token, "-ftag=",
						   (sizeof("-ftag=") - 1))) {
					audio_data->formattag = atoi(&token[sizeof("-ftag=") - 1]);
				} else if (!memcmp(token, "-cmask=",
						   (sizeof("-cmask=") - 1))) {
					audio_data->channelmask = atoi(&token[sizeof("-cmask=") - 1]);
				} else if (!memcmp(token, "-bsample=",
						   (sizeof("-bsample=") - 1))) {
					audio_data->bitspersample = atoi(&token[sizeof("-bsample=") - 1]);
				} else if (!memcmp(token, "-balign=",
						   (sizeof("-balign=") - 1))) {
					audio_data->asfpacketlength = atoi(&token[sizeof("-balign=") - 1]);
				} else if (!memcmp(token, "-advopt2=",
						   (sizeof("-advopt2=") - 1))) {
					audio_data->advancedencodeopt2 = atoi(&token[sizeof("-advopt2=") - 1]);
				} else if (!memcmp(token, "-advopt=",
						   (sizeof("-advopt=") - 1))) {
					audio_data->advancedencodeopt = atoi(&token[sizeof("-advopt=") - 1]);
				} else if (!memcmp(token, "-outsize=",
						   (sizeof("-outsize=") - 1))) {
					out_size = atoi(&token[sizeof("-outsize=") - 1]) + sizeof(struct dec_meta_out);
				} else if (!memcmp(token, "-insize=",
						   (sizeof("-insize=") - 1))) {
					in_size = atoi(&token[sizeof("-insize=") - 1]);
				} else if (!memcmp(token, "-tgt=",
						   (sizeof("-tgt=") - 1))) {
					context->config.tgt = atoi(&token[sizeof("-tgt=") - 1]);
				} else if (!memcmp(token, "-wr=",
						   (sizeof("-wr=") - 1))) {
					file_write = atoi(&token[sizeof("-wr=") - 1]);
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
			printf("reading parameters tgt %d\n", context->config.tgt);
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create(&context->thread, NULL,
				       wma_thread, (void *)context);
		}

	}
	return ret_val;

}

int wma_play_control_handler(void *private_data)
{
	int drvfd, ret_val = 0;
	int volume;	/* Default volume */
	char *token;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) private_data;

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

const char *wmaplay_help_txt = "Play WMA stream file: \n\
echo \"playwma path_of_file -id=xxx -mode=x -datareqthr=x -channels=x -bps=x -freq=x -encopt=x -ftag=x\
-cmask=x -bsample=x -balign=x -advopt=x -advopt2=x -outsize=x -insize=x -tgt= x -out=path_of_outfile -repeat=x\" > %s \n\
Codec type of WMA file\n\
mode= 0(tunnel mode) or 1 (non-tunnel mode) \n\
tgt: 08 - for 8660 target, default 7k target \n \
ftag: format tag \n\
cmask: channel mask \n\
bsample: bit per sample \n\
balign: block align \n\
outsize: size of pcm buffer \n\
insize: size for sending bitstrem \n\
Repeat 'x' no. of times, repeat infinitely if repeat = 0\n\
Supported control command: pause, resume, volume, flush, quit \n\
examples: \n\
echo \"playwma path_of_file -id=xxx -mode=<0 or 1>\" > %s \n\
echo \"control_cmd -id=xxx -cmd=pause\" > %s \n\
echo \"control_cmd -id=xxx -cmd=resume\" > %s \n\
echo \"control_cmd -id=xxx -cmd=flush\" > %s \n\
echo \"control_cmd -id=xxx -cmd=volume -value=yyyy\" > %s \n";

void wmaplay_help_menu(void)
{
	printf(wmaplay_help_txt, cmdfile, cmdfile, cmdfile,
			cmdfile, cmdfile, cmdfile);
}

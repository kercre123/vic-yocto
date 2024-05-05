/* amrnbtest.c - native AMRNB test application
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
#include "audiotest_def.h"
#include <sys/ioctl.h>
#include <linux/msm_audio_amrnb.h>
#include <pthread.h>
#include <errno.h>

#define AMRNB_PKT_SIZE 36
#define LOCAL_DEBUG

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
	{'f', 'm', 't', ' '}, 16, 1, 1, 8000, 16000, 2,
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

struct meta_in{
	unsigned short offset;
	long long timestamp;
	unsigned int nflags;
} __attribute__ ((packed));

typedef struct TIMESTAMP{
	unsigned long LowPart;
	unsigned long HighPart;
} __attribute__ ((packed)) TIMESTAMP;

struct meta_in_q6{
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
	long long timestamp;
	unsigned int nflags;
	unsigned short errflag;
	unsigned short sample_frequency;
	unsigned short channel;
	unsigned int tick_count;
} __attribute__ ((packed));

#define AMRNBTEST_IBUFSZ (32*1024)
#define AMRNBTEST_NUM_IBUF 2
#define AMRNBTEST_IPMEM_SZ (AMRNBTEST_IBUFSZ * AMRNBTEST_NUM_IBUF)

#define AMRNBTEST_OBUFSZ (32*1024)
#define AMRNBTEST_NUM_OBUF 2
#define AMRNBTEST_OPMEM_SZ (AMRNBTEST_OBUFSZ * AMRNBTEST_NUM_OBUF)

#ifdef _ANDROID_
static const char *cmdfile = "/data/audio_test";
/* static const char *outfile = "/data/pcm.wav"; */
#else
static const char *cmdfile = "/tmp/audio_test";
/* static const char *outfile = "/tmp/pcm.wav"; */
#endif

struct msm_audio_aio_buf aio_ip_buf[AMRNBTEST_NUM_IBUF];
struct msm_audio_aio_buf aio_op_buf[AMRNBTEST_NUM_OBUF];

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
}


static void *amrnb_dec(void *arg)
{
	struct meta_out meta;
	int fd, ret_val = 0;
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
        int afd = audio_data->afd;
	int len, total_len;
	len = 0;
	total_len = 0;

	fd = open(audio_data->outfile, O_RDWR | O_CREAT,
		  S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd < 0) {
		printf("Err while opening file decoder output file \n");
		pthread_exit((void *)ret_val);
	}

	printf(" amrnb_read Thread \n");

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
		printf(" Read = %d PCM samples\n", len/2);
		#endif
		if (len < 0) {
			printf("error reading the PCM samples \n");
			goto fail;
		} else if (len != 0) {
			memcpy(&meta, audio_data->recbuf, sizeof(struct meta_out));
			#ifdef DEBUG_LOCAL
			printf("\t\tMeta Out Timestamp:%lld\n", meta.timestamp);
			#endif
			if (meta.nflags == 1) {
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

static int initiate_play(struct audtest_config *clnt_config,
						 int (*fill)(void *buf, unsigned sz, void *cookie),
						 void *cookie)
{
	struct msm_audio_config config;
	// struct msm_audio_stats stats;
	unsigned n;
	pthread_t thread, event_th;
	int sz, used =0;
	char *buf; 
	int afd;
	int cntW=0;
	int ret = 0;

#ifdef AUDIOV2
	unsigned short dec_id;
#endif

	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;

	if (audio_data->mode) {
		printf("non-tunnel mode\n");
		afd = open("/dev/msm_amrnb", O_RDWR);
	} else {
		printf("tunnel mode\n");
		afd = open("/dev/msm_amrnb", O_WRONLY);

	}
	if (afd < 0) {
		perror("amrnb_play: cannot open AMRNB device");
		return -1;
	}

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

	audio_data->afd = afd; /* Store */

	pthread_create(&event_th, NULL, event_notify, (void *) audio_data);

	if (ioctl(afd, AUDIO_GET_CONFIG, &config)) {
		perror("could not get config");
		ret = -1;
		goto err_state;
	}

	if (audio_data->mode) {
		config.meta_field = 1;
		if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
			perror("could not set config");
			ret = -1;
			goto err_state;
		}
	}

	buf = (char*) malloc(sizeof(char) * config.buffer_size);
	if (buf == NULL) {
		perror("fail to allocate buffer\n");
		ret = -1;
		goto err_state;
	}

	config.buffer_size -= (config.buffer_size % AMRNB_PKT_SIZE);

	printf("initiate_play: buffer_size=%d, buffer_count=%d\n",
			config.buffer_size, config.buffer_count);

	fprintf(stderr,"prefill\n");

	if (audio_data->mode) {
		/* non - tunnel portion */
		struct msm_audio_pcm_config config_rec;
		printf(" selected non-tunnel part\n");
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
			free(buf);
			ret = -1;
			goto err_state;
		}
		pthread_create(&thread, NULL, amrnb_dec, (void *) audio_data);

	}

	for (n = 0; n < config.buffer_count; n++) {
		if ((sz = fill(buf, config.buffer_size, cookie)) < 0)
			break;
		if (write(afd, buf, sz) != sz)
			break;
	}
	cntW=cntW+config.buffer_count; 

	sz = 0;
	fprintf(stderr,"start playback\n");
	if (ioctl(afd, AUDIO_START, 0) >= 0) {
		for (;;) {
#if 0
			if (ioctl(afd, AUDIO_GET_STATS, &stats) == 0)
				fprintf(stderr,"%10d\n", stats.out_bytes);
#endif
			if (sz == 0) {
				if (((sz = fill(buf, config.buffer_size,
					cookie)) < 0) || (audio_data->quit == 1)) {
					if ((audio_data->repeat == 0) || (audio_data->quit == 1)) {
						printf(" file reached end or quit cmd issued, exit loop \n");
						if (audio_data->mode) {
							struct meta_in meta;
							meta.offset =
								sizeof(struct meta_in);
							meta.timestamp =
								(audio_data->frame_count * 20000);
							meta.nflags = 1;
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
							if (fsync(afd) < 0)
								printf("fsync \
								failed\n");
						}
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
			} else
				printf("amrnb_play: continue with unconsumed data\n");

			if (audio_data->suspend == 1) {
				printf("enter suspend mode\n");
				ioctl(afd, AUDIO_STOP, 0);
				while (audio_data->suspend == 1)
					sleep(1);
				ioctl(afd, AUDIO_START, 0);
				printf("exit suspend mode\n");
			}
			used = write(afd, buf, sz);
			printf(" amrnb_play: instance=%d repeat_cont=%d cntW=%d\n",
					(int) audio_data, audio_data->repeat, cntW);
			if (used > -1) {
				sz-=used;
				cntW++;
			} else {
				printf("amrnb_play: IO busy err#%d wait 5 ms\n", errno);
				sz = 0;
				usleep(5000);
			}
		}
		printf("end of amrnb play, stop audio\n");
		sleep(3);
		ioctl(afd, AUDIO_ABORT_GET_EVENT, 0);
		ioctl(afd, AUDIO_STOP, 0);
		sleep(10);

	} else {
		printf("amrnb_play: Unable to start driver\n");
	}
	free(buf);
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
	unsigned cpy_size = (sz < audio_data->avail?sz:audio_data->avail);

	if (audio_data->avail == 0) {
		return -1;
	}

	if (audio_data->mode) {
		meta.timestamp = (audio_data->frame_count * 20000);
		meta.offset = sizeof(struct meta_in);
		meta.nflags = 0;
		audio_data->frame_count += (cpy_size / AMRNB_PKT_SIZE);
		#ifdef DEBUG_LOCAL
		printf("Meta In timestamp: %lld\n", meta.timestamp);
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

/* Get File content and create meta */
static int fill_buffer_8660(void *buf, unsigned sz, void *cookie)
{
        struct meta_in_q6 meta;
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) cookie;
        unsigned cpy_size = (sz < audio_data->avail?sz:audio_data->avail);
	#ifdef DEBUG_LOCAL
	char *temp;
	printf("%s:frame count %d\n", __func__, audio_data->frame_count);
	#endif
	if (audio_data->mode) {
	        meta.ntimestamp.HighPart = 0;
	        meta.ntimestamp.LowPart = (unsigned long long)(audio_data->frame_count * 0x10000);
		meta.offset = sizeof(struct meta_in_q6);
	        audio_data->frame_count++;
	#ifdef DEBUG_LOCAL
                printf("Meta In High part is %lu\n",
                                meta.ntimestamp.HighPart);
                printf("Meta In Low part is %lu\n",
                                meta.ntimestamp.LowPart);
                printf("Meta In ntimestamp: %llu\n", (((unsigned long long)
                                        meta.ntimestamp.HighPart << 32) +
                                        meta.ntimestamp.LowPart));
                printf("meta in size %d\n", sizeof(struct meta_in_q6));
	#endif
		if (audio_data->avail == 0) {
			/* End of file, send EOS */
			meta.nflags = EOS;
	                memcpy(buf, &meta, sizeof(struct meta_in_q6));
	                return (sizeof(struct meta_in_q6));
		}
	        meta.nflags = 0;
		memcpy(buf, &meta, sizeof(struct meta_in_q6));
	        memcpy(((char *)buf + sizeof(struct meta_in_q6)), audio_data->next, cpy_size);
		#ifdef DEBUG_LOCAL
		temp = ((char*)buf + sizeof(struct meta_in_q6));
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
		return cpy_size + sizeof(struct meta_in_q6);
	else
		return cpy_size;
}

static void *amrnb_read_thread_8660(void *arg)
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
		// Log PCM samples to pcm out driver
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

static void *amrnb_write_thread_8660(void *arg)
{
	struct msm_audio_aio_buf aio_buf;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd, sz;
	struct meta_in_q6 *meta_in_ptr;
	int eos=0;

	while(1) {
		if(!eos) {
			// Copy write buffer
	 		aio_buf.buf_addr = aio_ip_buf[in_free_indx].buf_addr;
			aio_buf.buf_len =  aio_ip_buf[in_free_indx].buf_len;   
			aio_buf.private_data =  aio_ip_buf[in_free_indx].private_data;
			sz = fill_buffer_8660(aio_buf.buf_addr, in_size, audio_data);
			if (sz == sizeof(struct meta_in_q6)) { //NT mode EOS
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
		meta_in_ptr = (struct meta_in_q6 *)aio_ip_buf[in_data_indx].buf_addr;
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
	// Free memory done as part of initiate play 
	pthread_exit(NULL);
	return NULL;
}

static void *amrnb_dec_event_8660(void *arg)
{
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd, rc;
	struct msm_audio_event event;
	int eof = 0;
	struct dec_meta_out *meta_out_ptr;
	struct meta_out_dsp *meta_out_dsp;
	struct meta_in_q6 *meta_in_ptr;
	pthread_t evt_read_thread;
	pthread_t evt_write_thread;

	eos_ack = 0;
	if (audio_data->mode) // Non Tunnel mode
		pthread_create(&evt_read_thread, NULL, amrnb_read_thread_8660, (void *) audio_data);
	pthread_create(&evt_write_thread, NULL, amrnb_write_thread_8660, (void *) audio_data);
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
				if (event.event_payload.aio_buf.data_len >= sizeof(struct meta_in_q6)) {
					printf("%s:WRITE_DONE: addr %p len %d\n", __func__,
						event.event_payload.aio_buf.buf_addr,
						event.event_payload.aio_buf.data_len);
					meta_in_ptr = (struct meta_in_q6 *)event.event_payload.aio_buf.buf_addr;
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

static int initiate_play_8660(struct audtest_config *clnt_config)
{
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
	unsigned n = 0;
	pthread_t evt_thread;
	int sz;
	int rc = -1;
#ifdef AUDIOV2
	int dec_id;
#endif
	int afd, ipmem_fd[AMRNBTEST_NUM_IBUF], opmem_fd[AMRNBTEST_NUM_OBUF];
	void *ipmem_ptr[AMRNBTEST_NUM_IBUF], *opmem_ptr[AMRNBTEST_NUM_OBUF];
	struct msm_audio_pmem_info pmem_info;
	struct msm_audio_aio_buf aio_buf;
	struct msm_audio_buf_cfg buf_cfg;
	struct msm_audio_config config;
	unsigned int open_flags;

        audio_data->freq = 8000;
        audio_data->channels = 1;
        audio_data->bitspersample = 16;
	memset(ipmem_fd, 0, (sizeof(int) * AMRNBTEST_NUM_IBUF));
	memset(opmem_fd, 0, (sizeof(int) * AMRNBTEST_NUM_OBUF));
	memset(ipmem_ptr, 0, (sizeof(void *) * AMRNBTEST_NUM_IBUF));
	memset(opmem_ptr, 0, (sizeof(void *) * AMRNBTEST_NUM_OBUF));

	if(((in_size + sizeof(struct meta_in_q6)) > AMRNBTEST_IBUFSZ) ||
		(out_size > AMRNBTEST_OBUFSZ)) {
			perror("configured input / output size more"\
			"than pmem allocation");
			return -1; 
	}

	if (audio_data->mode)
		open_flags = O_RDWR | O_NONBLOCK;
	else
		open_flags = O_WRONLY | O_NONBLOCK;
	afd = open("/dev/msm_amrnb", open_flags);

	if (afd < 0) {
		perror("Cannot open AMRNB device");
		return -1;
	}

	audio_data->afd = afd; /* Store */

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
		printf("amrnbtest: unable to start driver\n");
		goto err_state2;
	}
	if (audio_data->mode) {
		/* non - tunnel portion */
		printf("selected non-tunnel part\n");
		// Register read buffers
		for (n = 0; n < AMRNBTEST_NUM_OBUF; n++) {
			opmem_fd[n] = open("/dev/pmem_audio", O_RDWR);
			printf("%s: opmem_fd %x\n",  __func__, opmem_fd[n]);
			opmem_ptr[n] = mmap(0, AMRNBTEST_OBUFSZ,
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
		for (n = 0; n < (AMRNBTEST_NUM_OBUF-1); n++) {
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
		out_free_indx = AMRNBTEST_NUM_OBUF-1;
	}
	//Register Write  buffer
	for (n = 0; n < AMRNBTEST_NUM_IBUF; n++) {
		ipmem_fd[n] = open("/dev/pmem_audio", O_RDWR);
		printf("%s: ipmem_fd %x\n",  __func__, ipmem_fd[n]);
		ipmem_ptr[n] = mmap(0, AMRNBTEST_IBUFSZ,
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
		aio_ip_buf[n].buf_len = AMRNBTEST_IBUFSZ;
		aio_ip_buf[n].data_len = 0; // Driver will notify actual size 
		aio_ip_buf[n].private_data = (void *)n; //Index
	}
	// Send n-1 write buffer
	for (n = 0; n < (AMRNBTEST_NUM_IBUF-1); n++) {
	 	aio_buf.buf_addr = aio_ip_buf[n].buf_addr;
		aio_buf.buf_len = aio_ip_buf[n].buf_len;
		if ((sz = fill_buffer_8660(aio_buf.buf_addr, in_size, audio_data)) < 0)
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
	in_free_indx = AMRNBTEST_NUM_IBUF-1;
	pthread_create(&evt_thread, NULL, amrnb_dec_event_8660, (void *) audio_data);
	pthread_join(evt_thread, NULL);
	printf("AUDIO_STOP as event thread completed\n");
done:
	rc = 0;
	ioctl(afd, AUDIO_STOP, 0);
err_state2:
	if (audio_data->mode) {
		for (n = 0; n < AMRNBTEST_NUM_OBUF; n++) {
			munmap(opmem_ptr[n], AMRNBTEST_OBUFSZ);
			close(opmem_fd[n]);
		}
	}
	for (n = 0; n < AMRNBTEST_NUM_IBUF; n++) {
		munmap(ipmem_ptr[n], AMRNBTEST_IBUFSZ);
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

static int play_file(struct audtest_config *config, 
					 int fd, size_t count)
{
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config->private_data;
	int ret_val = 0;
	char *content_buf;

	audio_data->next = (char*)malloc(count);

	printf(" play_file: count=%d,next=%p\n", count, audio_data->next);

	if (!audio_data->next) {
		fprintf(stderr,"could not allocate %d bytes\n", count);
		return -1;
	}

	audio_data->org_next = audio_data->next;
	content_buf = audio_data->org_next;

	if (read(fd, audio_data->next, count) != (ssize_t)count) {
		fprintf(stderr,"could not read %d bytes\n", count);
		free(content_buf);
		return -1;
	}

	audio_data->avail = count;
	audio_data->org_avail = audio_data->avail;
        if (config->tgt == 0x07)
		ret_val = initiate_play(config, fill_buffer, audio_data);
	else
		ret_val = initiate_play_8660(config);
	free(content_buf);
	return ret_val;
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
		fprintf(stderr, "playamrnb: cannot open '%s'\n", config->file_name);
		return -1;
	}

	(void) fstat(fd, &stat_buf);

	return play_file(config, fd, stat_buf.st_size);;
}

int amrnb_rec(struct audtest_config *config)
{

	unsigned char buf[8192];
	struct msm_audio_config cfg;
	struct msm_audio_amrnb_enc_config amrnb_cfg;
	unsigned sz;
	int fd, afd;
	unsigned total = 0;
	unsigned char tmp;

        fd = open(config->file_name, O_CREAT | O_RDWR, 0666);
	if (fd < 0) {
		perror("cannot open output file");
		return -1;
	}

	afd = open("/dev/msm_amrnb_in", O_RDONLY);
	if (afd < 0) {
		perror("cannot open msm_amrnb_in");
		close(fd);
		return -1;
	}

	config->private_data = (void*) afd;

	if (ioctl(afd, AUDIO_GET_CONFIG, &cfg)) {
		perror("cannot read audio config");
		goto fail;
	}

	sz = cfg.buffer_size;
	fprintf(stderr,"buffer size %d\n", sz);
	if (sz > sizeof(buf)) {
		fprintf(stderr,"buffer size %d too large\n", sz);
		goto fail;
	}

	/* AMRNB specific settings */
	if (ioctl(afd, AUDIO_GET_AMRNB_ENC_CONFIG, &amrnb_cfg)) {
		perror("cannot read audio amrnb config");
		goto fail;
	}
#if 0
	/* Use Default Value */
	amrnb_cfg.voicememoencweight1 = 0x0000;
	amrnb_cfg.voicememoencweight2 = 0x0000;
	amrnb_cfg.voicememoencweight3 = 0x4000;
	amrnb_cfg.voicememoencweight4 = 0x0000;
#endif

	amrnb_cfg.dtx_mode_enable = config->channel_mode; /* 0 - DTX off, 0xFFFF - DTX on */
	amrnb_cfg.enc_mode = config->sample_rate;

	if (ioctl(afd, AUDIO_SET_AMRNB_ENC_CONFIG, &amrnb_cfg)) {
		perror("cannot write audio amrnb config");
		goto fail;
	}

	fprintf(stderr,"voicememoencweight1=0x%4x\n", amrnb_cfg.voicememoencweight1);
	fprintf(stderr,"voicememoencweight2=0x%4x\n", amrnb_cfg.voicememoencweight2);
	fprintf(stderr,"voicememoencweight3=0x%4x\n", amrnb_cfg.voicememoencweight3);
	fprintf(stderr,"voicememoencweight4=0x%4x\n", amrnb_cfg.voicememoencweight4);
	fprintf(stderr,"dtx_mode_enable=0x%4x\n", amrnb_cfg.dtx_mode_enable);
	fprintf(stderr,"test_mode_enable=0x%4x\n", amrnb_cfg.test_mode_enable);
	fprintf(stderr,"enc_mode=0x%4x\n", amrnb_cfg.enc_mode);/* 0-MR475,1-MR515,2-MR59,3-MR67,4-MR74
                                5-MR795, 6- MR102, 7- MR122(default) */

	if (ioctl(afd, AUDIO_START, 0)) {
		perror("cannot start audio");
		goto fail;
	}

	fcntl(0, F_SETFL, O_NONBLOCK);
	fprintf(stderr,"\n*** AMRNB PACKET RECORDING * HIT ENTER TO STOP ***\n");

	for (;;) {
		while (read(0, &tmp, 1) == 1) {
			if ((tmp == 13) || (tmp == 10)) goto done;
		}
		if (read(afd, buf, sz) != (ssize_t)sz) {
			perror("cannot read buffer");
			goto fail;
		}
		if (write(fd, buf, sz) != (ssize_t)sz) {
			perror("cannot write buffer");
			goto fail;
		}
		total += sz;
	}
	done:
	if (ioctl(afd, AUDIO_STOP, 0)) {
		perror("cannot stop audio");
		goto fail;
	}
	close(afd);
	close(fd);
	return 0;

	fail:
	close(afd);
	close(fd);
	unlink(config->file_name);
	return -1;
}

void* playamrnb_thread(void* arg) {
	struct audiotest_thread_context *context = 
	(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = amrnb_play(&context->config);
        printf("Free audio instance 0x%8x \n", (unsigned int) context->config.private_data);
        free(context->config.private_data);
	free_context(context);
	pthread_exit((void*) ret_val);
    return NULL;
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

int amrnbplay_read_params(void) {
	struct audiotest_thread_context *context; 
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
                struct audio_pvt_data *audio_data;
                audio_data = (struct audio_pvt_data *) malloc(sizeof(struct audio_pvt_data));
                if(!audio_data) {
                        printf("error allocating audio instance structure \n");
                        free_context(context);
                        ret_val = -1;
                } else {
                        printf("Created audio instance 0x%8x \n",(unsigned int) audio_data);
                        memset(audio_data, 0, sizeof(struct audio_pvt_data));
                        #ifdef _ANDROID_
                        audio_data->outfile = "/data/pcm.wav";
                        #else
                        audio_data->outfile = "/tmp/pcm.wav";
                        #endif
			audio_data->repeat = 0;
			audio_data->quit = 0;
			context->config.file_name = "/data/data.amr"; 
			context->type = AUDIOTEST_TEST_MOD_AMRNB_DEC;
			context->config.tgt = 0x7;
			audio_data->mode = 0;
			out_size = 8192 + sizeof(struct dec_meta_out); 
			in_size = 320; 
			
			token = strtok(NULL, " "); 
			while (token != NULL) {
				if (!memcmp(token,"-id=", (sizeof("-id=") - 1))) {
					context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
				} else if
				(!memcmp(token, "-mode=", (sizeof("-mode=") - 1))) {
					audio_data->mode = atoi(&token[sizeof("-mode=") - 1]);
				} else if (!memcmp(token, "-out=",
                                        (sizeof("-out=") - 1))) {
                                        audio_data->outfile = token + (sizeof("-out=")-1);
				} else if (!memcmp(token,"-tgt=", (sizeof("-tgt=") - 1))) {
					context->config.tgt =  atoi(&token[sizeof("-tgt=") - 1]);
				} else if (!memcmp(token, "-wr=",(sizeof("-wr=") - 1))) {
					file_write = atoi(&token[sizeof("-wr=") - 1]);
				} else if (!memcmp(token, "-outsize=", (sizeof("-outsize=") - 1))) {
					out_size = atoi(&token[sizeof("-outsize=") - 1]) + sizeof(struct dec_meta_out);
				} else if (!memcmp(token, "-insize=", (sizeof("-insize=") - 1))) {
					in_size = atoi(&token[sizeof("-insize=") - 1]);
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

			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create( &context->thread, NULL, playamrnb_thread, (void*) context);
		}
	}
	return ret_val;
}

int amrnbrec_read_params(void) {
	struct audiotest_thread_context *context;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/record.raw";
		context->config.channel_mode = 0; /* DTX off */
		context->config.sample_rate =  7; /* 12.2 Kbps */

		token = strtok(NULL, " ");

		while (token != NULL) {
			printf("%s \n", token);
			if (!memcmp(token,"-id=", (sizeof("-id=") - 1))) {
				context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
			} else if (!memcmp(token,"-dtx=", (sizeof("-dtx=") - 1))) {
				context->config.channel_mode =  atoi(&token[sizeof("-dtx=") - 1]);
			} else if (!memcmp(token,"-rate=", (sizeof("-rate=") - 1))) {
				context->config.sample_rate =  atoi(&token[sizeof("-rate=") - 1]);
			} else {
				context->config.file_name = token;
			}
			token = strtok(NULL, " ");
		}
		context->type = AUDIOTEST_TEST_MOD_AMRNB_ENC;
		pthread_create( &context->thread, NULL, recamrnb_thread, (void*) context);
	}

	return ret_val;
}

int amrnb_play_control_handler(void* private_data) {
	int drvfd , ret_val = 0;
	int volume;
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) private_data;
	char *token;

	token = strtok(NULL, " ");
	if ((private_data != NULL) && 
		(token != NULL)) {
		drvfd = audio_data->afd;
		if(!memcmp(token,"-cmd=", (sizeof("-cmd=") -1))) {
			token = &token[sizeof("-cmd=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
			if (!strcmp(token, "pause")) {
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
				audio_data->avail = audio_data->org_avail;
				audio_data->next  = audio_data->org_next;
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

int amrnb_rec_control_handler(void* private_data) {
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

const char *amrnbplay_help_txt = 
	"Play amrnb file: type \n\
echo \"playamrnb path_of_file -id=xxx -mode=x -out=path_of_outfile\" > %s \n\
mode= 0(tunnel mode) or 1 (non-tunnel mode) \n\
Supported control command: pause, resume, volume, flush, quit \n ";

void amrnbplay_help_menu(void) {
	printf(amrnbplay_help_txt, cmdfile);
}

const char *amrnbrec_help_txt =
"Record amrnb: type \n\
echo \"recamrnb path_of_file -rate=yyyy -dtx=zz -id=xxx\" > %s \n\
dtx= 0(off) or 65535(on) -repeat=x \n\
rate= 0(MR475),1(MR515),2(MR59),3(MR67),4(MR74),5(MR795),6(MR102),7(MR122) \n\
Repeat 'x' no. of times, repeat infinitely if repeat = 0\n\
Supported control command: N/A \n ";

void amrnbrec_help_menu(void) {
	printf(amrnbrec_help_txt, cmdfile);
}

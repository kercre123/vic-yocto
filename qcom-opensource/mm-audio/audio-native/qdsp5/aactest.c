/* aactest.c - native AAC test application
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
#include <sys/stat.h>
#include<unistd.h>
#include<string.h>
#include <errno.h>
#include "audiotest_def.h"
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/msm_audio.h>
#include <linux/msm_audio_aac.h>
#include "ion_alloc.h"

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
#define EOS			      1
#define NUM_BITS_PER_SAMPLE		2

#define AUDIO_AAC_MODE_AAC_LC            0x02
#define AUDIO_AAC_MODE_AAC_P             0x05
#define AUDIO_AAC_MODE_EAAC_P            0x1D
#define AAC_FORMAT_ADTS                  65535

#define MIN(A,B)	(((A) < (B))?(A):(B))

struct sample_rate_idx {
	uint32 sample_rate;
	uint32 sample_rate_idx;
};

static struct sample_rate_idx sample_idx_tbl[10] = {
	{8000, 0x0b},
	{11025, 0x0a},
	{12000, 0x09},
	{16000, 0x08},
	{22050, 0x07},
	{24000, 0x06},
	{32000, 0x05},
	{44100, 0x04},
	{48000, 0x03},
	{64000, 0x02},
};

uint8   audaac_header[AUDAAC_MAX_ADTS_HEADER_LENGTH];
unsigned int audaac_hdr_bit_index;
static unsigned int aac_rec_bitrate;
static unsigned int aac_type;// AAC_LC(2), AAC_P(5), EAAC_P(1d)
static unsigned short aac_channels; /* 1 for mono,2 for stereo,6 for AAC 5.1 */
int tickcount;
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
	{'f', 'm', 't', ' '}, 16, 1, 2, 48000, 96000, 4,
	16, {'d', 'a', 't', 'a'}, 0
	};

typedef struct TIMESTAMP{
	unsigned long LowPart;
	unsigned long HighPart;
} __attribute__ ((packed)) TIMESTAMP;

struct meta_in{
	unsigned char reserved[18];
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

struct enc_meta_out_8660{
	unsigned int offset_to_frame;
	unsigned int frame_size;
	unsigned int encoded_pcm_samples;
	unsigned int msw_ts;
	unsigned int lsw_ts;
	unsigned int nflags;
} __attribute__ ((packed));

struct meta_out_8660_pb{
	unsigned int offset_to_frame;
	unsigned int frame_size;
	unsigned int encoded_pcm_samples;
	unsigned int msw_ts;
	unsigned int lsw_ts;
	unsigned int nflags;
} __attribute__ ((packed));

struct dec_meta_out{
	unsigned int reserved[7];
	unsigned int num_of_frames;
	struct meta_out_8660_pb meta_out_8660_pb[];
} __attribute__ ((packed));

static int in_size =0;
static int out_size =0;
static int file_write=0;
static int eos_ack=0;
static pthread_mutex_t avail_lock;
static pthread_cond_t avail_cond;
static pthread_mutex_t consumed_lock;
static pthread_cond_t consumed_cond;
static pthread_mutex_t aac_ref_lock;
static int data_is_available = 0;
static int data_is_consumed = 0;
static int in_free_indx;
static int in_data_indx;
static int out_free_indx;
static int out_data_indx;
static int aac_read_buf_ref_cnt = 0;
extern int ionfd;

#define AACTEST_IBUFSZ (32*1024)
#define AACTEST_NUM_IBUF 2
#define AACTEST_IPMEM_SZ (AACTEST_IBUFSZ * AACTEST_NUM_IBUF)

#define AACTEST_OBUFSZ (32*1024)
#define AACTEST_NUM_OBUF 2
#define AACTEST_OPMEM_SZ (AACTEST_OBUFSZ * AACTEST_NUM_OBUF)

struct msm_audio_aio_buf aio_ip_buf[AACTEST_NUM_IBUF];
struct msm_audio_aio_buf aio_op_buf[AACTEST_NUM_OBUF];

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

static int aac_start_8660(struct audtest_config *clnt_config);
static void *aac_dec_event_8660(void *arg);
static void *aac_write_thread_8660(void *arg);
static void *aac_read_thread_8660(void *arg);
static void *setup_aac_file(struct audtest_config *clnt_config);



#ifdef _ANDROID_
static const char *cmdfile = "/data/audio_test";
#else
static const char *cmdfile = "/tmp/audio_test";
#endif

//void audaac_rec_install_bits(uint8 *input,byte num_bits_reqd,uint32 value,uint16 *hdr_bit_index);


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


void audaac_rec_install_adts_header_variable (uint16  byte_num,
			uint32 sample_index, uint8 channel_config)
{
  //uint16  bit_index=0;

  uint32  value;

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
                          0x660, /* Currently kept with CBR value */
                          &audaac_hdr_bit_index);

  /* number_of_raw_data_locks_in_frame, 2 bits */
  audaac_rec_install_bits(audaac_header,
                          2,
                          0,
                          &audaac_hdr_bit_index);

} /* audaac_rec_install_adts_header_variable */

static void *aac_dec(void *arg)
{
	struct meta_out meta;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd;
	unsigned long long *time;
	int fd, ret_val = 0;
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

	printf(" aac_read Thread, recsize=%d \n", audio_data->recsize);

	lseek(fd, 44, SEEK_SET);	/* Set Space for Wave Header */
	do {
		if (audio_data->bitstream_error == 1) {
			printf("Bitstream error notified, exit read thread\n");
			break;
		}
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
			if ((audio_data->flush_enable == 1 ||
				audio_data->outport_flush_enable == 1)
				&& errno == EBUSY) {
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
	printf(" end of recording PCM samples\n");
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
			sleep(1);
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
			} else if
			(suspend_event.event_type == AUDIO_EVENT_STREAM_INFO) {
				printf("event_notify: STREAM_INFO EVENT FROM \
					DRIVER:%d\n", suspend_event.event_type);
				printf("codec_type : %d\nchan_info : %d\n\
					sample_rate : %d\nstream_info: %d\n",
				 suspend_event.event_payload.stream_info.codec_type,
				 suspend_event.event_payload.stream_info.chan_info,
				 suspend_event.event_payload.stream_info.sample_rate,
				 suspend_event.event_payload.stream_info.bit_stream_info);
				#ifdef AUDIOV2
				if (audio_data->mode) {
					audio_data->outport_flush_enable = 1;
					ioctl(afd, AUDIO_OUTPORT_FLUSH, 0);
					audio_data->outport_flush_enable = 0;
				}
				#endif
			} else if
			(suspend_event.event_type ==
				AUDIO_EVENT_BITSTREAM_ERROR_INFO) {
				printf("event_notify: BITSTREAM ERROR EVENT \
					FROM DRIVER:%d\n",
					suspend_event.event_type);
				printf("BITSTREAM ERROR:\n codec_type : %d\n \
					error_count : %d\n error_type : %d\n",
					 suspend_event.event_payload.error_info.dec_id,
					 (0x0000FFFF &
					suspend_event.event_payload.error_info.err_msg_indicator),
					 suspend_event.event_payload.error_info.err_type);
				#ifdef AUDIOV2
				audio_data->bitstream_error = 1;
				#endif
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
	struct msm_audio_aac_config aac_config;
	// struct msm_audio_stats stats;
	unsigned n;
	pthread_t thread, event_th;
	int sz;
	char *buf; 
	int afd;
	int cntW=0;
	int ret = 0;

#ifdef AUDIOV2
	unsigned short dec_id;
#endif

	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;

	if (audio_data->mode)   {
		printf("non-tunel mode\n");
		afd = open("/dev/msm_aac", O_RDWR);
	} else {
		printf("tunel mode\n");
		afd = open("/dev/msm_aac", O_WRONLY);

	}

	if (afd < 0) {
		perror("aac_play: cannot open AAC device");
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

	if (audio_data->mode) {
		config.meta_field = 1;
		#ifdef AUDIOV2
		if (ioctl(afd, AUDIO_SET_ERR_THRESHOLD_VALUE, &audio_data->err_threshold_value)) {
			perror("could not set error threshold value");
			ret = -1;
			goto err_state;
		}
		#endif
	}

	config.sample_rate = clnt_config->sample_rate;
	config.channel_count = clnt_config->channel_mode;
	
	if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
		perror("could not set config");
		ret = -1;
		goto err_state;
	}

	if (ioctl(afd, AUDIO_GET_AAC_CONFIG, &aac_config)) {
		perror("could not get aac config");
		ret = -1;
		goto err_state;
	}

	aac_config.format = clnt_config->fmt_config.aac.format_type;
	aac_config.audio_object = clnt_config->fmt_config.aac.object_type;
	aac_config.sbr_on_flag = clnt_config->fmt_config.aac.sbr_flag;
	aac_config.sbr_ps_on_flag = clnt_config->fmt_config.aac.sbr_ps_flag;
	aac_config.channel_configuration = clnt_config->channel_mode;

	if (ioctl(afd, AUDIO_SET_AAC_CONFIG, &aac_config)) {
		perror("could not set aac config");
		ret = -1;
		goto err_state;
	}

	buf = (char*) malloc(sizeof(char) * config.buffer_size);
	if (buf == NULL) {
		perror("fail to allocate buffer\n");
		ret = -1;
		goto err_state;
	}

	if (audio_data->mode)
		config.buffer_size =
			(config.buffer_size - sizeof(struct meta_in));

	printf("initiate_play: buffer_size=%d, buffer_count=%d\n", config.buffer_size,
		   config.buffer_count);

	fprintf(stderr,"prefill\n");

	if (audio_data->mode) {
		/* non - tunnel portion */
		struct msm_audio_pcm_config config_rec;
		printf(" selected non-tunnel part\n");
		append_header.Sample_rate = clnt_config->sample_rate;
		append_header.Number_Channels = clnt_config->channel_mode;
		append_header.Bytes_Sec = append_header.Sample_rate *
			append_header.Number_Channels * 2;
		append_header.Block_align = append_header.Number_Channels * 2;
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
		config_rec.buffer_size += sizeof(struct meta_out);
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
		pthread_create(&thread, NULL, aac_dec, (void *) audio_data);
	}

	for (n = 0; n < config.buffer_count; n++) {
		if ((sz = fill(buf, config.buffer_size,
			cookie)) < 0)
			break;
		if (write(afd, buf, sz) != sz)
			break;
	}
	cntW=cntW+config.buffer_count; 

	fprintf(stderr,"start playback\n");
	if (ioctl(afd, AUDIO_START, 0) >= 0) {
		for (;audio_data->bitstream_error != 1;) {
#if 0
			if (ioctl(afd, AUDIO_GET_STATS, &stats) == 0)
				fprintf(stderr,"%10d\n", stats.out_bytes);
#endif
			if (((sz = fill(buf, config.buffer_size,
				cookie)) < 0) || (audio_data->quit == 1) ||
				audio_data->bitstream_error) {
				if (audio_data->bitstream_error == 1)
					break;
				if ((audio_data->repeat == 0) || (audio_data->quit == 1)) {
					printf(" File reached end or quit issued, exit loop \n");
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
						while (fsync(afd) < 0) {
							printf("fsync \
								failed\n");
							sleep(1);
						}
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
			if (audio_data->suspend == 1) {
				printf("enter suspend mode\n");
				ioctl(afd, AUDIO_STOP, 0);
				while (audio_data->suspend == 1)
					sleep(1);
				ioctl(afd, AUDIO_START, 0);
				printf("exit suspend mode\n");
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
				printf(" aac_play: instance=%d repeat_cont=%d cntW=%d\n",
						(int) audio_data, audio_data->repeat, cntW);
			}
		}
		printf("end of aac play\n");
		ioctl(afd, AUDIO_ABORT_GET_EVENT, 0);
		sleep(5); 
	} else {
		printf("aac_play: Unable to start driver\n");
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

/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */


static int fill_buffer(void *buf, unsigned sz, void *cookie)
{
	struct meta_in meta;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) cookie;
	unsigned cpy_size = (sz < audio_data->avail?sz:audio_data->avail);

	if (audio_data->avail == 0) {
		return -1;
	}

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
		memcpy(((char*)buf + sizeof(struct meta_in)), audio_data->next, cpy_size);
	} else
		memcpy(buf, audio_data->next, cpy_size);

	audio_data->next += cpy_size; 
	audio_data->avail -= cpy_size;

	if (audio_data->mode)
		return cpy_size + sizeof(struct meta_in);
	else
		return cpy_size;
}

static int play_file(struct audtest_config *config, 
					 int fd, size_t count)
{
	int ret_val = 0;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config->private_data;
	char *content_buf;

	audio_data->next = (char*)malloc(count);
	printf("play_file: count=%d,next=%p\n", count, audio_data->next);
	if (!audio_data->next) {
		fprintf(stderr,"could not allocate %d bytes\n", count);
		return -1;
	}
	content_buf = audio_data->next;
	audio_data->org_next = audio_data->next;

	if (read(fd, audio_data->next, count) != (ssize_t) count) {
		fprintf(stderr,"could not read %d bytes\n", count);
		free(content_buf);
		return -1;
	}
	audio_data->avail = count;
	audio_data->org_avail = audio_data->avail;
	ret_val = initiate_play(config, fill_buffer, audio_data);
	free(content_buf);
	return ret_val;
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

	(void) fstat(fd, &stat_buf);


	play_file(config, fd, stat_buf.st_size);

	return 0;
}

void* playaac_thread(void* arg) {
	struct audiotest_thread_context *context = 
	(struct audiotest_thread_context*) arg;
	int ret_val;

	if (context->config.tgt == 0x08)
		ret_val = aac_start_8660(&context->config);
	else
		ret_val = aac_play(&context->config);

	if (!context->config.private_data)
		free(context->config.private_data);
	printf(" Free audio instance 0x%8x \n", (unsigned int) context->config.private_data);
	free(context->config.private_data);
	free_context(context);
	pthread_exit((void*) ret_val);
    return NULL;
}

int aacplay_read_params(void) {
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
                        printf(" Created audio instance 0x%8x \n",(unsigned int) audio_data);
                        memset(audio_data, 0, sizeof(struct audio_pvt_data));
                        #ifdef _ANDROID_
                        audio_data->outfile = "/data/pcm.wav";
                        #else
                        audio_data->outfile = "/tmp/pcm.wav";
                        #endif
			audio_data->err_threshold_value = 1;
			audio_data->bitstream_error = 0;
			audio_data->repeat = 0;
			audio_data->quit = 0;
			context->config.file_name = "/data/data.aac";
			memset(&context->config.fmt_config, 0, 
				   sizeof(context->config.fmt_config));
			context->config.sample_rate = 44100;
			context->config.channel_mode = 2;
			context->config.fmt_config.aac.object_type = AUDIO_AAC_OBJECT_LC;
			context->config.fmt_config.aac.format_type = AUDIO_AAC_FORMAT_RAW;
			context->config.tgt = 0x07;
			out_size = 8192 + sizeof(struct dec_meta_out);
			in_size = 8192;
			file_write = 1;
			aac_channels = 2; /* default setting to stereo AAC, set to 6
								 for AAC 5.1 */

			token = strtok(NULL, " ");
			while (token != NULL) {
				if (!memcmp(token, "-outsize=",
						   (sizeof("-outsize=") - 1))) {
					out_size = atoi(&token[sizeof("-outsize=") - 1]) + sizeof(struct dec_meta_out);
				} else if (!memcmp(token, "-insize=",
						   (sizeof("-insize=") - 1))) {
					in_size = atoi(&token[sizeof("-insize=") - 1]);
				} else if (!memcmp(token, "-wr=",
						   (sizeof("-wr=") - 1))) {
					file_write = atoi(&token[sizeof("-wr=") - 1]);
				}else if (!memcmp(token,"-rate=", (sizeof("-rate=") - 1))) {
					context->config.sample_rate = atoi(&token[sizeof("-rate=") - 1]);
					audio_data->freq = context->config.sample_rate;
					printf("-->SR %d\n", context->config.sample_rate);
				} else if (!memcmp(token,"-cmode=", (sizeof("-cmode=") - 1))) {
					context->config.channel_mode = 
					atoi(&token[sizeof("-cmode=") - 1]);
					audio_data->channels  = context->config.channel_mode;
					printf("-->ch %d\n", context->config.channel_mode);
				} else if (!memcmp(token,"-aac_channels=", (sizeof("-aac_channels=") - 1))) {
					aac_channels = atoi(&token[sizeof("-aac_channels=") - 1]);
					printf("AAC-->ch %d\n", aac_channels);
				} else if (!memcmp(token,"-profile=", (sizeof("-profile=") - 1))) {
					token = &token[sizeof("-profile=") - 1];
					printf("aac profile %s\n", token);
					if (!strcmp(token, "aac")) {
						context->config.fmt_config.aac.sbr_flag = 0;
						context->config.fmt_config.aac.sbr_ps_flag = 0;
					} else if (!strcmp(token, "aac+")) {
						context->config.fmt_config.aac.sbr_flag = 1;
						context->config.fmt_config.aac.sbr_ps_flag = 0;
					} else if (!strcmp(token, "eaac+")) {
						context->config.fmt_config.aac.sbr_flag = 1;
						context->config.fmt_config.aac.sbr_ps_flag = 1;
					} else {
						ret_val = -1;
						break;
					}
				} else if (!memcmp(token,"-type=", (sizeof("-type=") - 1))) {
					token = &token[sizeof("-type=") - 1];
					printf("aac format type %s\n", token);
					if (!strcmp(token, "adts")) {
						context->config.fmt_config.aac.format_type
						= AUDIO_AAC_FORMAT_ADTS;
					} else if (!strcmp(token, "raw")) {
						context->config.fmt_config.aac.format_type
						= AUDIO_AAC_FORMAT_RAW;
					} else if (!strcmp(token, "loas")) {
						context->config.fmt_config.aac.format_type
						= AUDIO_AAC_FORMAT_LOAS;
					} else if (!strcmp(token, "praw")) {
						context->config.fmt_config.aac.format_type
						= AUDIO_AAC_FORMAT_PSUEDO_RAW;
					} else if (!strcmp(token, "adif")) {
						context->config.fmt_config.aac.format_type
						= AUDIO_AAC_FORMAT_ADIF;
					} else {
						ret_val = -1;
						break;
					}
				} else if (!memcmp(token, "-bitstream=", (sizeof("-bitstream=") - 1))) {
					token = &token[sizeof("-bitstream=") - 1];
					printf("aac bitstream type %s\n", token);
					if (!strcmp(token, "lc")) {
						context->config.fmt_config.aac.object_type
						= AUDIO_AAC_OBJECT_LC;
					} else if (!strcmp(token, "ltp")) {
						context->config.fmt_config.aac.object_type
						= AUDIO_AAC_OBJECT_LTP;
					} else if (!strcmp(token, "erlc")) {
						context->config.fmt_config.aac.object_type
						= AUDIO_AAC_OBJECT_ERLC;
					} else if (!strcmp(token, "bsac")) {
						context->config.fmt_config.aac.object_type
						= AUDIO_AAC_OBJECT_BSAC;
					} else {
						ret_val = -1;
						break;
					}
				} else if (!memcmp(token,"-id=", (sizeof("-id=") - 1))) {
					context->cxt_id= atoi(&token[sizeof("-id=") - 1]);
				} else if
					(!memcmp(token, "-mode=", (sizeof("-mode=") - 1))) {
					audio_data->mode = atoi(&token[sizeof("-mode=") - 1]);
				} else if (!memcmp(token, "-out=",
                                        (sizeof("-out=") - 1))) {
                                        audio_data->outfile = token + (sizeof("-out=")-1);
				} else if (!memcmp(token, "-err_thr=",
					(sizeof("-err_thr=") - 1))) {
					audio_data->err_threshold_value =
						atoi(&token[sizeof("-err_thr=") - 1]);
				} else if (!memcmp(token, "-repeat=",
					(sizeof("-repeat=") - 1))) {
					audio_data->repeat = atoi(&token[sizeof("-repeat=") - 1]);
					if (audio_data->repeat == 0)
						audio_data->repeat = -1;
					else
						audio_data->repeat--;
                                }  else if (!memcmp(token, "-tgt=",
						   (sizeof("-tgt=") - 1))) {
					context->config.tgt = atoi(&token[sizeof("-tgt=") - 1]);
					printf("-->TGT=%d\n",context->config.tgt);
				} else {
					context->config.file_name = token;
				}
				token = strtok(NULL, " ");
			}
			if (context->config.tgt == 0x07) {
				if (context->config.fmt_config.aac.format_type == AUDIO_AAC_FORMAT_ADIF){
					printf("adif contents not supported for 7k targets\n");
					ret_val = -1;
				}
			}

			if (!ret_val) {
				context->type = AUDIOTEST_TEST_MOD_AAC_DEC;
                	        context->config.private_data = (struct audio_pvt_data *) audio_data;
				pthread_create( &context->thread, NULL, playaac_thread, 
								(void*) context);
			} else {
			        printf(" Free audio instance 0x%8x \n", (int) audio_data);
			        free(audio_data);
				free_context(context);
			}
		}

	}
	return 0;
}


static int rec_stop;

/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

struct WAV_header {
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

static int fill_pcm_buffer(void *buf, unsigned sz, void *cookie)
{
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) cookie;
	unsigned cpy_size = 0;

	cpy_size = (sz < audio_data->avail ? sz : audio_data->avail);
	printf("cpy_size = %d audio_data->next = %p buf = %p\n", cpy_size, audio_data->next, buf);
	if (audio_data->avail == 0)
		return -1;
	if (!audio_data->next) {
		printf("error in next buffer returning with out copying\n");
		return -1;
	}
	if (cpy_size == 0) {
		return -1;
	}
	memcpy(buf, audio_data->next, cpy_size);
	audio_data->next += cpy_size;
	audio_data->avail -= cpy_size;
	return cpy_size;
}


void add_meta_out(char *pcm_buf, int eos, void *config, int buffer_size)
{
	unsigned long long duration = 0;
	struct meta_out metaout;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config;
	printf("add_meta_out");

	metaout.offset = sizeof(metaout);
	duration = audio_data->frame_count * ((buffer_size * 1000) /
				(audio_data->freq * audio_data->channels
				* NUM_BITS_PER_SAMPLE));
	printf("duration = %llu\n", duration);
	metaout.ntimestamp.LowPart = duration & 0xFFFFFFFF;
	metaout.ntimestamp.HighPart = (duration >> 32) & 0xFFFFFFFF;
	metaout.nflags = eos;
	metaout.sample_frequency = audio_data->freq;
	metaout.channel = audio_data->channels;
	metaout.tick_count = tickcount++;
	metaout.errflag = 0;
	memcpy(pcm_buf, &metaout, sizeof(metaout));
#ifdef DEBUG_LOCAL
	printf("Meta_out High part is %lu\n",
				metaout.ntimestamp.HighPart);
	printf("Meta_out Low part is %lu\n",
				metaout.ntimestamp.LowPart);
	printf("Meta Out Timestamp: %llu\n",
				(((unsigned long long)metaout.ntimestamp.HighPart << 32)
				 + metaout.ntimestamp.LowPart));
#endif
}

static void *aac_nt_enc(void *arg)
{
	struct audtest_config *config = (struct audtest_config *)arg;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *)
						config->private_data;
	#ifdef AUDIOV2
	struct msm_audio_pcm_config pcm_config;
	#else
	struct msm_audio_config pcm_config;
	#endif
	int afd = audio_data->afd;
	char *pcm_buf;
	int fd;
	int len, total_len;
	struct WAV_header hdr;
	int ret = 0;
	int cntW = 0, sz = 0;
	unsigned n = 0;
	int eos_sent = 0;
	len = 0;
	total_len = 0;

	if (config == NULL) {
		return (void *)-1;
	}
	fd = open(config->in_file_name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "playwav: cannot open '%s'\n", config->in_file_name);
		return (void *)-1;
	}
	if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		fprintf(stderr, "playwav: cannot read header\n");
		return (void *)-1;
	}
	fprintf(stderr,"playwav: %d ch, %d hz, %d bit, %s\n",
			hdr.num_channels, hdr.sample_rate, hdr.bits_per_sample,
			hdr.audio_format == FORMAT_PCM ? "PCM" : "unknown");
	if ((hdr.riff_id != ID_RIFF) ||
		(hdr.riff_fmt != ID_WAVE) ||
		(hdr.fmt_id != ID_FMT)) {
		fprintf(stderr, "playwav: '%s' is not a riff/wave file\n",
		config->in_file_name);
		return (void *)-1;
	}
	if ((hdr.audio_format != FORMAT_PCM) ||
		(hdr.fmt_sz != 16)) {
		fprintf(stderr, "playwav: '%s' is not pcm format\n", config->in_file_name);
		return (void *)-1;
	}
	if (hdr.bits_per_sample != 16) {
		fprintf(stderr, "playwav: '%s' is not 16bit per sample\n", config->in_file_name);
		return (void *)-1;
	}
	audio_data->next = (char*)malloc(hdr.data_sz);
	audio_data->org_next = audio_data->next;
	printf(" play_file: count=%d,next=%p\n", hdr.data_sz, audio_data->next);
	if (!audio_data->next) {
		fprintf(stderr,"could not allocate %d bytes\n", hdr.data_sz);
		return (void *)-1;
	}
	if (read(fd, audio_data->next, hdr.data_sz) != (ssize_t) hdr.data_sz) {
		fprintf(stderr,"could not read %d bytes\n", hdr.data_sz);
		return (void *)-1;
	}
	audio_data->avail = hdr.data_sz;
	audio_data->org_avail = audio_data->avail;

	/* non - tunnel encoding portion */
	printf(" selected non-tunnel part\n");
	if (ioctl(afd, AUDIO_GET_CONFIG, &pcm_config)) {
		perror("could not get config");
		ret = -1;
		goto err_state;
	}
	pcm_buf = (char*) malloc(sizeof(char) * pcm_config.buffer_size + sizeof(struct meta_out));
		if (pcm_buf == NULL) {
			perror("fail to allocate buffer\n");
			ret = -1;
			goto err_state;
		}
	printf("aac_nt_enc: buffer_size=%d, buffer_count=%d\n", pcm_config.buffer_size,
		pcm_config.buffer_count);
	audio_data->frame_count = pcm_config.buffer_count;
	pcm_config.buffer_size = (8 * 1024) + sizeof(struct meta_out); /*taking 8k as
									input buffer + size of meta*/
	fprintf(stderr,"prefill\n");
	for (n = 0; n < pcm_config.buffer_count; n++) {
		if ((sz = fill_pcm_buffer((pcm_buf + sizeof(struct meta_out)), pcm_config.buffer_size - sizeof(struct meta_out), (void *)audio_data)) < 0)
			break;
		if (sz < (signed)(pcm_config.buffer_size - sizeof(struct meta_out)))
			add_meta_out(pcm_buf, 1, &audio_data, pcm_config.buffer_size);
		else
			add_meta_out(pcm_buf, 0, &audio_data, pcm_config.buffer_size);
		if (write(afd, pcm_buf, sz + sizeof(struct meta_out)) != sz)
			//break;
		audio_data->frame_count++;
	}

	cntW = cntW + pcm_config.buffer_count;
	fprintf(stderr,"start encoding\n");
	if (ioctl(afd, AUDIO_START, 0) < 0) {
		perror("cannot start audio");
		goto fail;
	}
	while (1) {
		sz = fill_pcm_buffer(pcm_buf + sizeof(struct meta_out), pcm_config.buffer_size - sizeof(struct meta_out), (void *)audio_data);
		printf("sz = %d\n", sz);
		if (sz < (signed int)(pcm_config.buffer_size - sizeof(struct meta_out)) && !eos_sent) {
			printf("sending eos\n");
			add_meta_out(pcm_buf, 1, audio_data, pcm_config.buffer_size);
			if (sz > 0)
				sz += sizeof(struct meta_out);
			else
				sz = sizeof(struct meta_out);
			eos_sent = 1;
		}
		else if (sz < 0) {
			printf("end of file reached\n");
			goto exit;
		}
		else {
			add_meta_out(pcm_buf, 0, audio_data, pcm_config.buffer_size);
			sz += sizeof(struct meta_out);
		}
		if (write(afd, pcm_buf, sz) != sz) {
			printf(" write return not equal to sz, exit loop\n");
			break;
		} else {
			cntW++;
			audio_data->frame_count++;
			printf(" NT enc PCM dump:cntW=%d frame_count = %d\n", cntW, audio_data->frame_count++);
		}
	}
exit:
	printf("end of pcm dump\n");
	sleep(5);
	printf("came out of sleep nt encoder function\n");
	if (ioctl(afd, AUDIO_FLUSH, NULL)) {
		perror("Failed to flush buffers\n");
		goto fail;
	}
	free(pcm_buf);
err_state:
fail:
	close(fd);
	printf("returning from nt encoder function\n");
	return (void *)ret;
}


struct aac_encoded_meta_in {
	uint16_t metadata_len;
	uint16_t time_stamp_dword_lsw;
	uint16_t time_stamp_dword_msw;
	uint16_t time_stamp_lsw;
	uint16_t time_stamp_msw;
	uint16_t nflag_lsw;
	uint16_t nflag_msw;
};

int mode;
int aac_rec(struct audtest_config *config)
{
  unsigned char *buf=NULL;
  struct msm_audio_stream_config stream_cfg;
  struct msm_audio_aac_enc_config aac_enc_cfg;
  struct audio_pvt_data audio_data;

  int sample_idx = 0;
  unsigned int loop;
  signed int framesize = 0;
  int out_fd, afd;
  unsigned total = 0;
  static unsigned int cnt = 0;
  pthread_t thread;
  struct aac_encoded_meta_in nt_frame;
  unsigned char *start_buf=NULL;
#ifdef AUDIOV2
  unsigned short enc_id;
#endif
	mode = config->mode;
	printf("file_name = %s\n", config->file_name);
	out_fd = open(config->file_name, O_CREAT | O_RDWR, 0666);
		if (out_fd < 0) {
			perror("cannot open output file");
			return -1;
		}
#if defined(AUDIOV2) || defined(AUDIO7X27A)
	if (!mode) {
		  afd = open("/dev/msm_aac_in", O_RDONLY);
		  if (afd < 0) {
			perror("cannot open msm_aac_in");
			close(out_fd);
			return -1;
		  }
	} else {
		  afd = open("/dev/msm_aac_in", O_RDWR);
		  if (afd < 0) {
			perror("cannot open msm_aac_in");
			return -1;
		  }
	}
#else
  afd = open("/dev/msm_pcm_in", O_RDWR);
  if (afd < 0) {
    perror("cannot open msm_pcm_in");
    close(out_fd);
    return -1;
  }
#endif
#ifdef AUDIOV2
	if (!mode) {
		if (ioctl(afd, AUDIO_GET_SESSION_ID, &enc_id)) {
			perror("could not get encoder session id\n");
			close(out_fd);
			close(afd);
			return -1;
		}

		if (devmgr_register_session(enc_id, DIR_TX) < 0) {
			perror("could not get register encoder session id\n");
			close(out_fd);
			close(afd);
			return -1;
		}
	}
#endif

	audio_data.afd = afd;
	audio_data.mode = mode;
	audio_data.channels = config->channel_mode;
	audio_data.freq = config->sample_rate;
	config->private_data = (struct audio_pvt_data *)&audio_data;

  cnt = 0;
  for (loop=0; loop< sizeof(sample_idx_tbl) / \
			 sizeof(struct sample_rate_idx); \
			 loop++) {
	if(sample_idx_tbl[loop].sample_rate == config->sample_rate) {
		sample_idx  = sample_idx_tbl[loop].sample_rate_idx;
	}
  }

  if (ioctl(afd, AUDIO_GET_STREAM_CONFIG, &stream_cfg)) {
    perror("cannot read audio stream config");
    goto fail;
  }
	printf("Default buffer size %d, buffer count %d\n", stream_cfg.buffer_size, stream_cfg.buffer_count);
	buf = (unsigned char *) malloc(stream_cfg.buffer_size);
	if (buf == NULL) {
		perror("cannot allocate memory for record");
		goto fail;
	}
	start_buf = buf;
  /* Set buffer size to default, So AAC is selected as encoder in driver */
  if (ioctl(afd, AUDIO_SET_STREAM_CONFIG, &stream_cfg)) {
    perror("cannot write audio stream config");
    goto fail;
  }

  if (mode)
	pthread_create(&thread, NULL, aac_nt_enc, (void *) config);

  if (ioctl(afd, AUDIO_GET_AAC_ENC_CONFIG, &aac_enc_cfg)) {
    perror("cannot read aac encoder  config");
    goto fail;
  }
  printf("Default channel %d, sample rate %d bit rate %d\n", aac_enc_cfg.channels,
	aac_enc_cfg.sample_rate, aac_enc_cfg.bit_rate);

  aac_enc_cfg.channels = config->channel_mode;
  aac_enc_cfg.sample_rate = config->sample_rate;
  aac_enc_cfg.bit_rate = aac_rec_bitrate;
  printf("channel mode = %d\n", aac_enc_cfg.channels);
  if (ioctl(afd, AUDIO_SET_AAC_ENC_CONFIG, &aac_enc_cfg)) {
    perror("cannot write aac encoder  config");
    goto fail;
  }

  fcntl(0, F_SETFL, O_NONBLOCK);
  fprintf(stderr,"\n*** RECORDING * HIT ENTER TO STOP ***\n");

  if (!mode) {
	if (ioctl(afd, AUDIO_START, 0) < 0) {
		perror("cannot start audio");
		goto fail;
	}
  }

  rec_stop = 0;
   while (!rec_stop) {
    framesize = read(afd, buf, stream_cfg.buffer_size);
    printf("read call returned framesize = %d\n", framesize);

    if (!config->mode) {
		audaac_rec_install_adts_header_variable(framesize + AUDAAC_MAX_ADTS_HEADER_LENGTH,
				sample_idx, (config->channel_mode - 1 ));
	} else {
		audaac_rec_install_adts_header_variable(framesize + AUDAAC_MAX_ADTS_HEADER_LENGTH - sizeof(struct aac_encoded_meta_in),
				sample_idx, (config->channel_mode - 1 ));
		memcpy(&nt_frame, buf, sizeof(struct aac_encoded_meta_in));
		buf = buf + sizeof(struct aac_encoded_meta_in);
		printf("nflags_lsw = %d, nflags_msw = %d\n", nt_frame.nflag_lsw, nt_frame.nflag_msw);
		framesize -= sizeof(struct aac_encoded_meta_in);
	}
    if (framesize > 0) {
	    write(out_fd,audaac_header,AUDAAC_MAX_ADTS_HEADER_LENGTH);  // writing aac adts header format with frame len
	    printf(" AAC recoded frame num = %d , size = %d\n",++cnt, framesize);
	    if(write(out_fd, buf, framesize) != framesize) {
	      perror("cannot write buffer");
	      goto fail;
	    }
	    total += framesize;
		if (config->mode)
			buf = start_buf;
    } else
	rec_stop = 1;
  }
  if (config->mode)
   sleep(6);
  printf("\n*** RECORDING * STOPPED ***\n");
  close(afd);

	if(buf)
		free(start_buf);
  close(out_fd);
  printf("closed out file fd\n");
#ifdef AUDIOV2
	if (!config->mode)
		if (devmgr_unregister_session(enc_id, DIR_TX) < 0) {
			return -1;
		}
#endif

  return 0;

  fail:
  close(afd);
  if(buf)
	free(buf);
  close(out_fd);
#ifdef AUDIOV2
	if (!config->mode)
		if (devmgr_unregister_session(enc_id, DIR_TX) < 0) {
			return -1;
		}
#endif
  unlink(config->file_name);
  return -1;
}


/* 8660 Record*/
static int fill_pcm_buffer_8660(void *buf, unsigned sz, void *cookie)
{
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) cookie;
	unsigned cpy_size = (sz < audio_data->avail ? sz : audio_data->avail);   
	printf("cpy_size = %d audio_data->avail= %d \n", cpy_size, audio_data->avail);
	if (audio_data->avail == 0)
		return -1;
	if (!audio_data->next) {
		printf("error in next buffer returning with out copying\n");
		return -1;
	}
	memcpy(buf, audio_data->next, cpy_size);
	audio_data->next += cpy_size; 
	audio_data->avail -= cpy_size;
	//printf("%s: cpy_size=%d\n", __func__, cpy_size);
	return cpy_size;
}


void add_meta_in_8660(char *pcm_buf, int eos, void *config, int buffer_size)
{
	unsigned long long duration = 0;
	struct meta_in meta;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config;
	meta.offset = sizeof(struct meta_in);
	duration = audio_data->frame_count * ((buffer_size * audio_data->freq) / (audio_data->channels * 2));
	meta.ntimestamp.LowPart = duration & 0xFFFFFFFF;
	meta.ntimestamp.HighPart = (duration >> 32) & 0xFFFFFFFF;
	meta.nflags = eos;
	memcpy(pcm_buf, &meta, sizeof(meta));
}

static void *aac_nt_enc_8660(void *arg)
{
	int ret = 0;
#ifdef AUDIOV2
	struct audtest_config *config = (struct audtest_config *)arg;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config->private_data;
	struct msm_audio_config pcm_config;
	int afd = audio_data->afd;
	char *pcm_buf;
	int fd;
	int len, total_len;
	struct WAV_header hdr;
	int cntW = 0, sz = 0;
	len = 0;
	total_len = 0;
	printf("%s==========>\n", __func__);
	if (config == NULL) {
		return (void *)-1;
	}

	fd = open(config->in_file_name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "playwav: cannot open '%s'\n", config->file_name);
		return (void *)-1;
	}
	if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		fprintf(stderr, "playwav: cannot read header\n");
		return (void *)-1;
	}
	fprintf(stderr,"playwav: %d ch, %d hz, %d bit, %s\n",
	hdr.num_channels, hdr.sample_rate, hdr.bits_per_sample,
	hdr.audio_format == FORMAT_PCM ? "PCM" : "unknown");

	if ((hdr.riff_id != ID_RIFF) ||
			(hdr.riff_fmt != ID_WAVE) ||
			(hdr.fmt_id != ID_FMT)) {
		fprintf(stderr, "playwav: '%s' is not a riff/wave file\n", 
		config->in_file_name);
		return (void *)-1;
	}
	if ((hdr.audio_format != FORMAT_PCM) ||
			(hdr.fmt_sz != 16)) {
		fprintf(stderr, "playwav: '%s' is not pcm format\n", config->file_name);
		return (void *)-1;
	}
	if (hdr.bits_per_sample != 16) {
		fprintf(stderr, "playwav: '%s' is not 16bit per sample\n", config->file_name);
		return (void *)-1;
	}

	audio_data->next = (char*)malloc(hdr.data_sz);
	audio_data->org_next = audio_data->next;
	printf(" play_file: count=%d,next=%p\n", hdr.data_sz, audio_data->next);
	if (!audio_data->next) {
		fprintf(stderr,"could not allocate %d bytes\n", hdr.data_sz);
		return (void *)-1;
	}
	if (read(fd, audio_data->next, hdr.data_sz) != (ssize_t)hdr.data_sz) {
		fprintf(stderr,"could not read %d bytes\n", hdr.data_sz);
		return (void *)-1;
	}
	audio_data->avail = hdr.data_sz;
	audio_data->org_avail = audio_data->avail;

	/* non - tunnel encoding portion */
	printf(" selected non-tunnel part %d\n", audio_data->avail);
	
	if (ioctl(afd, AUDIO_GET_CONFIG, &pcm_config)) {
		perror("could not get pcm config");
		ret = -1;
		goto err_state;
	}

	pcm_buf = (char*) malloc((sizeof(char) * (pcm_config.buffer_size)));

	if (pcm_buf == NULL) {
		perror("fail to allocate buffer\n");
		ret = -1;
		goto err_state;
	}
	printf("aac_nt_enc: buffer_size=%d, buffer_count=%d\n", pcm_config.buffer_size,
	pcm_config.buffer_count);

	audio_data->frame_count = pcm_config.buffer_count;

	fprintf(stderr,"prefill\n");
	fprintf(stderr,"start encoding\n");
	sleep(2);

	while (1) {
		sz = fill_pcm_buffer_8660((pcm_buf+sizeof(struct meta_in)) , 
					(pcm_config.buffer_size -sizeof(struct meta_in)), 
		(void *)audio_data);
		printf("%s: size=%d\n", __func__, sz);
		if (sz > 0) {

			add_meta_in_8660(pcm_buf, 0, &audio_data, pcm_config.buffer_size);
			if (write(afd, pcm_buf, (sz + sizeof(struct meta_in)) ) != (ssize_t)
					(sz + sizeof(struct meta_in))) { 
				printf(" write return not equal to sz, exit loop\n");
				break;
			} else {
				cntW++;
				audio_data->frame_count++;
				printf(" NT enc PCM dump:cntW=%d frame_count = %d\n", cntW, 
								audio_data->frame_count++);
			}
		}
		else
		{
			printf("Writing EOS flag\n");
			add_meta_out(pcm_buf, 1, &audio_data, pcm_config.buffer_size);
			if (write(afd, pcm_buf, sizeof(struct meta_in))  != sizeof(struct meta_in))
			printf(" Write EOS failed");
			break;
		}
	}
	printf("end of pcm dump\n");
	sleep(5); 
	free(pcm_buf);
err_state:
	close(fd);
#endif //AUDIOV2
	return (void *)ret;
}


int aac_rec_8660(struct audtest_config *config)
{
#ifdef AUDIOV2
	unsigned char *buf;
	unsigned char *start_buf = NULL;
	struct enc_meta_out_8660 *meta = NULL;

	uint32_t format = config->fmt_config.aac.format_type; 
	unsigned int num_of_frames = config->frames_per_buf;
	unsigned int len = 0;

	struct msm_audio_stream_config stream_cfg;
	struct msm_audio_aac_enc_config aac_enc_cfg;
	struct msm_audio_buf_cfg buf_cfg;
	struct msm_audio_config pcm_cfg;
	struct msm_audio_aac_config aac_config;

	struct audio_pvt_data audio_data;
	int sample_idx=0, aac_sample_rate = 24000;
	unsigned loop;
	unsigned framesize = 0;
	int out_fd, afd;
	unsigned total = 0;
	static unsigned int cnt = 0;
	pthread_t thread;
	int offset = 0;
	unsigned short enc_id;
	mode = config->mode;
	printf("file_name = %s\n", config->file_name);
	out_fd = open(config->file_name, O_CREAT | O_RDWR, 0666);
	if (out_fd < 0) {
		perror("cannot open output file");
		return -1;
	}
	printf("%s: mode=%d\n", __func__, mode);
	if (!mode) {
		afd = open("/dev/msm_aac_in", O_RDONLY);
		if (afd < 0) {
			perror("cannot open msm_aac_in");
			close(out_fd);
			return -1;
		}
	} else {
		afd = open("/dev/msm_aac_in", O_RDWR);
		if (afd < 0) {
			perror("cannot open msm_aac_in");
			return -1;
		}
	}
	if (!mode) {
		if (ioctl(afd, AUDIO_GET_SESSION_ID, &enc_id)) {
			perror("could not get encoder session id\n");
			close(out_fd);
			close(afd);
			return -1;
		}

		if (devmgr_register_session(enc_id, DIR_TX) < 0) {
			perror("could not get register encoder session id\n");
			close(out_fd);
			close(afd);
			return -1;
		}
	}
	audio_data.afd = afd;
	audio_data.mode = mode;
	audio_data.channels = config->channel_mode;
	audio_data.freq = config->sample_rate;
	config->private_data = (struct audio_pvt_data *)&audio_data;

	cnt = 0;
	aac_sample_rate = config->sample_rate;
	if ((format == AUDIO_AAC_FORMAT_RAW) &&
		((aac_type == AUDIO_AAC_MODE_AAC_P) ||
		 (aac_type == AUDIO_AAC_MODE_EAAC_P))){
		if (config->sample_rate >= 24000) {
			printf("aac_rec_8660(): ==Sample rate change for aac+/eaac+==\n");
			aac_sample_rate = config->sample_rate/2;
		}
	}

	for (loop=0; loop< sizeof(sample_idx_tbl) / \
	sizeof(struct sample_rate_idx); \
	loop++) {
		if(sample_idx_tbl[loop].sample_rate == aac_sample_rate) {
			sample_idx  = sample_idx_tbl[loop].sample_rate_idx;
		}
	}

	if (ioctl(afd, AUDIO_GET_STREAM_CONFIG, &stream_cfg)) {
		perror("cannot read audio stream config");
		goto fail;
	}

	printf("Default buffer size %d, buffer count %d\n", stream_cfg.buffer_size, stream_cfg.buffer_count);
	buf = (unsigned char *) malloc(stream_cfg.buffer_size);
	if (buf == NULL) {
		perror("cannot allocate memory for record");
		goto fail;
	}
	start_buf = buf;
	
	/* Set buffer size to default, So AAC is selected as encoder in driver */
	if (ioctl(afd, AUDIO_SET_STREAM_CONFIG, &stream_cfg)) {
		perror("cannot write audio stream config");
		goto fail;
	}

	if (mode) {
		pthread_create(&thread, NULL, aac_nt_enc_8660, (void *) config);
	}

	if (ioctl(afd, AUDIO_GET_AAC_ENC_CONFIG, &aac_enc_cfg)) {
		perror("cannot read aac encoder  config");
		goto fail;
	}
	printf("Default channel %d, sample rate %d bit rate %d\n", aac_enc_cfg.channels,
	aac_enc_cfg.sample_rate, aac_enc_cfg.bit_rate);

	aac_enc_cfg.channels = config->channel_mode;
	aac_enc_cfg.sample_rate = config->sample_rate;
	aac_enc_cfg.bit_rate = aac_rec_bitrate;
	aac_enc_cfg.stream_format = format;
	printf("channel mode = %d\n", aac_enc_cfg.channels);
	if (ioctl(afd, AUDIO_SET_AAC_ENC_CONFIG, &aac_enc_cfg)) {
		perror("cannot write aac encoder  config");
		goto fail;
	}

	if (ioctl(afd, AUDIO_GET_AAC_CONFIG, &aac_config)) {
		perror("could not get aac config");
		goto fail;
	}

	aac_config.format = config->fmt_config.aac.format_type;
	aac_config.audio_object = config->fmt_config.aac.object_type;
	aac_config.sbr_on_flag = config->fmt_config.aac.sbr_flag;
	aac_config.sbr_ps_on_flag = config->fmt_config.aac.sbr_ps_flag;
	aac_config.channel_configuration = config->channel_mode;

	if (ioctl(afd, AUDIO_SET_AAC_CONFIG, &aac_config)) {
		perror("could not set aac config");
		goto fail;
	}

	printf("GET-BUF-CFG...\n");
	if (ioctl(afd, AUDIO_GET_BUF_CFG, &buf_cfg)) {
		perror("cannot get buf config");
		goto fail;
	}
	printf("SET-BUF-CFG...\n");
	buf_cfg.frames_per_buf = num_of_frames;
	buf_cfg.meta_info_enable = 1;
	if (ioctl(afd, AUDIO_SET_BUF_CFG, &buf_cfg)) {
		perror("cannot set buf config");
		goto fail;
	}
	if(mode) {
		if (ioctl(afd, AUDIO_GET_CONFIG, &pcm_cfg)) {
			perror("cannot get config");
			goto fail;
		}
		pcm_cfg.channel_count = config->channel_mode;
		pcm_cfg.sample_rate  = config->sample_rate ;
		if (ioctl(afd, AUDIO_SET_CONFIG, &pcm_cfg)) {
			perror("cannot set config");
			goto fail;
		}
		offset = sizeof(struct enc_meta_out_8660);
	}
	if (ioctl(afd, AUDIO_START, 0) < 0) {
		perror("cannot start audio");
		goto fail;
	}

	fcntl(0, F_SETFL, O_NONBLOCK);
	fprintf(stderr,"\n*** RECORDING * HIT ENTER TO STOP **frames_per_buf[%d]*\n", num_of_frames);
	rec_stop = 0;
	while (!rec_stop) {
		buf = start_buf;
		num_of_frames =  config->frames_per_buf;
		framesize = read(afd, buf, stream_cfg.buffer_size);
		printf("==>read[%d] num of frames[%d] \n", framesize, buf[0]);
		if(num_of_frames != buf[0]){
			printf("Num of frames rxed[%d] not in sync with configured[%d]\n",
							buf[0], num_of_frames);
			num_of_frames = buf[0];
		}
		/* Skip the first bytes */
		buf += sizeof(unsigned char);
		meta = (struct enc_meta_out_8660 *)buf;

		if(mode && (meta->nflags == 0x01)) {
			printf("***************EOS reached on o/p port******************\n");
			printf("nflags[%d]\n", meta->nflags);
			printf("***************EOS reached on o/p port******************\n");
			break;
		}
	
		while (num_of_frames > 0) {
			meta = (struct enc_meta_out_8660 *)buf;
			printf("offset[%d]framesize[%d]enc_pcm[%d]msw_ts[%d]lsw_ts[%d]\n",
				meta->offset_to_frame, 
				meta->frame_size,
				meta->encoded_pcm_samples, meta->msw_ts, meta->lsw_ts);
			len = meta->frame_size;
			if ( format == AUDIO_AAC_FORMAT_RAW)
			{
				printf("ADTS header: native_sample_rate = %d,"
					   "aac_sample_rate = %d, sample_idx = %d, channel = %d\n",
					   config->sample_rate, aac_sample_rate, sample_idx,
					   config->channel_mode);

				audaac_rec_install_adts_header_variable((len + 
							AUDAAC_MAX_ADTS_HEADER_LENGTH), 
							sample_idx, 
							(config->channel_mode - 1 ));
				write(out_fd,audaac_header,AUDAAC_MAX_ADTS_HEADER_LENGTH);
				total += AUDAAC_MAX_ADTS_HEADER_LENGTH;
			}
			if((write(out_fd, (start_buf +sizeof(unsigned char) + meta->offset_to_frame) , len)) != (ssize_t)len) {
				perror("cannot write buffer");
				goto fail;
			}
			buf += sizeof(struct enc_meta_out_8660);
			num_of_frames--;
			total += len;
			printf(" AAC recoded frame num[%d]totalrxed[%d] \n",++cnt, total);
		}

	}
	printf("\n*** RECORDING * STOPPED **total encoded bytes rxed[%d]\n", total);
	close(afd);

	if(start_buf)
	    free(start_buf);
	close(out_fd);

	if (devmgr_unregister_session(enc_id, DIR_TX) < 0) {
		return -1;
	}

	return 0;

fail:
	close(afd);
	if(start_buf)
		free(start_buf);
	close(out_fd);
	if (devmgr_unregister_session(enc_id, DIR_TX) < 0) {
		return -1;
	}
	unlink(config->file_name);
	return -1;
#endif //AUDIOV2
	return 0;
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
void* recaac_thread_8660(void* arg) {
	struct audiotest_thread_context *context = 
	(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = aac_rec_8660(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);
	return NULL;
}
int aacrec_read_params(void) {
	struct audiotest_thread_context *context; 
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/data.aac";
		context->config.sample_rate = 44100;
		context->config.channel_mode = 2;
		context->config.mode = 0;
		context->type = AUDIOTEST_TEST_MOD_AAC_ENC;
		aac_rec_bitrate =  168000;
		context->config.tgt = 0x07;
		aac_type = AUDIO_AAC_MODE_AAC_LC;
		context->config.fmt_config.aac.format_type = AUDIO_AAC_FORMAT_ADTS;
		context->config.fmt_config.aac.object_type = AUDIO_AAC_OBJECT_LC;
		context->config.fmt_config.aac.sbr_flag = 0;
		context->config.fmt_config.aac.sbr_ps_flag = 0;
	
		token = strtok(NULL, " ");
		while (token != NULL) {

			if (!memcmp(token,"-id=", (sizeof("-id=") - 1))) {
				context->cxt_id= atoi(&token[sizeof("-id=") - 1]);
			} else if (!memcmp(token,"-rate=", (sizeof("-rate=" - 1)))) {
					context->config.sample_rate = atoi(&token[sizeof("-rate=") - 1]);
			} else if (!memcmp(token,"-mode=", (sizeof("-mode=" - 1)))) {
					context->config.mode = atoi(&token[sizeof("-mode=") - 1]);
			} else if (!memcmp(token, "-channel=", (sizeof("-channel=") - 1))) {
					context->config.channel_mode =
					atoi(&token[sizeof("-channel=") - 1]);
			} else if (!memcmp(token, "-bps=", (sizeof("-bps=") - 1))) {
					aac_rec_bitrate = atoi(&token[sizeof("-bps=") - 1]);
			} else if (!memcmp(token, "-aac_type=", (sizeof("-aac_type=") - 1))) {
					token = &token[sizeof("-aac_type=") - 1];
					printf("aac object type is %s\n", token);
					if (!strcmp(token, "aac_lc")) {
						aac_type = AUDIO_AAC_MODE_AAC_LC;
						context->config.fmt_config.aac.sbr_flag = 0;
						context->config.fmt_config.aac.sbr_ps_flag = 0;
					} else if (!strcmp(token, "aac+")) {
						aac_type = AUDIO_AAC_MODE_AAC_P;
						context->config.fmt_config.aac.sbr_flag = 1;
						context->config.fmt_config.aac.sbr_ps_flag = 0;
					} else if (!strcmp(token, "eaac+")) {
						aac_type = AUDIO_AAC_MODE_EAAC_P;
						context->config.fmt_config.aac.sbr_flag = 1;
						context->config.fmt_config.aac.sbr_ps_flag = 1;
					} else {
						ret_val = -1;
						break;
					}
			} else if (!memcmp(token, "-tgt=", (sizeof("-tgt=") - 1))) {
					context->config.tgt = atoi(&token[sizeof("-tgt=") - 1]);
			} else if (!memcmp(token, "-infile=", (sizeof("-infile=") - 1))) {
					token = &token[sizeof("-infile=") - 1];
					printf("infile = %s\n", token);
					context->config.in_file_name = token;
			} else if (!memcmp(token, "-outfile=", (sizeof("-outfile=") - 1))) {
					token = &token[sizeof("-outfile=") - 1];
					context->config.file_name = token;
			} else if (!memcmp(token, "-frames=", (sizeof("-frames=") - 1))) {
				context->config.frames_per_buf= atoi(&token[sizeof("-frames=") - 1]);
				printf("Num of frames per buf=%d\n",context->config.frames_per_buf);
			}
			else if (!memcmp(token,"-type=", (sizeof("-type=") - 1))) {
				token = &token[sizeof("-type=") - 1];
				printf("aac format type %s\n", token);
				if (!strcmp(token, "adts")) {
					context->config.fmt_config.aac.format_type
						= AUDIO_AAC_FORMAT_ADTS;
				} else if (!strcmp(token, "raw")) {
					context->config.fmt_config.aac.format_type
						= AUDIO_AAC_FORMAT_RAW;
				} else {
					ret_val = -1;
					break;
				}
			}
			token = strtok(NULL, " ");
		}
		if(context->config.frames_per_buf < 1 || context->config.frames_per_buf > 5)
			context->config.frames_per_buf = 1;
		printf("format=%d frames_per_buf=%d config.sample_rate=%d,config.channel_mode=%d,aac_rec_bitrate=%d\n",
			context->config.fmt_config.aac.format_type,context->config.frames_per_buf, 
			context->config.sample_rate, context->config.channel_mode, aac_rec_bitrate);

		if(context->config.tgt != 0x08 )
		pthread_create( &context->thread, NULL, recaac_thread,
							(void*) context);
		else {
			pthread_create( &context->thread, NULL, recaac_thread_8660,
							(void*) context);
			if ((context->config.sample_rate < 8000) && (context->config.sample_rate > 48000)) {
				printf("ERROR in setting samplerate = %d. Supported "
					  "samplerates are 8000, 11025, 12000, 16000, 22050, "
					  "24000, 32000, 44100, 48000\n", context->config.sample_rate);
					ret_val = -1;
			} else {
				if ((aac_type == AUDIO_AAC_MODE_AAC_P) || (aac_type == AUDIO_AAC_MODE_EAAC_P)) {
					if (context->config.sample_rate < 24000) {
						printf("ERROR in setting samplerate = %d. Supported"
						 " samplerates for AAC+/EAAC+ are 24000, 32000,"
						 " 44100, 48000\n", context->config.sample_rate);
						ret_val = -1;
					}
				}
			}
			if ((context->config.channel_mode > 2) || (context->config.channel_mode <= 0)) {
				printf("ERROR in setting channels = %d. Supported "
					   "number of channels are 1 and 2\n", context->config.channel_mode);
				ret_val = -1;
			}
			if (!((aac_type == AUDIO_AAC_MODE_AAC_LC) || (aac_type == AUDIO_AAC_MODE_AAC_P) || (aac_type == AUDIO_AAC_MODE_EAAC_P))) {
				printf("ERROR in setting AAC profile = %d. Supported "
							"profile values are aac_lc(2), aac+(5), eaac+(29)\n", aac_type);
				ret_val = -1;
			}
			if (!((context->config.fmt_config.aac.format_type == AAC_FORMAT_ADTS)
				  || (context->config.fmt_config.aac.format_type == AUDIO_AAC_FORMAT_RAW)))  {
				printf("ERROR in setting AAC format = %d. Supported "
							"formats are adts, raw\n", context->config.fmt_config.aac.format_type);
				ret_val = -1;
			}
		}
	}
	return ret_val;
}

int aac_play_control_handler(void* private_data) {
	int drvfd , ret_val = 0;

	char *token;
	int volume;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) private_data;

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
				audio_data->flush_enable = 1;
				ioctl(drvfd, AUDIO_FLUSH, 0);
			}  else if (!strcmp(token, "quit")) {
				audio_data->quit = 1;
				printf("quit session\n");
                        } else
				ret_val = -1;
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
		if(!memcmp(token,"-cmd=", (sizeof("-cmd=") -1))) {
			token = &token[sizeof("-id=") - 1];
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
/***********************************************/
/*************** 8660 PLAYBACK *****************/
/***********************************************/

/* Get File content and create meta */
static int fill_buffer_8660_pb(void *buf, unsigned sz, void *cookie)
{
        struct meta_in meta;
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) cookie;
        unsigned cpy_size = (sz < audio_data->avail?sz:audio_data->avail);
	#ifdef DEBUG_LOCAL
	char *temp;
	printf("%s:frame count %d\n", __func__, audio_data->frame_count);
	#endif
	if (audio_data->mode) {
		meta.ntimestamp.LowPart = 0;
	        meta.ntimestamp.HighPart = (unsigned long long)(audio_data->frame_count);
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

/* Expect on raw file, which is  only aac data file */
static void *setup_aac_file(struct audtest_config *clnt_config)
{
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
        struct stat stat_buf;
        char *content_buf;
        int fd;
        size_t buffer_size;

        printf("setup aac file\n");
        fd = open(clnt_config->file_name, O_RDONLY);
        if (fd < 0) {
                printf("Err while opening AAC bin file for decoder \
                        : %s\n", clnt_config->in_file_name);
		return((void *)-1);
        }
        (void) fstat(fd, &stat_buf);
        buffer_size = stat_buf.st_size;
	/* memory set for file */
        audio_data->next = (char*)malloc(buffer_size);
        printf("Total AAC bin file len: %d, buffer start addr=0x%p\n", buffer_size, audio_data->next);

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
        printf("Total available aac len: %d, buffer start addr=%p\n", audio_data->avail, audio_data->org_next);
	return 0;
fail:
        close(fd);
        free(content_buf);
        return ((void*)-1);
}

static void *aac_read_thread_8660(void *arg)
{
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd;
	int total_len;
	int fd = 0;
	struct dec_meta_out *meta_out_ptr;
	struct meta_out_8660_pb *meta_out_8660_pb;
	struct msm_audio_aio_buf aio_buf;
	struct msm_audio_config config;
#ifdef AUDIOV2
	unsigned short dec_id;
#endif
	unsigned int first_frame_offset, idx;
	unsigned int total_frame_size;
printf("*********************************\n");
printf("%s:fw=%d\n", __func__, file_write);
printf("*********************************\n");
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
		} else {
			pthread_mutex_lock(&aac_ref_lock);
			aac_read_buf_ref_cnt++;
			pthread_mutex_unlock(&aac_ref_lock);
		}
		meta_out_ptr = (struct dec_meta_out *)aio_op_buf[out_free_indx].buf_addr;
		meta_out_8660_pb = (struct meta_out_8660_pb *)(((char *)meta_out_ptr + sizeof(struct dec_meta_out)));
		printf("nr of frames %d\n", meta_out_ptr->num_of_frames);
#ifdef DEBUG_LOCAL
		printf("%s:msw ts 0x%8x, lsw_ts 0x%8x, nflags 0x%8x\n", __func__,
			meta_out_8660_pb->msw_ts,
			meta_out_8660_pb->lsw_ts,
			meta_out_8660_pb->nflags);
#endif
		first_frame_offset = meta_out_8660_pb->offset_to_frame;
		total_frame_size = 0;
		if(meta_out_ptr->num_of_frames != 0xFFFFFFFF) {
			// Go over all meta data field to find exact frame size
			for(idx=0; idx < meta_out_ptr->num_of_frames; idx++) { 
				total_frame_size +=  meta_out_8660_pb->frame_size;
				meta_out_8660_pb++;
			}
			printf("total size %d\n", total_frame_size);
		} else {
			//OutPut EOS reached
			if (meta_out_8660_pb->nflags == EOS) {
				printf("%s:Received EOS at output port 0x%8x\n", __func__,
				meta_out_8660_pb->nflags);
				break;
			}
		}
		printf("%s: Read Size %d offset %d\n", __func__,
			total_frame_size, first_frame_offset);
		write(fd, ((char *)aio_op_buf[out_free_indx].buf_addr + first_frame_offset + sizeof(struct dec_meta_out)),
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

static void *aac_write_thread_8660(void *arg)
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
			sz = fill_buffer_8660_pb(aio_buf.buf_addr, in_size, audio_data);
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
			printf("%s:ASYNC_WRITE addr %p len %d, filled_sz = %d\n", __func__,
				aio_buf.buf_addr, aio_buf.data_len, sz);
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
	// Free memory created during aac bin file 
        free(audio_data->org_next);
	pthread_exit(NULL);
	return NULL;
}

static void *aac_dec_event_8660(void *arg)
{
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) arg;
	int afd = audio_data->afd, rc;
	struct msm_audio_event event;
	int eof = 0;
	struct dec_meta_out *meta_out_ptr;
	struct meta_out_8660_pb *meta_out_8660_pb;
	struct meta_in *meta_in_ptr;
	pthread_t evt_read_thread;
	pthread_t evt_write_thread;

	eos_ack = 0;
	if (audio_data->mode) // Non Tunnel mode
		pthread_create(&evt_read_thread, NULL, aac_read_thread_8660, (void *) audio_data);
	pthread_create(&evt_write_thread, NULL, aac_write_thread_8660, (void *) audio_data);
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
				if (event.event_payload.aio_buf.data_len >= 0) {
		  			printf("%s: READ_DONE: addr %p len %d\n", __func__,
						event.event_payload.aio_buf.buf_addr,
						event.event_payload.aio_buf.data_len);
					meta_out_ptr = (struct dec_meta_out *)event.event_payload.aio_buf.buf_addr;
					out_data_indx =(int) event.event_payload.aio_buf.private_data;
					meta_out_8660_pb = (struct meta_out_8660_pb *)(((char *)meta_out_ptr + sizeof(struct dec_meta_out)));
					if (aac_read_buf_ref_cnt) {
						pthread_mutex_lock(&aac_ref_lock);
						aac_read_buf_ref_cnt--;
						pthread_mutex_unlock(&aac_ref_lock);
					}
					//OutPut EOS reached
					if (meta_out_8660_pb->nflags == EOS) {
			  			eof = 1;
						printf("%s:Received EOS event at output 0x%8x\n", __func__,
						meta_out_8660_pb->nflags);
					}
					data_available();
				} else {
					printf("%s:AUDIO_EVENT_READ_DONE:unexpected length = %d\n",
						   __func__, event.event_payload.aio_buf.data_len);
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
			case AUDIO_EVENT_STREAM_INFO:
				{
				printf("aac_dec_event_8660: STREAM_INFO EVENT FROM DRIVER\n");
				printf("chan_info : %d, sample_rate : %d\n",
					   event.event_payload.stream_info.chan_info,
					   event.event_payload.stream_info.sample_rate);
#ifdef AUDIOV2
				if (audio_data->mode) {
					audio_data->outport_flush_enable = 1;
					ioctl(afd, AUDIO_OUTPORT_FLUSH, 0);
					audio_data->outport_flush_enable = 0;
					printf("aac_dec_event_8660: outport flush complete. "
						   "trigger async_read\n");
					if (aac_read_buf_ref_cnt == 0) {
						data_available();
					}
				}
#endif
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


static int aac_start_8660(struct audtest_config *clnt_config)
{
        struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
	unsigned n = 0;
	pthread_t evt_thread;
	int sz;
	int rc = -1;
#ifdef AUDIOV2
	int dec_id;
#endif
	int afd;
	struct msm_audio_aio_buf aio_buf;
	struct msm_audio_buf_cfg buf_cfg;
	struct msm_audio_config config;
	struct msm_audio_aac_config aac_config;
	unsigned int open_flags;
	struct mmap_info *in_ion_buf[AACTEST_NUM_IBUF];
	struct mmap_info *out_ion_buf[AACTEST_NUM_OBUF];

	if(((in_size + sizeof(struct meta_in)) > AACTEST_IBUFSZ) ||
		(out_size > AACTEST_OBUFSZ)) {
			perror("configured input / output size more"\
			"than ION allocation");
			return -1; 
	}

	if (audio_data->mode)
		open_flags = O_RDWR | O_NONBLOCK;
	else
		open_flags = O_WRONLY | O_NONBLOCK;

	if ((aac_channels == 1) || (aac_channels == 2)) {
		afd = open("/dev/msm_aac", open_flags);
	} else if (aac_channels == 6) {
		afd = open("/dev/msm_multi_aac", open_flags);
	} else
		perror(" Invalid AAC Channels");

	if (afd < 0) {
		perror("Cannot open AAC device");
		return -1;
	}

	audio_data->afd = afd; /* Store */

	setup_aac_file(clnt_config);
	if (audio_data->mode) {
		/* PCM config */
		if (ioctl(afd, AUDIO_GET_CONFIG, &config)) {
			perror("could not get config");
			goto err_state1;
		}
		config.sample_rate = clnt_config->sample_rate;
		config.channel_count = clnt_config->channel_mode;

		if (ioctl(afd, AUDIO_SET_CONFIG, &config)) {
			perror("could not set config");
			goto err_state1;
		}
		printf("%s:config sample_rate=%d channels=%d bitspersample=%d \n",
			__func__,
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
	aac_config.format = clnt_config->fmt_config.aac.format_type;
	aac_config.audio_object = clnt_config->fmt_config.aac.object_type;
	aac_config.sbr_on_flag = clnt_config->fmt_config.aac.sbr_flag;
	aac_config.sbr_ps_on_flag = clnt_config->fmt_config.aac.sbr_ps_flag;
	aac_config.channel_configuration = aac_channels;
	aac_config.sample_rate = clnt_config->sample_rate;

	if (ioctl(afd, AUDIO_SET_AAC_CONFIG, &aac_config)) {
		perror("could not set AUDIO_SET_AAC_CONFIG_V2");
		goto err_state2;
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
	pthread_mutex_init(&aac_ref_lock, 0);
	data_is_available = 0;
	data_is_consumed = 0;
	in_free_indx=0;
	out_free_indx=0;
	if ((ioctl(afd, AUDIO_START, 0))< 0 ) {
	   		printf("aactest: unable to start driver\n");
			goto err_state2;
	}
	else printf("%s: AUDIO_START success\n", __func__);
	if (audio_data->mode) {
		/* non - tunnel portion */
		printf(" selected non-tunnel part\n");
		// Register read buffers
		for (n = 0; n < AACTEST_NUM_OBUF; n++) {
			out_ion_buf[n] = alloc_ion_buffer(ionfd, AACTEST_OBUFSZ);
			if (!out_ion_buf[n]) {
                                printf("\n alloc_ion_buffer: out_ion_buf[n] allocation failed\n");
                                goto err_state2;
                        }

			rc = audio_register_ion(afd, out_ion_buf[n]);
                        if (-1 == rc) {
                                printf("\n audio_register_ion: out_ion_buf[n] failed\n");
				free_ion_buffer(ionfd, &out_ion_buf[n]);
                                goto err_state2;
                        }
			// Read buffers local structure
                        aio_op_buf[n].buf_addr = out_ion_buf[n]->pBuffer;
			aio_op_buf[n].buf_len = out_size;
			aio_op_buf[n].data_len = 0; // Driver will notify actual size 
			aio_op_buf[n].private_data = (void *)n; //Index
		}
		// Send n-1 Read buffer
		for (n = 0; n < (AACTEST_NUM_OBUF-1); n++) {
		 	aio_buf.buf_addr = aio_op_buf[n].buf_addr;
			aio_buf.buf_len = aio_op_buf[n].buf_len;
			aio_buf.data_len = aio_op_buf[n].data_len; 
			aio_buf.private_data = aio_op_buf[n].private_data;
			printf("ASYNC_READ addr %p len %d\n", aio_buf.buf_addr,
				aio_buf.buf_len);
			if (ioctl(afd, AUDIO_ASYNC_READ, &aio_buf) < 0) {
				printf("error on async read\n");
				goto err_state2;
			} else {
				pthread_mutex_lock(&aac_ref_lock);
				aac_read_buf_ref_cnt++;
				pthread_mutex_unlock(&aac_ref_lock);
			}

		}
		//Indicate available free buffer as (n-1)
		out_free_indx = AACTEST_NUM_OBUF-1;
	}
	//Register Write  buffer
	for (n = 0; n < AACTEST_NUM_IBUF; n++) {
		in_ion_buf[n] = alloc_ion_buffer(ionfd, AACTEST_IBUFSZ);
		if (!in_ion_buf[n]) {
			printf("\n alloc_ion_buffer: in_ion_buf[%d] allocation failed\n", n);
			goto err_state2;
		}

		rc = audio_register_ion(afd, in_ion_buf[n]);
		if (-1 == rc) {
			printf("\n audio_register_ion: in_ion_buf[%d] failed\n", n);
			free_ion_buffer(ionfd, &in_ion_buf[n]);
			goto err_state2;
                }
		// Write buffers local structure
	 	aio_ip_buf[n].buf_addr = in_ion_buf[n]->pBuffer;
		aio_ip_buf[n].buf_len = (AACTEST_IBUFSZ + 4095) & (~4095);;
		aio_ip_buf[n].data_len = 0; // Driver will notify actual size 
		aio_ip_buf[n].private_data = (void *)n; //Index
	}
	// Send n-1 write buffer
	for (n = 0; n < (AACTEST_NUM_IBUF-1); n++) {
	 	aio_buf.buf_addr = aio_ip_buf[n].buf_addr;
		aio_buf.buf_len = aio_ip_buf[n].buf_len;
		if ((sz = fill_buffer_8660_pb(aio_buf.buf_addr, in_size, audio_data)) < 0)
			goto err_state2;
		aio_buf.data_len = sz;
		aio_ip_buf[n].data_len = sz; 
		aio_buf.private_data = aio_ip_buf[n].private_data;
		printf("ASYNC_WRITE addr %p len = %d, filled_sz = %d\n", aio_buf.buf_addr,
			aio_buf.data_len, sz);
		rc = ioctl(afd, AUDIO_ASYNC_WRITE, &aio_buf);
		if(rc < 0) {
			printf( "error on async write=%d\n",rc);
			goto err_state2;
		}
	}
	//Indicate available free buffer as (n-1)
	in_free_indx = AACTEST_NUM_IBUF-1;
	pthread_create(&evt_thread, NULL, aac_dec_event_8660, (void *) audio_data);
	pthread_join(evt_thread, NULL);
	printf("AUDIO_STOP as event thread completed\n");
done:
	rc = 0;
	ioctl(afd, AUDIO_STOP, 0);
err_state2:
	if (audio_data->mode) {
		for (n = 0; n < AACTEST_NUM_OBUF; n++) {
			free_ion_buffer(ionfd, &out_ion_buf[n]);
		}
	}
	for (n = 0; n < AACTEST_NUM_IBUF; n++) {
		free_ion_buffer(ionfd, &in_ion_buf[n]);
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

const char *aacplay_help_txt =
"Play aac file: type \n \
echo \"playaac path_of_file -type=xxxx -repeat=x -rate=xxxx -cmode=x \
-aac_channels=x -profile=xxx -id=xxx -mode=x -bitstream=xxx -err_thr=x -tgt = x\
-out=path_of_outfile\" > %s \n \
Sample rate of AAC file <= 48000 \n \
tgt: 08 - for 8660 target, default 7k target \n \
Type: adts, raw, loas, praw, adif  \n \
 Type needs to be set to praw when bitstream is converted to\n \
 psuedo raw format as required by DSP. adif only supported for 8660 and 8960 \n \
Channel mode(no. of channels for PCM) 1 or 2 \n \
aac_channels(no. of channels for AAC) 1 or 2 or 6(for AAC 5.1) \n \
Profile aac, aac+, eaac+ \n \
Mode 1 (Non-Tunneled) and 0 (Tunneled) \n \
Bitstream lc, ltp, erlc or bsac \n \
Error threshold value 0 to 0x7fff \n \
Repeat 'x' no. of times, repeat infinitely if repeat = 0\n\
Supported control command: pause, resume, flush, volume, quit \n\ 
examples: \n\
echo \"playaac path_of_file -id=xxx -mode=<0 or 1>\" > %s \n\
echo \"control_cmd -id=xxx -cmd=pause\" > %s \n\
echo \"control_cmd -id=xxx -cmd=resume\" > %s \n\
echo \"control_cmd -id=xxx -cmd=flush\" > %s \n\
echo \"control_cmd -id=xxx -cmd=volume -value=yyyy\" > %s \n";

void aacplay_help_menu(void) {
	printf("%s\n", aacplay_help_txt);
}

const char *aacrec_help_txt =
"Record aac file: type \n\
echo \"recaac -infile=path_of_file -outfile=path_of_file \
-id=xxx -rate=xxxx -channel=x -mode=x -bps=xx -frames=xxx -type=xxx \
-tgt=xxx -aac_type=xxxx\" > %s \n\
Sample rate of source <= 48000 \n \
Channel mode 1 or 2 \n \
bps: bit per second 64k to 384k \n \
frames: number of frames per buffer valid for 8660 \n \
tgt :08 for 8660, by default target type set to 7k \n \
type: AAC format types, adts or raw \n \
aac_type: AAC object types, give any one of below strings \n \
aac_lc or aac_plus or eaac_plus \n \
Supported control command: N/A \n ";

void aacrec_help_menu(void) {
	printf(aacrec_help_txt, cmdfile);
}

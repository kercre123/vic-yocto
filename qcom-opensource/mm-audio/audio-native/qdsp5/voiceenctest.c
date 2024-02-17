/* voiceenctest.c - native voice encoder test application
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2009-2011, The Linux Foundation. All rights reserved.
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
#include <linux/msm_audio_qcp.h>
#include <linux/msm_audio_amrnb.h>
#include <linux/msm_audio_amrwb.h>
#include <pthread.h>
#include <errno.h>
#include "audiotest_def.h"

#define QCELP_DEVICE_NODE "/dev/msm_qcelp_in"
#define EVRC_DEVICE_NODE "/dev/msm_evrc_in"
#define AMRNB_DEVICE_NODE "/dev/msm_amrnb_in"
#define AMRWB_DEVICE_NODE "/dev/msm_amrwb_in"

struct qcp_header {
	/* RIFF Section */
	char riff[4];
	unsigned int s_riff;
	char qlcm[4];

	/* Format chunk */
	char fmt[4];
        unsigned int s_fmt;
        char mjr;
        char mnr;
        unsigned int data1;         /* UNIQUE ID of the codec */
        unsigned short data2;
        unsigned short data3;
        char data4[8];
        unsigned short ver;         /* Codec Info */
        char name[80];
        unsigned short abps;    /* average bits per sec of the codec */
        unsigned short bytes_per_pkt;
        unsigned short samp_per_block;
        unsigned short samp_per_sec;
        unsigned short bits_per_samp;
        unsigned char vr_num_of_rates;         /* Rate Header fmt info */
        unsigned char rvd1[3];
        unsigned short vr_bytes_per_pkt[8];
        unsigned int rvd2[5];

	/* Vrat chunk */
        unsigned char vrat[4];
        unsigned int s_vrat;
        unsigned int v_rate;
        unsigned int size_in_pkts;

	/* Data chunk */
        unsigned char data[4];
        unsigned int s_data;
} __attribute__ ((packed));

/* Common part */
static struct qcp_header append_header = {
        {'R', 'I', 'F', 'F'}, 0, {'Q', 'L', 'C', 'M'}, /* Riff */
        {'f', 'm', 't', ' '}, 150, 1, 0, 0, 0, 0,{0}, 0, {0},0,0,160,8000,16,0,{0},{0},{0}, /* Fmt */
        {'v','r','a','t'}, 0, 0, 0, /* Vrat */
        {'d','a','t','a'},0 /* Data */
        };

static char amr_header[6] = { '#','!','A','M','R','\n'};

#define QCP_HEADER_SIZE sizeof(struct qcp_header)
#define AMR_HEADER_SIZE sizeof(amr_header)

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

static int rec_type; // record type
static int rec_stop;
static int frame_format;
static int dtx_mode;
static int min_rate;
static int max_rate;
static int rec_source;
static int eos_sent = false;

static struct msm_audio_evrc_enc_config evrccfg;
static struct msm_audio_qcelp_enc_config qcelpcfg;
static struct msm_audio_amrnb_enc_config_v2 amrnbcfg_v2;
static struct msm_audio_amrnb_enc_config amrnbcfg;
static struct msm_audio_amrwb_enc_config amrwbcfg;

#ifdef _ANDROID_
static const char *cmdfile = "/data/audio_test";
#else
static const char *cmdfile = "/tmp/audio_test";
#endif

static uint8_t qcelp_pkt_size[5] = {0x00, 0x03, 0x07, 0x10, 0x22};
static uint8_t evrc_pkt_size[5] = {0x00, 0x02, 0x00, 0xa, 0x16};
static uint8_t amrnb_pkt_size[8] = {0x0c, 0x0d, 0x0f, 0x11, 0x13, 0x20, 0x1a, 0x1f};

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
	unsigned offset_to_frame;
        unsigned frame_size;
        unsigned encoded_pcm_samples;
        unsigned msw_ts;
        unsigned lsw_ts;
        unsigned nflags;
}__attribute__ ((packed));


struct meta_out_enc{
	unsigned char num_of_frames;
	struct meta_out meta_out_dsp[];
}__attribute__ ((packed));

#define NUM_BYTES_PER_SAMPLE	2

struct meta_in_7k{
	unsigned short offset;
	uint16_t time_stamp_dword_lsw;
	uint16_t time_stamp_dword_msw;
	uint16_t time_stamp_lsw;
	uint16_t time_stamp_msw;
	unsigned int nflags;
	unsigned short errflag;
	unsigned short sample_frequency;
	unsigned short channel;
	unsigned int tick_count;
} __attribute__ ((packed));

struct meta_out_7k{
	uint16_t metadata_len;
	uint16_t time_stamp_dword_lsw;
	uint16_t time_stamp_dword_msw;
	uint16_t time_stamp_lsw;
	uint16_t time_stamp_msw;
	uint16_t nflag_lsw;
	uint16_t nflag_msw;
};

static void create_qcp_header(int Datasize, int Frames)
{
	append_header.s_riff = Datasize + QCP_HEADER_SIZE - 8; /* exclude riff id and size field */
	if( rec_type  == 1 ) { /* QCELP 13K */
		printf("QCELP 13K header\n");
		append_header.data1 = 0x5E7F6D41;
		append_header.data2 = 0xB115;
		append_header.data3 = 0x11D0;
		append_header.data4[0] = 0xBA;
		append_header.data4[1] = 0x91;
		append_header.data4[2] = 0x00;
		append_header.data4[3] = 0x80;
		append_header.data4[4] = 0x5F;
		append_header.data4[5] = 0xB4;
		append_header.data4[6] = 0xB9;
		append_header.data4[7] = 0x7E;
		append_header.ver = 0x0002;
		memcpy(append_header.name, "Qcelp 13K", 9);
		append_header.abps = 13000;
		append_header.bytes_per_pkt = 35;
		append_header.vr_num_of_rates = 5;
		append_header.vr_bytes_per_pkt[0] = 0x0422;
		append_header.vr_bytes_per_pkt[1] = 0x0310;
		append_header.vr_bytes_per_pkt[2] = 0x0207;
		append_header.vr_bytes_per_pkt[3] = 0x0103;
		append_header.s_vrat = 0x00000008;
		append_header.v_rate = 0x00000001;
		append_header.size_in_pkts = Frames;
	} else if ( rec_type   == 2) { /* EVRC */
		printf("EVRC header\n");
		append_header.data1 = 0xe689d48d;
		append_header.data2 = 0x9076;
		append_header.data3 = 0x46b5;
		append_header.data4[0] = 0x91;
		append_header.data4[1] = 0xef;
		append_header.data4[2] = 0x73;
		append_header.data4[3] = 0x6a;
		append_header.data4[4] = 0x51;
		append_header.data4[5] = 0x00;
		append_header.data4[6] = 0xce;
		append_header.data4[7] = 0xb4;
		append_header.ver = 0x0001;
		memcpy(append_header.name, "TIA IS-127 Enhanced Variable Rate Codec, Speech Service Option 3", 64);
		append_header.abps = 9600;
		append_header.bytes_per_pkt = 23;
		append_header.vr_num_of_rates = 4;
		append_header.vr_bytes_per_pkt[0] = 0x0416;
		append_header.vr_bytes_per_pkt[1] = 0x030a;
		append_header.vr_bytes_per_pkt[2] = 0x0200;
		append_header.vr_bytes_per_pkt[3] = 0x0102;
		append_header.s_vrat = 0x00000008;
		append_header.v_rate = 0x00000001;
		append_header.size_in_pkts = Frames;
	} else if( rec_type == 3 ) { /* AMRNB */
		printf("AMRNB header\n");
		append_header.data1 = 0x6aa8e053;
		append_header.data2 = 0x474f;
		append_header.data3 = 0xbd46;
		append_header.data4[0] = 0x8a;
		append_header.data4[1] = 0xfa;
		append_header.data4[2] = 0xac;
		append_header.data4[3] = 0xf2;
		append_header.data4[4] = 0x32;
		append_header.data4[5] = 0x82;
		append_header.data4[6] = 0x73;
		append_header.data4[7] = 0xbd;
		append_header.ver = 0x0001;
		memcpy(append_header.name, "AMR-NB   ", 9);
		append_header.abps = 0x9c31;
		append_header.bytes_per_pkt = 32;
		append_header.vr_num_of_rates = 8;
		append_header.vr_bytes_per_pkt[0] = 0x081f;
		append_header.vr_bytes_per_pkt[1] = 0x071b;
		append_header.vr_bytes_per_pkt[2] = 0x0614;
		append_header.vr_bytes_per_pkt[3] = 0x0513;
		append_header.vr_bytes_per_pkt[4] = 0x0411;
		append_header.vr_bytes_per_pkt[4] = 0x040f;
		append_header.vr_bytes_per_pkt[5] = 0x030d;
		append_header.vr_bytes_per_pkt[6] = 0x020c;
		append_header.s_vrat = 0x00000008;
		append_header.v_rate = 0x00000001;
		append_header.size_in_pkts = Frames;
	}
	append_header.s_data = Datasize;
        return;
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
		memcpy(((char *)buf + sizeof(struct meta_in)), audio_data->next, cpy_size);
	} else

		memcpy(buf, audio_data->next, cpy_size);

	audio_data->next += cpy_size;
	audio_data->avail -= cpy_size;

	if (audio_data->mode) {
		return cpy_size + sizeof(struct meta_in);
	} else
		return cpy_size;
}
int  add_meta_in(char *buf, int eos, void *config, int buffer_size)
{
	static uint64_t duration = 0;
	struct meta_in_7k  metain;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config;
	memset(&metain,0, sizeof(metain));
	metain.offset = sizeof(metain);
	if(eos)
		duration = 0;
	printf("duration = %llu\n", duration);
	metain.time_stamp_dword_lsw = (duration >> 32) & 0x0000FFFF;
	metain.time_stamp_dword_msw = ((duration >> 32) & 0xFFFF0000) >> 16;
	metain.time_stamp_lsw = duration & 0x0000FFFF;
	metain.time_stamp_msw = (duration & 0xFFFF0000) >> 16;
	metain.nflags = eos;
	metain.sample_frequency = audio_data->freq;
	metain.channel = audio_data->channels;
	metain.tick_count = audio_data->frame_count;
	metain.errflag = 0;
	memcpy(buf, &metain, sizeof(metain));
	audio_data->frame_count++;
#ifdef DEBUG_LOCAL
	printf(" dmswts = %x dlswts = %x\n", metain.time_stamp_dword_msw, metain.time_stamp_dword_lsw);
	printf(" mswts = %x lswts = %x\n", metain.time_stamp_msw,metain.time_stamp_lsw);
#endif
	if (!eos)
		duration += (1000 * ((buffer_size * 1000  ) /
				(audio_data->freq * audio_data->channels
				 * NUM_BYTES_PER_SAMPLE)));

	return (sizeof(metain));
}
static int fill_buffer_7k(void *buf, unsigned sz, void *cookie)
{
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) cookie;
	unsigned cpy_size = 0;
	unsigned meta_size = 0;

	if (sz == 0) {
		return -1;
	}
	cpy_size = (sz < audio_data->avail ? sz : audio_data->avail);
	printf("cpy_size = %d audio_data->next = %p buf = %p\n", cpy_size, audio_data->next, buf);
	if (!audio_data->next) {
		printf("error in next buffer returning with out copying\n");
		return -1;
	}
	if (!eos_sent) {
		if(audio_data->avail == 0){
			eos_sent = 1;
		}
		meta_size = add_meta_in(buf, eos_sent, audio_data, cpy_size);
	}
	else {
		return -1;
	}

	if(cpy_size) {
		memcpy((char *)buf + meta_size, audio_data->next, cpy_size);
		audio_data->next += cpy_size;
		audio_data->avail -= cpy_size;
	}
	return (cpy_size + meta_size);
}

static void *voiceenc_nt_7k(void *arg)
{
	struct audtest_config *clnt_config= (struct audtest_config *)arg;
	struct meta_in meta;
	struct stat stat_buf;
	char *content_buf;
	int fd, ret_val = 0;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
	int afd = audio_data->afd;
	int len, total_len;
	len = 0;
	total_len = 0;
	size_t buffer_size;
	struct wav_header hdr;
        struct msm_audio_config config_pcm;
	eos_sent = 0;

	printf("voiceenc pcm write Thread\n");
	fd = open(clnt_config->in_file_name, O_RDONLY);
	if (fd < 0) {
		printf("Err while opening PCM file for encoder \
			: %s\n", clnt_config->in_file_name);
		pthread_exit((void *)ret_val);
	}
	(void) fstat(fd, &stat_buf);
	if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		printf("Cannot read file header\n");
		close(fd);
		pthread_exit((void *)ret_val);
	}
	printf("voiceenc_nt: %d ch, %d hz, 0x%4x bit\n",
			hdr.Number_Channels, hdr.Sample_rate, hdr.Significant_Bits_sample);

	printf("Total file size: %lld\n", stat_buf.st_size);
	buffer_size = stat_buf.st_size - sizeof(hdr);

	if (ioctl(afd, AUDIO_GET_CONFIG, &config_pcm)) {
		printf("Error getting AUDIO_GET_CONFIG\n");
		close(fd);
		pthread_exit((void *)ret_val);
	}
	printf("Default: config_pcm.buffer_count = %d , \
			config_pcm.buffer_size=%d \n", \
			config_pcm.buffer_count, config_pcm.buffer_size);

	audio_data->recsize = config_pcm.buffer_size;
	/*7k  voice encode only supports 8k mono */
	audio_data->freq = 8000;
	audio_data->channels = 1;

	audio_data->recbuf = (char *)malloc(audio_data->recsize);
	if (!audio_data->recbuf) {
	printf("could not allocate memory for pcm buffer\n");
		close(fd);
		pthread_exit((void *)ret_val);
	}
	memset(audio_data->recbuf, 0, audio_data->recsize);

	printf("Set: config_pcm.buffer_count = %d , \
			config_pcm.buffer_size=%d \n, \
			config_pcm.sample_rate=%d \n, \
			config_pcm.channel_count=%d \n", \
			config_pcm.buffer_count, config_pcm.buffer_size,
			config_pcm.sample_rate,config_pcm.channel_count);

	/* set back to witohut meta as the buffer corresponding to PCM */
        audio_data->recsize = audio_data->recsize - sizeof(struct meta_in_7k);
	audio_data->next = (char*)malloc(buffer_size);
	ioctl(afd, AUDIO_START, 0);
	printf("Total file PCM len: %d,next=%p\n", buffer_size, audio_data->next);

	if (!audio_data->next) {
                fprintf(stderr,"could not allocate %d bytes\n", buffer_size);
		close(fd);
		free(audio_data->recbuf);
		pthread_exit((void *)ret_val);
        }

	audio_data->org_next = audio_data->next;
	content_buf = audio_data->org_next;
	if (read(fd, audio_data->next, buffer_size) != (ssize_t)buffer_size) {
		fprintf(stderr,"could not read %d bytes\n", buffer_size);
		goto fail;
	}
	audio_data->avail = stat_buf.st_size;
	audio_data->org_avail = audio_data->avail;
	while (1) {
		len = fill_buffer_7k(
				audio_data->recbuf,
				audio_data->recsize,
				(void *)audio_data);
		printf("sz = %d\n", len);
		if (len < 0) {
			printf("end of file reached \n");
			break;
		} else {
			printf("fill buffer size = %d \n", len);
			len = write(afd, audio_data->recbuf, len);
		}
	}
fail:
	close(fd);
	free(content_buf);
	free(audio_data->recbuf);
	pthread_exit((void *)ret_val);
	return NULL;
}


static void *voiceenc_nt(void *arg)
{
	struct audtest_config *clnt_config= (struct audtest_config *)arg;
	struct meta_in meta;
	struct stat stat_buf;
	char *content_buf;
	int fd, ret_val = 0;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
	int afd = audio_data->afd;
	int len, total_len;
	len = 0;
	total_len = 0;
	size_t buffer_size;
	struct wav_header hdr;
        struct msm_audio_config config_pcm;

	printf("voiceenc pcm write Thread\n");
	fd = open(clnt_config->in_file_name, O_RDONLY);
	if (fd < 0) {
		printf("Err while opening PCM file for encoder \
			: %s\n", clnt_config->in_file_name);
		pthread_exit((void *)ret_val);
	}
	(void) fstat(fd, &stat_buf);

	if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		printf("Cannot read file header\n");
		close(fd);
		pthread_exit((void *)ret_val);
	}
	printf("voiceenc_nt: %d ch, %d hz, 0x%4x bit\n",
			hdr.Number_Channels, hdr.Sample_rate, hdr.Significant_Bits_sample);

	printf("Total file size: %lld\n", stat_buf.st_size);
	buffer_size = stat_buf.st_size - sizeof(hdr);

	if (ioctl(afd, AUDIO_GET_CONFIG, &config_pcm)) {
               printf("Error getting AUDIO_GET_CONFIG\n");
		close(fd);
		pthread_exit((void *)ret_val);
	}
        printf("Default: config_pcm.buffer_count = %d , \
                config_pcm.buffer_size=%d \n", \
                config_pcm.buffer_count, config_pcm.buffer_size);

	audio_data->recsize = config_pcm.buffer_size;

        config_pcm.channel_count = hdr.Number_Channels;
	config_pcm.sample_rate	 = hdr.Sample_rate;

        audio_data->recbuf = (char *)malloc(audio_data->recsize);
        if (!audio_data->recbuf) {
                printf("could not allocate memory for pcm buffer\n");
		close(fd);
		pthread_exit((void *)ret_val);
	}
        memset(audio_data->recbuf, 0, audio_data->recsize);

	if (ioctl(afd, AUDIO_SET_CONFIG, &config_pcm)) {
		printf("could not set PCM config\n");
		close(fd);
		free(audio_data->recbuf);
		pthread_exit((void *)ret_val);
	}

	if (ioctl(afd, AUDIO_GET_CONFIG, &config_pcm)) {
                printf("Error getting AUDIO_GET_PCM_CONFIG\n");
		close(fd);
		free(audio_data->recbuf);
		pthread_exit((void *)ret_val);
	}
        printf("Set: config_pcm.buffer_count = %d , \
                config_pcm.buffer_size=%d \n", \
                config_pcm.buffer_count, config_pcm.buffer_size);

	/* set back to witohut meta as the buffer corresponding to PCM */
        audio_data->recsize = audio_data->recsize - sizeof(struct meta_in);
	ioctl(afd, AUDIO_START, 0);
	audio_data->next = (char*)malloc(buffer_size);
	printf("Total file PCM len: %d,next=%p\n", buffer_size, audio_data->next);

	if (!audio_data->next) {
                fprintf(stderr,"could not allocate %d bytes\n", buffer_size);
		close(fd);
		free(audio_data->recbuf);
		pthread_exit((void *)ret_val);
        }

	audio_data->org_next = audio_data->next;
	content_buf = audio_data->org_next;
	if (read(fd, audio_data->next, buffer_size) != (ssize_t)buffer_size) {
		fprintf(stderr,"could not read %d bytes\n", buffer_size);
		goto fail;
	}
	audio_data->avail = stat_buf.st_size;
	audio_data->org_avail = audio_data->avail;
	do {
		if((len = fill_buffer(audio_data->recbuf, audio_data->recsize, audio_data)) < 0) {
			printf("end of file reached \n");
			break;
		} else {
			printf("fill buffer size = %d \n", len);
			len = write(afd, audio_data->recbuf, len);
		}
	} while (1);
	/* End of file, send EOS */
	if(audio_data->mode) {
		meta.ntimestamp.LowPart = ((audio_data->frame_count * 20000) & 0xFFFFFFFF);
		meta.ntimestamp.HighPart = (((unsigned long long)(audio_data->frame_count
						* 20000) >> 32) & 0xFFFFFFFF);
		meta.offset = sizeof(struct meta_in);
		meta.nflags = 0x01;
		memcpy(audio_data->recbuf, &meta, sizeof(struct meta_in));
		write(afd, audio_data->recbuf, sizeof(struct meta_in));
		printf("Sent EOS on input buffer\n");
	}
fail:
	close(fd);
	free(content_buf);
	free(audio_data->recbuf);
	pthread_exit((void *)ret_val);
	return NULL;
}

static int voiceenc_start(struct audtest_config *clnt_config)
{
	int afd, fd;
        unsigned char buf[1024];
        unsigned sz;
	int readcnt,writecnt,open_flags;
	struct msm_audio_stream_config cfg;
	struct msm_audio_stats stats;
	int datasize=0, framecnt=0;
	unsigned short enc_id;
	pthread_t thread;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
	memset(&stats,0,sizeof(stats));
	memset(&cfg,0,sizeof(cfg));

	fd = open(clnt_config->file_name, O_CREAT | O_RDWR, 0666);

	if (fd < 0) {
		printf("Unable to create output file = %s\n",
			clnt_config->file_name);
		goto file_err;
	}
	else
		printf("file created =%s\n",clnt_config->file_name);

	if (clnt_config->mode)
		open_flags = O_RDWR;
	else
		open_flags = O_RDONLY;

	/* Open Device 	Node */
	if (rec_type == 1) {
		afd = open(QCELP_DEVICE_NODE, open_flags);
	} else if (rec_type == 2) {
		afd = open(EVRC_DEVICE_NODE, open_flags);
	} else if (rec_type == 3) {
		afd = open(AMRNB_DEVICE_NODE, open_flags);
	} else
		goto device_err;

	if (afd < 0) {
		printf("Unable to open audio device = %s\n",
			(rec_type == 1? QCELP_DEVICE_NODE:(rec_type == 2? \
				EVRC_DEVICE_NODE: AMRNB_DEVICE_NODE)));
		goto device_err;
	}

#ifndef AUDIO7X27A
	if (ioctl(afd, AUDIO_GET_SESSION_ID, &enc_id)) {
		perror("could not get encoder id\n");
		close(fd);
		close(afd);
		return -1;
	}
	if (!clnt_config->mode) {
		if (devmgr_register_session(enc_id, DIR_TX) < 0) {
			close(afd);
			goto fail;
		}
	}
#endif
	/* Config param */
	if(ioctl(afd, AUDIO_GET_STREAM_CONFIG, &cfg)) {
		printf(" Error getting buf config param AUDIO_GET_CONFIG \n");
		goto fail;
	}

	if(ioctl(afd, AUDIO_SET_STREAM_CONFIG, &cfg)) {
		printf(" Error getting buf config param AUDIO_GET_CONFIG \n");
		goto fail;
	}
	printf("Default buffer size = 0x%8x\n", cfg.buffer_size);
	printf("Default buffer count = 0x%8x\n",cfg.buffer_count);

	if (rec_type == 1) {
		if (ioctl(afd, AUDIO_GET_QCELP_ENC_CONFIG, &qcelpcfg)) {
			printf("Error: AUDIO_GET_QCELP_ENC_CONFIG failed\n");
			goto fail;
		}
		printf("cdma rate = 0x%8x\n", qcelpcfg.cdma_rate);
		printf("min_bit_rate = 0x%8x\n", qcelpcfg.min_bit_rate);
		printf("max_bit_rate = 0x%8x\n", qcelpcfg.max_bit_rate);
		qcelpcfg.cdma_rate = max_rate;
		qcelpcfg.min_bit_rate = min_rate;
		qcelpcfg.max_bit_rate = max_rate;
		if (ioctl(afd, AUDIO_SET_QCELP_ENC_CONFIG, &qcelpcfg)) {
			printf("Error: AUDIO_SET_QCELP_ENC_CONFIG failed\n");
			goto fail;
		}
		printf("cdma rate = 0x%8x\n", qcelpcfg.cdma_rate);
		printf("min_bit_rate = 0x%8x\n", qcelpcfg.min_bit_rate);
		printf("max_bit_rate = 0x%8x\n", qcelpcfg.max_bit_rate);
	} else if(rec_type == 2) {

		if (ioctl(afd, AUDIO_GET_EVRC_ENC_CONFIG, &evrccfg)) {
			printf("Error: AUDIO_GET_EVRC_ENC_CONFIG failed\n");
			goto fail;
		}
		printf("cdma rate = 0x%8x\n", evrccfg.cdma_rate);
		printf("min_bit_rate = 0x%8x\n", evrccfg.min_bit_rate);
		printf("max_bit_rate = 0x%8x\n", evrccfg.max_bit_rate);
		evrccfg.cdma_rate = max_rate;
		evrccfg.min_bit_rate = min_rate;
		evrccfg.max_bit_rate = max_rate;
		if (ioctl(afd, AUDIO_SET_EVRC_ENC_CONFIG, &evrccfg)) {
			printf("Error: AUDIO_GET_EVRC_ENC_CONFIG failed\n");
			goto fail;
		}
		printf("cdma rate = 0x%8x\n", evrccfg.cdma_rate);
		printf("min_bit_rate = 0x%8x\n", evrccfg.min_bit_rate);
		printf("max_bit_rate = 0x%8x\n", evrccfg.max_bit_rate);
	} else if (rec_type == 3) {
	#ifdef AUDIO7X27A
		/* AMRNB specific settings */
		if (ioctl(afd, AUDIO_GET_AMRNB_ENC_CONFIG, &amrnbcfg)) {
			perror("Error: AUDIO_GET_AMRNB_ENC_CONFIG failed");
			goto fail;
		}
		printf("dtx mode = 0x%8x\n", amrnbcfg.dtx_mode_enable);
		printf("rate  = 0x%8x\n", amrnbcfg.enc_mode);
		amrnbcfg.dtx_mode_enable = dtx_mode; /* 0 - DTX off, 1 - DTX on */
		amrnbcfg.enc_mode = max_rate;
		if (ioctl(afd, AUDIO_SET_AMRNB_ENC_CONFIG, &amrnbcfg)) {
			perror("Error: AUDIO_SET_AMRNB_ENC_CONFIG failed");
			goto fail;
		}
		printf("dtx mode = 0x%8x\n", amrnbcfg.dtx_mode_enable);
		printf("rate  = 0x%8x\n", amrnbcfg.enc_mode);
	#else
		/* AMRNB specific settings */
		if (ioctl(afd, AUDIO_GET_AMRNB_ENC_CONFIG_V2, &amrnbcfg_v2)) {
			perror("Error: AUDIO_GET_AMRNB_ENC_CONFIG_V2 failed");
			goto fail;
		}
		printf("dtx mode = 0x%8x\n", amrnbcfg_v2.dtx_enable);
		printf("rate = 0x%8x\n", amrnbcfg_v2.band_mode);
		amrnbcfg_v2.dtx_enable = dtx_mode; /* 0 - DTX off, 1 - DTX on */
		amrnbcfg_v2.band_mode = max_rate;
		if (ioctl(afd, AUDIO_SET_AMRNB_ENC_CONFIG_V2, &amrnbcfg_v2)) {
			perror("Error: AUDIO_GET_AMRNB_ENC_CONFIG_V2 failed");
			goto fail;
		}
		printf("dtx mode = 0x%8x\n", amrnbcfg_v2.dtx_enable);
		printf("rate = 0x%8x\n", amrnbcfg_v2.band_mode);
	#endif
	}

	#ifndef AUDIO7X27A
	/* Record form voice link */
	if (rec_source <= VOC_REC_BOTH ) {

		if (ioctl(afd, AUDIO_SET_INCALL, &rec_source)) {
			perror("Error: AUDIO_SET_INCALL failed");
			goto fail;
		}
		printf("rec source = 0x%8x\n", rec_source);
	}
	#endif
	/* Store handle for commands pass*/
	audio_data->afd = afd;
	sz = cfg.buffer_size;

	if (clnt_config->mode) {
		/* non - tunnel portion for 7k */
		pthread_create(&thread, NULL, voiceenc_nt_7k, (void *)clnt_config);
	} else
		ioctl(afd, AUDIO_START, 0);

	printf("Voice encoder started 7k\n");

	if(frame_format == 1) { /* QCP file */
	        lseek(fd, QCP_HEADER_SIZE, SEEK_SET);
		printf("qcp_headsize %d\n",QCP_HEADER_SIZE);
		printf("QCP format\n");
	} else
		printf("DSP format\n");
	rec_stop = 0;
	while(!rec_stop) {
		memset(buf,0,sz);
		readcnt = read(afd, buf, sz);
		if (readcnt <= 0) {
			printf("cannot read buffer error code =0x%8x", readcnt);
			goto fail;
		}
		else
		{
			unsigned char *memptr = buf;
			int metasize = 0;
			printf("read cnt %d\n",readcnt);

			if(clnt_config->mode)
			{
				struct meta_out_7k meta_out;
				metasize = sizeof(struct meta_out_7k);
				memcpy(&meta_out, buf, metasize);
				memptr += metasize;
		#ifdef DEBUG_LOCAL
				printf(" dmswts = %x dlswts = %x\n",
					metain.time_stamp_dword_msw,
					metain.time_stamp_dword_lsw);
				printf(" mswts = %x lswts = %x\n",
					meta_out.time_stamp_msw,
					meta_out.time_stamp_lsw);
				printf("nflags_lsw = %d, nflags_msw = %d\n",
					meta_out.nflag_lsw,
					meta_out.nflag_msw);
		#endif
				readcnt -= metasize;
				if(meta_out.nflag_lsw & 0x1)
				{
					printf("output eos received \n");
					break;
				}

			}
			/* QCP Format */
			if( frame_format == 1) {
				// logic for qcp generation
				if (rec_type ==  1) {
					if (buf[metasize+1] <= 4 || buf[metasize+1] >=1) {
						readcnt = qcelp_pkt_size[buf[metasize+1]] + 1;
						memptr = &buf[metasize+1];
						printf("0x%2x, %d\n", buf[metasize+1], readcnt);
					} else
						printf("Unexpected frame\n");
				} else if(rec_type == 2) {
					if ((buf[metasize+1] <= 4 || buf[metasize+1] >=1) && (buf[metasize+1] != 2)) {
						readcnt = evrc_pkt_size[buf[metasize+1]] + 1;
						memptr = &buf[metasize+1];
						printf("0x%2x, %d\n", buf[metasize+1], readcnt);
					} else
						printf("Unexpected frame\n");
				} else if(rec_type == 3) {
					if (buf[metasize+1] <= 7) {
						readcnt = amrnb_pkt_size[buf[metasize+1]] + 1;
						memptr = &buf[metasize+1];
						printf("0x%2x, %d\n", buf[metasize+1], readcnt);
					} else
						printf("Unexpected frame\n");
				}
			}
			writecnt = write(fd, memptr, readcnt);
			if (writecnt <= 0) {
				printf("cannot write buffer error code =0x%8x", writecnt);
				goto fail;
			}
		}
		framecnt++;
		datasize += writecnt;
        }
	ioctl(afd, AUDIO_GET_STATS, &stats);
	printf("\n read_bytes = %d, read_frame_counts = %d\n",datasize, framecnt);
	ioctl(afd, AUDIO_STOP, 0);
	if(frame_format == 1) { /* QCP file */
		create_qcp_header(datasize, framecnt);
	        lseek(fd, 0, SEEK_SET);
		write(fd, (char *)&append_header, QCP_HEADER_SIZE);
	}

	printf("Secondary encoder stopped \n");
	close(afd);

#ifndef AUDIO7X27A
	if (!clnt_config->mode) {
		if (devmgr_unregister_session(enc_id, DIR_TX) < 0) {
			perror("\ncould not unregister recording session\n");
		}
	}
#endif
	return 0;
fail:
	close(afd);

#ifndef AUDIO7X27A
	if (!clnt_config->mode) {
		if (devmgr_unregister_session(enc_id, DIR_TX) < 0) {
			perror("\ncould not unregister recording session\n");
		}
	}
#endif
device_err:
	close(fd);
	unlink(clnt_config->file_name);
file_err:
	return -1;
}

static int voiceenc_start_8660(struct audtest_config *clnt_config)
{
	int afd, fd;
        unsigned char buf[1024];
        unsigned sz;
	int readcnt,writecnt;
	struct msm_audio_stream_config cfg;
	struct msm_audio_stats stats;
	struct msm_audio_buf_cfg buf_cfg;
	int datasize=0, framecnt=0;
	unsigned short enc_id;
	unsigned int open_flags;
	pthread_t thread;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) clnt_config->private_data;
	memset(&stats,0,sizeof(stats));
	memset(&cfg,0,sizeof(cfg));

	fd = open(clnt_config->file_name, O_CREAT | O_RDWR, 0666);

	if (fd < 0) {
		printf("Unable to create output file = %s\n",
			clnt_config->file_name);
		goto file_err;
	}
	else
		printf("file created =%s\n",clnt_config->file_name);

	if (clnt_config->mode)
		open_flags = O_RDWR;
	else
		open_flags = O_RDONLY;

	/* Open Device 	Node */
	if (rec_type == 1) {
			afd = open(QCELP_DEVICE_NODE, open_flags);
	} else if (rec_type == 2) {
			afd = open(EVRC_DEVICE_NODE, open_flags);
	} else if (rec_type == 3) {
			afd = open(AMRNB_DEVICE_NODE, open_flags);
	} else if (rec_type == 4) {
			afd = open(AMRWB_DEVICE_NODE, open_flags);
	} else
		goto device_err;

	if (afd < 0) {
		printf("Unable to open audio device = %s in mode %d\n",
			(rec_type == 1? QCELP_DEVICE_NODE:(rec_type == 2? \
				EVRC_DEVICE_NODE:(rec_type == 3) ? \
				AMRNB_DEVICE_NODE:AMRWB_DEVICE_NODE)), clnt_config->mode);
		goto device_err;
	}

#ifndef AUDIO7X27A
	if (ioctl(afd, AUDIO_GET_SESSION_ID, &enc_id)) {
		perror("could not get encoder id\n");
		close(fd);
		close(afd);
		return -1;
	}
	if(!clnt_config->mode)
		if (devmgr_register_session(enc_id, DIR_TX) < 0) {
			close(fd);
			close(afd);
			return -1;
		}
#endif
	/* Config param */
	if(ioctl(afd, AUDIO_GET_STREAM_CONFIG, &cfg)) {
		printf("Error getting AUDIO_GET_STREAM_CONFIG\n");
		goto fail;
	}
	printf("Default buffer size = 0x%8x\n", cfg.buffer_size);
	printf("Default buffer count = 0x%8x\n",cfg.buffer_count);

	if(ioctl(afd, AUDIO_SET_STREAM_CONFIG, &cfg)) {
		printf("Error setting AUDIO_SET_STREAM_CONFIG\n");
		goto fail;
	}
	/* Config param */
	if(ioctl(afd, AUDIO_GET_BUF_CFG, &buf_cfg)) {
		printf("Error getting AUDIO_GET_BUF_CONFIG\n");
		goto fail;
	}
	printf("Default meta_info_enable = 0x%8x\n", buf_cfg.meta_info_enable);
	printf("Default frames_per_buf = 0x%8x\n", buf_cfg.frames_per_buf);
	buf_cfg.frames_per_buf = clnt_config->frames_per_buf;
	if(ioctl(afd, AUDIO_SET_BUF_CFG, &buf_cfg)) {
		printf("Error setting AUDIO_SET_BUF_CONFIG\n");
		goto fail;
	}
	if (rec_type == 1) {
		if (ioctl(afd, AUDIO_GET_QCELP_ENC_CONFIG, &qcelpcfg)) {
			printf("Error: AUDIO_GET_QCELP_ENC_CONFIG failed\n");
			goto fail;
		}
		printf("cdma rate = 0x%8x\n", qcelpcfg.cdma_rate);
		printf("min_bit_rate = 0x%8x\n", qcelpcfg.min_bit_rate);
		printf("max_bit_rate = 0x%8x\n", qcelpcfg.max_bit_rate);
		qcelpcfg.cdma_rate = max_rate;
		qcelpcfg.min_bit_rate = min_rate;
		qcelpcfg.max_bit_rate = max_rate;
		if (ioctl(afd, AUDIO_SET_QCELP_ENC_CONFIG, &qcelpcfg)) {
			printf("Error: AUDIO_SET_QCELP_ENC_CONFIG failed\n");
			goto fail;
		}
		printf("cdma rate = 0x%8x\n", qcelpcfg.cdma_rate);
		printf("min_bit_rate = 0x%8x\n", qcelpcfg.min_bit_rate);
		printf("max_bit_rate = 0x%8x\n", qcelpcfg.max_bit_rate);
	} else if(rec_type == 2) {

		if (ioctl(afd, AUDIO_GET_EVRC_ENC_CONFIG, &evrccfg)) {
			printf("Error: AUDIO_GET_EVRC_ENC_CONFIG failed\n");
			goto fail;
		}
		printf("cdma rate = 0x%8x\n", evrccfg.cdma_rate);
		printf("min_bit_rate = 0x%8x\n", evrccfg.min_bit_rate);
		printf("max_bit_rate = 0x%8x\n", evrccfg.max_bit_rate);
		evrccfg.cdma_rate = max_rate;
		evrccfg.min_bit_rate = min_rate;
		evrccfg.max_bit_rate = max_rate;
		if (ioctl(afd, AUDIO_SET_EVRC_ENC_CONFIG, &evrccfg)) {
			printf("Error: AUDIO_GET_EVRC_ENC_CONFIG failed\n");
			goto fail;
		}
		printf("cdma rate = 0x%8x\n", evrccfg.cdma_rate);
		printf("min_bit_rate = 0x%8x\n", evrccfg.min_bit_rate);
		printf("max_bit_rate = 0x%8x\n", evrccfg.max_bit_rate);
	} else if (rec_type == 3) {

		/* AMRNB specific settings */
		if (ioctl(afd, AUDIO_GET_AMRNB_ENC_CONFIG_V2, &amrnbcfg_v2)) {
			perror("Error: AUDIO_GET_AMRNB_ENC_CONFIG_V2 failed");
			goto fail;
		}
		printf("dtx mode = 0x%8x\n", amrnbcfg_v2.dtx_enable);
		printf("rate = 0x%8x\n", amrnbcfg_v2.band_mode);
		amrnbcfg_v2.dtx_enable = dtx_mode; /* 0 - DTX off, 1 - DTX on */
		amrnbcfg_v2.band_mode = max_rate;
		if (ioctl(afd, AUDIO_SET_AMRNB_ENC_CONFIG_V2, &amrnbcfg_v2)) {
			perror("Error: AUDIO_GET_AMRNB_ENC_CONFIG_V2 failed");
			goto fail;
		}
		printf("dtx mode = 0x%8x\n", amrnbcfg_v2.dtx_enable);
		printf("rate = 0x%8x\n", amrnbcfg_v2.band_mode);
	} else if (rec_type == 4) {

		/* AMRWB specific settings */
		if (ioctl(afd, AUDIO_GET_AMRWB_ENC_CONFIG, &amrwbcfg)) {
			perror("Error: AUDIO_GET_AMRWB_ENC_CONFIG failed");
			goto fail;
		}
		printf("dtx mode = 0x%8x\n", amrwbcfg.dtx_enable);
		printf("rate = 0x%8x\n", amrwbcfg.band_mode);
		amrwbcfg.dtx_enable = dtx_mode; /* 0 - DTX off, 1 - DTX on */
		amrwbcfg.band_mode = max_rate;
		if (ioctl(afd, AUDIO_SET_AMRWB_ENC_CONFIG, &amrwbcfg)) {
			perror("Error: AUDIO_GET_AMRWB_ENC_CONFIG failed");
			goto fail;
		}
		printf("dtx mode = 0x%8x\n", amrwbcfg.dtx_enable);
		printf("rate = 0x%8x\n", amrwbcfg.band_mode);
	}

	/* Record form voice link */
	if (rec_source <= VOC_REC_BOTH ) {

		if (ioctl(afd, AUDIO_SET_INCALL, &rec_source)) {
			perror("Error: AUDIO_SET_INCALL failed");
			goto fail;
		}
		printf("rec source = 0x%8x\n", rec_source);
	}
	/* Store handle for commands pass*/
	audio_data->afd = afd;
	sz = cfg.buffer_size;
        if (clnt_config->mode) {
               	/* non - tunnel portion for 8660 */
               	pthread_create(&thread, NULL, voiceenc_nt, (void *)clnt_config);
		/* Sleep to ensure audio start been called, before
		 * Driver read done */
		sleep(1);
	} else {
		ioctl(afd, AUDIO_START, 0);
	}

	printf("Voice encoder started 8660\n");

	if((frame_format == 1) || ((frame_format == 2) && (rec_type < 3))) { /* QCP file */
	        lseek(fd, QCP_HEADER_SIZE, SEEK_SET);
		printf("qcp_headsize %d\n",QCP_HEADER_SIZE);
		printf("QCP format\n");
	} else if ((frame_format == 2) && (rec_type == 3)) /*AMR file*/ {
	        lseek(fd, 0, SEEK_SET);
        	write(fd, (char *)&amr_header, AMR_HEADER_SIZE);
		printf("AMR format\n");
	} else
		printf("DSP format\n");

	rec_stop = 0;
	while(!rec_stop) {
		memset(buf,0,sz);
		readcnt = read(afd, buf, sz);
                if (readcnt <= 0) {
                        printf("cannot read buffer error code =0x%8x", readcnt);
			goto fail;
                }
		else
		{
			/* Multiframing Supported */
			unsigned char *memptr = buf;
			struct meta_out_enc *meta_enc;
			struct meta_out *meta;
			unsigned char nr_of_frames;
			meta_enc = (struct meta_out_enc *)memptr;
			nr_of_frames = meta_enc->num_of_frames;
			meta = (struct meta_out *) (memptr + sizeof(meta_enc->num_of_frames));
			printf("meta = 0x%p meta_enc = 0x%p\n", meta, meta_enc);
			printf("Read cnt = %d\n", readcnt);
			printf("number of frames = 0x%2x\n", nr_of_frames);
			while(nr_of_frames > 0) {
				printf(" offset_to_frame = %d frame_size = %d\n", meta->offset_to_frame, meta->frame_size);
				printf(" encoded_pcm_samples = %d\n", meta->encoded_pcm_samples);
				printf(" mswts = %d lswts = %d\n", meta->msw_ts, meta->lsw_ts);
				printf(" mswts = %d lswts = %d\n", meta->msw_ts, meta->lsw_ts);
				printf(" nflags = 0x%8x\n", meta->nflags);
				if (meta->nflags & 0x01) {
					printf("EOS reached on input as well \n");
					goto done;
				}
				memptr = buf + sizeof(meta_enc->num_of_frames) + meta->offset_to_frame;
				readcnt = meta->frame_size;
				writecnt = write(fd, memptr, readcnt);
				if (writecnt <= 0) {
					printf("cannot write buffer error code =0x%8x", writecnt);
					goto fail;
				}
				framecnt++;
				datasize += writecnt;
				meta++;
				printf("meta = 0x%p\n", meta);
				printf(" frame cnt = %d\n", framecnt);
				nr_of_frames --;
                	}
        	}
	}
done:
	ioctl(afd, AUDIO_GET_STATS, &stats);
	printf("\n read_bytes = %d, read_frame_counts = %d\n",datasize, framecnt);
	ioctl(afd, AUDIO_STOP, 0);
	if((frame_format == 1) || ((frame_format == 2) && (rec_type < 3))) { /* QCP file */
		create_qcp_header(datasize, framecnt);
	        lseek(fd, 0, SEEK_SET);
		write(fd, (char *)&append_header, QCP_HEADER_SIZE);
	}

	printf("Secondary encoder stopped \n");
	if(!audio_data->recbuf)
	        free(audio_data->recbuf);
	close(afd);
#ifndef AUDIO7X27A
	if(!clnt_config->mode)
		if (devmgr_unregister_session(enc_id, DIR_TX) < 0) {
			perror("\ncould not unregister recording session\n");
		}
#endif
	return 0;
fail:
	if(!audio_data->recbuf)
	        free(audio_data->recbuf);
	close(afd);

#ifndef AUDIO7X27A
	if(!clnt_config->mode)
		if (devmgr_unregister_session(enc_id, DIR_TX) < 0) {
			perror("\ncould not unregister recording session\n");
		}
#endif
device_err:
	close(fd);
	unlink(clnt_config->file_name);
file_err:
	return -1;
}

void *voiceenc_thread(void *arg)
{
	struct audiotest_thread_context *context =
	    (struct audiotest_thread_context *)arg;
	int ret_val = 0;

	if(context->config.tgt == 0x07)
		ret_val = voiceenc_start(&context->config);
	else if(context->config.tgt == 0x08)
		ret_val = voiceenc_start_8660(&context->config);

	free_context(context);
	pthread_exit((void *)ret_val);
	return NULL;
}

int voiceenc_read_params(void)
{
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
				context->config.file_name = "/data/sample.qcp";
				context->config.in_file_name = "/data/pcm.wav";
			#else
				context->config.file_name = "/tmp/sample.qcp";
				context->config.in_file_name = "/tmp/pcm.wav";
			#endif
			context->type = AUDIOTEST_TEST_MOD_VOICE_ENC;
			token = strtok(NULL, " ");
			rec_type = 1; /* qcelp */
			frame_format = 0;
			dtx_mode = 0;
			min_rate = 4;
			max_rate = 4;
			rec_source = 0;
			context->config.mode = 0;
			context->config.tgt = 0x07;
			context->config.frames_per_buf= 1;
			audio_data->mode = 0;
			while (token != NULL) {
				if (!memcmp(token, "-id=", (sizeof("-id=") - 1))) {
					context->cxt_id =
					    atoi(&token[sizeof("-id=") - 1]);
				}else if (!memcmp(token, "-type=", (sizeof("-type=") - 1))) {
					rec_type =
					    atoi(&token[sizeof("-type=") - 1]);
				}else if (!memcmp(token, "-fmt=", (sizeof("-fmt=") - 1))) {
					frame_format =
					    atoi(&token[sizeof("-fmt=") - 1]);
				}else if (!memcmp(token, "-dtx=", (sizeof("-dtx=") - 1))) {
					dtx_mode =
					    atoi(&token[sizeof("-dtx=") - 1]);
				}else if (!memcmp(token, "min=", (sizeof("-min=") - 1))) {
					min_rate =
					    atoi(&token[sizeof("-min=") - 1]);
				}else if (!memcmp(token, "-max=", (sizeof("-max=") - 1))) {
					max_rate =
					    atoi(&token[sizeof("-max=") - 1]);
				}else if (!memcmp(token, "-src=", (sizeof("-src=") - 1))) {
					rec_source =
					    atoi(&token[sizeof("-src=") - 1]);
				}else if (!memcmp(token,"-mode=", (sizeof("-mode=" - 1)))) {
						context->config.mode = atoi(&token[sizeof("-mode=") - 1]);
						audio_data->mode = context->config.mode;
				}else if (!memcmp(token, "-out=",
                	                        (sizeof("-out=") - 1))) {
                        	        context->config.file_name = token + (sizeof("-out=")-1);
				}else if (!memcmp(token, "-tgt=", (sizeof("-tgt=") - 1))) {
						context->config.tgt = atoi(&token[sizeof("-tgt=") - 1]);
				}else if (!memcmp(token, "-infile=", (sizeof("-infile=") - 1))) {
						token = &token[sizeof("-infile=") - 1];
						printf("infile = %s\n", token);
						context->config.in_file_name = token;
				}else if (!memcmp(token, "-frames=", (sizeof("-frames=") - 1))) {
					context->config.frames_per_buf= atoi(&token[sizeof("-frames=") - 1]);
					printf("Num of frames per buf=%d\n",context->config.frames_per_buf);
				}
				token = strtok(NULL, " ");
			}
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create(&context->thread, NULL,
				       voiceenc_thread, (void *)context);
		}
	}

	return ret_val;

}

int voiceenc_control_handler(void *private_data)
{
	int drvfd , ret_val = 0;
	char *token;

	token = strtok(NULL, " ");
	if ((private_data != NULL) && (token != NULL)) {
		drvfd = (int) private_data;
		if (!memcmp(token, "-cmd=", (sizeof("-cmd=") - 1))) {
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

const char *voiceenc_help_txt =
	"Voice encoder \n \
echo \"voiceenc -id=xxx -infile=path_of_inputfile -out=path_of_outputfile -type=yy -fmt=zz -dtx=yy -min=zz -max=yy -src=zz -frames=ww -mode=zz -tgt=zz\" > %s \n\
type: 1 - qcelp, 2 - evrc, 3 - amrnb, 4 - amrwb\n \
fmt: 0 - dsp 1 - qcp[dsp transcode] 2 - qcp [no dsp transcode] \n \
src: 0 - Uplink 1 - Downlink, 2 - UL/DL, 3 - Mic \n \
dtx: 0 - disable 1 - enable \n \
tgt: 08 - for 8660 target, default 7k target \n \
min:max: rate qcelp 1 to 4, rate evrc 1 to 4[exclude 2], rate amr 1 to 7, rate amrwb 0 to 8\n \
frames: number of frames per buffer(default 1)\n \
mode: 0 - Tunnel 1 - NonTunnel\n \
examples: \n\
echo \"voiceenc -id=123 -out=path_of_file -type=3 -fmt=1 -dtx=0 -min=7 -max=7 -src=3\" > %s \n\
Supported control command: stop \n ";

void voiceenc_help_menu(void) {
	printf(voiceenc_help_txt, *cmdfile, *cmdfile);
}

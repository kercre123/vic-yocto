/* qcptest.c - native QCP test application
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
#include <unistd.h>
#include <string.h>
#include <linux/ioctl.h>
#include <linux/msm_audio_qcp.h>
#include "audiotest_def.h"

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

#define QCP_HEADER_SIZE sizeof(struct qcp_header)

static int rec_stop;

static void create_qcp_header(int Datasize, int Frames, int Format)
{
	append_header.s_riff = Datasize + QCP_HEADER_SIZE - 8;

	if(!Format) {

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
		append_header.s_data = Datasize;
	}
	else {

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
	}

	append_header.s_data = Datasize;
	return;
}

int qcp_rec(struct audtest_config *config)
{

	struct qcp_header hdr;
	unsigned char buf[1024];
	struct msm_audio_evrc_enc_config evrc_cfg;
	struct msm_audio_qcelp_enc_config qcelp_cfg;
	struct msm_audio_stream_config str_cfg;
	struct msm_voicerec_mode voicerec_mode;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *) config->private_data;
	unsigned sz;
	int fd, afd;
	int total = 0;
	int read_size = 0;


	//voicerec_mode.rec_mode = config->rec_mode;
	fd = open(config->file_name, O_CREAT | O_RDWR, 0666);
	if (fd < 0) {
		perror("cannot open output file");
		return -1;
	}
	if(!audio_data->format) {
		afd = open("/dev/msm_evrc_in", O_RDWR);
		if (afd < 0) {
			perror("cannot open msm_evrc_in");
			close(fd);
			return -1;
		}
		if (ioctl(afd, AUDIO_GET_STREAM_CONFIG, &str_cfg)) {
			perror("cannot read audio stream config");
			goto fail;
		}

		sz = 23;
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


		if (ioctl(afd, AUDIO_GET_EVRC_ENC_CONFIG, &evrc_cfg)) {
			perror("cannot read audio config");
			goto fail;
		}

		evrc_cfg.cdma_rate = CDMA_RATE_FULL;
		evrc_cfg.min_bit_rate = 4;
		evrc_cfg.max_bit_rate = 4;

		if (ioctl(afd, AUDIO_SET_EVRC_ENC_CONFIG, &evrc_cfg)) {
			perror("cannot write audio config");
			goto fail;
		}
	}
	else {
		afd = open("/dev/msm_qcelp_in", O_RDWR);
		if (afd < 0) {
			perror("cannot open msm_qcp_in");
			close(fd);
			return -1;
		}
		if (ioctl(afd, AUDIO_GET_STREAM_CONFIG, &str_cfg)) {
			perror("cannot read audio stream config");
			goto fail;
		}

		sz = 35;
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

		if (ioctl(afd, AUDIO_GET_QCELP_ENC_CONFIG, &qcelp_cfg)) {
			perror("cannot read audio config");
			goto fail;
		}

		qcelp_cfg.cdma_rate = CDMA_RATE_FULL;
		qcelp_cfg.min_bit_rate = 4;
		qcelp_cfg.max_bit_rate = 4;

		if (ioctl(afd, AUDIO_SET_QCELP_ENC_CONFIG, &qcelp_cfg)) {
			perror("cannot write audio config");
			goto fail;
		}

	}


	voicerec_mode.rec_mode=2;
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
	lseek(fd, QCP_HEADER_SIZE, SEEK_SET);
	rec_stop = 0;
	while (rec_stop != 1) {
		read_size = read(afd, buf, sz);
		if (read_size <= 0) {
                        printf("cannot read buffer error code =0x%8x", read_size);
                        goto fail;
                }
		if (write(fd, buf, read_size) != read_size) {
			perror("cannot write buffer");
			goto fail;
		}
		total += read_size;
	}
	close(afd);
	create_qcp_header(total, 1, audio_data->format);
	lseek(fd, 0, SEEK_SET);
	write(fd, (char *)&append_header, QCP_HEADER_SIZE);
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

void* recqcp_thread(void* arg) {
	struct audiotest_thread_context *context =
		(struct audiotest_thread_context*) arg;
	int ret_val;

	ret_val = qcp_rec(&context->config);
	free_context(context);
	pthread_exit((void*) ret_val);

    return NULL;
}

int qcprec_read_params(void) {
	struct audiotest_thread_context *context;
	struct itimerspec ts;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.file_name = "/data/rec.qcp";
		context->config.rec_mode = 2;
		context->type = AUDIOTEST_TEST_MOD_QCP_ENC;
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
				if (!memcmp(token,"-rec_mode=", (sizeof("-rec_mode=") - 1))) {
					context->config.rec_mode =
					       atoi(&token[sizeof("-rec_mode=") - 1]);
				} else if (!memcmp(token,"-id=", (sizeof("-id=") - 1))) {
					context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
				} else if (!memcmp(token,"-format=", (sizeof("-format=") - 1))) {
					audio_data->format = atoi(&token[sizeof("-format=") - 1]);
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
			printf("%s : rec_mode=%d\n", __FUNCTION__, context->config.rec_mode);
			context->config.private_data = (struct audio_pvt_data *) audio_data;
			pthread_create(&context->thread, NULL, recqcp_thread, (void *) context);
		}
	}
	return ret_val;
}

int qcp_rec_control_handler(void *private_data)
{
	int /* drvfd ,*/ ret_val = 0;
	char *token;

	token = strtok(NULL, " ");
	if ((private_data != NULL) &&
		(token != NULL)) {
		/* drvfd = (int) private_data */
		if (!memcmp(token, "-cmd=", (sizeof("-cmd=")-1))) {
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

const char *qcprec_help_txt =
"Record qcp file: type \n\
echo \"recqcp path_of_file -format=x -rec_mode=x -id=xxx -timeout=x\" > /data/audio_test \n\
timeout = x (value in seconds) \n\
format = 0 (evrc) or 1 (qcelp) \n\
record mode: 0=txonly 1=rxonly 2=mixed)\n ";

void qcprec_help_menu(void)
{
	printf("%s\n", qcprec_help_txt);
}

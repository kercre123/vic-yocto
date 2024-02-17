/* voicememotest.c - native voicememo test application
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2009, The Linux Foundation. All rights reserved.
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
#include <linux/msm_audio_voicememo.h>
#include <pthread.h>
#include "audiotest_def.h"

#define VOICEMEMO_DEVICE_NODE "/dev/msm_voicememo"

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

static int start;
static struct msm_audio_voicememo_config scfg;

static void create_qcp_header(int Datasize, int Frames)
{
	append_header.s_riff = Datasize + QCP_HEADER_SIZE - 8; /* exclude riff id and size field */
	if( scfg.capability  == 4 ) { /* QCELP 13K */
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
	} else if ( scfg.capability  == 8) { /* EVRC */
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
	} else if( scfg.capability  == 64 ) { /* AMRNB */
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

static int voicememo_start(struct audtest_config *clnt_config)
{
	int afd, fd;
	unsigned char tmp;
        unsigned char buf[1024];
        unsigned sz,total; 
	int readcnt,writecnt;
	struct msm_audio_config cfg;
	struct msm_audio_voicememo_config gcfg;
	struct msm_audio_stats stats;
	memset(&stats,0,sizeof(stats));
	memset(&cfg,0,sizeof(cfg));
	memset(&scfg,0,sizeof(scfg));

	fd = open(clnt_config->file_name, O_CREAT | O_RDWR, 0666);
	
	if (fd < 0) {
		printf("Unable to create output file = %s\n", 
			clnt_config->file_name);
		goto file_err;
	}
	else
		printf("file created =%s\n",clnt_config->file_name);

	/* Open Device 	Node */
	afd = open(VOICEMEMO_DEVICE_NODE, O_RDWR);
	if (afd < 0) {
		printf("Unable to open audio device = %s\n", 
			VOICEMEMO_DEVICE_NODE);
		goto device_err;
	}

	/* Config param */
	if(ioctl(afd, AUDIO_GET_CONFIG, &cfg)) {
		printf(" Error getting buf config param AUDIO_GET_CONFIG \n");
		goto fail; 
	}

	if (ioctl(afd, AUDIO_GET_VOICEMEMO_CONFIG, &gcfg)) {
		printf("Error: AUDIO_GET_VOICEMEMO_CONFIG failed\n");
		goto fail; 
	}

#ifdef DEBUG_LOCAL
	printf("Default rec_type = 0x%8x\n",gcfg.rec_type);
	printf("Default rec_interval_ms = 0x%8x\n",gcfg.rec_interval_ms);
	printf("Default auto_stop_ms = 0x%8x\n",gcfg.auto_stop_ms);
	printf("Default capability = 0x%8x\n",gcfg.capability);
	printf("Default max_rate = 0x%8x\n",gcfg.max_rate);
	printf("Default min_rate = 0x%8x\n",gcfg.min_rate);
	printf("Default frame_format = 0x%8x\n",gcfg.frame_format);
	printf("Default dtx_enable = 0x%8x\n",gcfg.dtx_enable);
	printf("Default data_req_ms = 0x%8x\n",gcfg.data_req_ms);
#endif
	/* Store handle for commands pass*/
	clnt_config->private_data = (void *) afd;
	
	start = 0;
	do {
		usleep(1000000);
	} while(!start);

	sz = cfg.buffer_size; 
	total = 0;
        
	/* Set Via  config param */
	if (ioctl(afd, AUDIO_SET_VOICEMEMO_CONFIG, &scfg)) {
		printf("Error: AUDIO_SET_VOICEMEMO_CONFIG failed\n");
		goto fail; 
	}

	if (ioctl(afd, AUDIO_GET_VOICEMEMO_CONFIG, &gcfg)) {
		printf("Error: AUDIO_GET_VOICEMEMO_CONFIG failed\n");
		goto fail; 
	}

#ifdef DEBUG_LOCAL
	printf("After set rec_type = 0x%8x\n",gcfg.rec_type);
	printf("After set rec_interval_ms = 0x%8x\n",gcfg.rec_interval_ms);
	printf("After set auto_stop_ms = 0x%8x\n",gcfg.auto_stop_ms);
	printf("After set capability = 0x%8x\n",gcfg.capability);
	printf("After set max_rate = 0x%8x\n",gcfg.max_rate);
	printf("After set min_rate = 0x%8x\n",gcfg.min_rate);
	printf("After set frame_format = 0x%8x\n",gcfg.frame_format);
	printf("After set dtx_enable = 0x%8x\n",gcfg.dtx_enable);
	printf("After set data_req_ms = 0x%8x\n",gcfg.data_req_ms);
#endif
	fcntl(0, F_SETFL, O_NONBLOCK);
	ioctl(afd, AUDIO_START, 0);

	printf("Voice Memo started \n");

	ioctl(afd, AUDIO_GET_STATS, &stats);
#ifdef DEBUG_LOCAL
	printf("\n read_bytes = %d, read_frame_counts = %d\n",stats.byte_count, stats.sample_count);
#endif
	printf("\n*** RECORDING -  HIT ENTER TO STOP ***\n");
	if( scfg.frame_format == 3) /* QCP file */
	        lseek(fd, QCP_HEADER_SIZE, SEEK_SET);
	else if ( scfg.frame_format == 4) /* AMR file */
	        lseek(fd, AMR_HEADER_SIZE, SEEK_SET);

	for (;;) {
		/* Look for Enter key to terminate */
                while (read(0, &tmp, 1) == 1) {
                        if ((tmp == 13) || (tmp == 10)) goto done;
                }
		
		readcnt = read(afd, buf, sz);
#ifdef DEBUG_LOCAL
		printf(" Read bytes = %d \n", readcnt); 
#endif
                if (readcnt <= 0) {
                        printf("cannot read buffer error code =0x%8x", readcnt);
			goto fail;
                }
		else
		{
			writecnt = write(fd, buf, readcnt);
                	if (writecnt <= 0) {
                        	printf("cannot write buffer error code =0x%8x", writecnt);
				goto fail;
			}	
#ifdef DEBUG_LOCAL
			printf(" Written bytes = %d \n", writecnt); 
#endif
                }
		total+=writecnt;
        }
done:
	ioctl(afd, AUDIO_GET_STATS, &stats);
	printf("\n read_bytes = %d, read_frame_counts = %d\n",stats.byte_count, stats.sample_count);
	ioctl(afd, AUDIO_STOP, 0);
	if( scfg.frame_format == 3) { /* QCP file */
        	create_qcp_header(stats.byte_count, stats.sample_count);
	        lseek(fd, 0, SEEK_SET);
        	write(fd, (char *)&append_header, QCP_HEADER_SIZE);
	} else if ( scfg.frame_format == 4) { /* AMR file */
	        lseek(fd, 0, SEEK_SET);
        	write(fd, (char *)&amr_header, AMR_HEADER_SIZE);
	}
#ifdef DEBUG_LOCAL
	printf("Bytes recorded %d\n", total);
#endif
	printf("Voice Memo stopped \n");
	close(afd);
	close(fd);
	return 0;
fail:
	close(afd);
device_err:
	close(fd);
	unlink(clnt_config->file_name);
file_err:
	return -1;
}

void *voicememo_thread(void *arg)
{
	struct audiotest_thread_context *context =
	    (struct audiotest_thread_context *)arg;
	int ret_val;

	ret_val = voicememo_start(&context->config);
	free_context(context);
	pthread_exit((void *)ret_val);
	return NULL;
}

int voicememo_read_params(void)
{
	struct audiotest_thread_context *context;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		#ifdef _ANDROID_
			context->config.file_name = "/data/sample.raw";
		#else
			context->config.file_name = "/tmp/sample.raw";
		#endif
		context->type = AUDIOTEST_TEST_MOD_VOICEMEMO;
		token = strtok(NULL, " ");
		while (token != NULL) {
			if (!memcmp(token, "-id=", (sizeof("-id=") - 1))) {
				context->cxt_id =
				    atoi(&token[sizeof("-id=") - 1]);
			} else if (!memcmp(token, "-out=",
                                        (sizeof("-out=") - 1))) {
                                context->config.file_name = token + (sizeof("-out=")-1);
			}	
			token = strtok(NULL, " ");
		}
		pthread_create(&context->thread, NULL,
			       voicememo_thread, (void *)context);
	}

	return ret_val;

}

int voicememo_control_handler(void *private_data)
{
	int drvfd , ret_val = 0;
	char *token;

	token = strtok(NULL, " ");
	if ((private_data != NULL) && (token != NULL)) {
		drvfd = (int) private_data;
		if (!memcmp(token, "-cmd=", (sizeof("-cmd=") - 1))) {
			token = &token[sizeof("-cmd=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
			if (!strcmp(token, "pause")) {
				ioctl(drvfd, AUDIO_PAUSE, 1);
			} else if (!strcmp(token, "resume")) {
				ioctl(drvfd, AUDIO_PAUSE, 0);
			} else if (!strcmp(token, "config")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-rec=",
					(sizeof("-rec=") - 1)))
				scfg.rec_type =
				atoi(&token[sizeof("-rec=") - 1]);
				token = strtok(NULL, " ");
				if (!memcmp(token, "-cap=",
					(sizeof("-cap=") - 1)))
				scfg.capability =
				atoi(&token[sizeof("-cap=") - 1]);
				token = strtok(NULL, " ");
				if (!memcmp(token, "-maxr=",
					(sizeof("-maxr=") - 1)))
				scfg.max_rate =
				atoi(&token[sizeof("-maxr=") - 1]);
				token = strtok(NULL, " ");
				if (!memcmp(token, "-minr=",
					(sizeof("-minr=") - 1)))
				scfg.min_rate =
				atoi(&token[sizeof("-minr=") - 1]);
				token = strtok(NULL, " ");
				if (!memcmp(token, "-frame=",
					(sizeof("-frame=") - 1)))
				scfg.frame_format =
				atoi(&token[sizeof("-frame=") - 1]);
				token = strtok(NULL, " ");
				if (!memcmp(token, "-datams=",
					(sizeof("-datams=") - 1)))
				scfg.data_req_ms =
				atoi(&token[sizeof("-datams=") - 1]);
			        scfg.rec_interval_ms = 0;
				scfg.auto_stop_ms = 0;
			        scfg.dtx_enable = 0;
				start = 1;
			} 
		}
	} else {
		ret_val = -1;
	}
	return ret_val;
}

const char *voicememo_help_txt = "Record voice in QCP/AMR file format: type \n\
echo \"voicememo -id=xxx -out=path_of_file \" > /tmp/audio_test \n\
Supported control command: pause, resume, config \n\
examples: \n\
echo \"voicememo -id=xxx -out=path_of_file \" > /tmp/audio_test \n\
echo \"control_cmd -id=xxx -cmd=pause\" > /tmp/audio_test \n\
echo \"control_cmd -id=xxx -cmd=resume\" > /tmp/audio_test \n\
echo \"control_cmd -id=xxx cmd=config -rec=aaa -cap=bbb -maxr=ccc -minr=ddd -frame=eee -datams=fff \" > /tmp/audio_test \n";

void voicememo_help_menu(void)
{
	printf("%s\n", voicememo_help_txt);
}

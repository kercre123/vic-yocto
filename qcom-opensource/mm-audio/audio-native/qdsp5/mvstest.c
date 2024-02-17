/* mvstest.c - native mvs test application
 *
 * Copyright (c) 2011-2012 The Linux Foundation. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/msm_audio_mvs.h>
#include "audiotest_def.h"

#if defined(TARGET_USES_QCOM_MM_AUDIO)
#include "control.h"
#include "acdb-loader.h"
#include "acdb-id-mapper.h"
#endif

#ifdef QDSP6V2
struct test_gsm_header {
 unsigned char bfi;
 unsigned char sid;
 unsigned char taf;
 unsigned char ufi;
};

struct test_frame_type {
	union {
	unsigned int frame_type;
	unsigned int packet_rate;
	struct test_gsm_header test_gsm_frame_type;
	}test_header;
	unsigned int len;
	unsigned char voc_pkts[640];
};
#else
struct test_frame_type {
	unsigned int frame_type;
	unsigned int len;
	unsigned char voc_pkts[320];
};
#endif

static int lp_stop = 0;

static int mvs_lp_test(struct audtest_config *config) {
	int fd = NULL;
	int ret = -1;
	int device_id_rx, device_id_tx;
	struct mvs_pvt_data *mvs_data = (struct mvs_pvt_data *) config->private_data;


	/* Open MVS driver. */
	fd = open("/dev/msm_mvs", O_RDWR);
	if (fd < 0)
		printf("MVS open returned %d \n", fd);

	struct msm_audio_mvs_config mvs_config;
	mvs_config.mvs_mode = mvs_data->g_mvs_mode;
	mvs_config.rate_type = mvs_data->g_rate_type;
	mvs_config.dtx_mode = mvs_data->g_dtx_mode;
	mvs_config.min_max_rate.min_rate = mvs_data->g_min_max_rate.min_rate;
	mvs_config.min_max_rate.max_rate = mvs_data->g_min_max_rate.max_rate;

	ret = ioctl(fd, AUDIO_SET_MVS_CONFIG, &mvs_config);
	printf("MVS ioctl returned %d \n", ret);

	ret = ioctl(fd, AUDIO_START, NULL);
	printf("MVS start returned %d \n", ret);

	/* Setup device and route voice to device. */
	ret = msm_mixer_open("/dev/snd/controlC0", 0);
	if (ret < 0)
		printf("Error %d opening mixer \n", ret);

	device_id_rx = mvs_data->g_rx_devid;
	device_id_tx = mvs_data->g_tx_devid;

	ret = msm_route_voice(device_id_rx, device_id_tx, 1);
        if (ret < 0)
                printf("Error %d routing voice to handset \n", ret);

	devmgr_enable_voice_device(device_id_rx, device_id_tx);

	/* Send voice calibration */
	printf("Setting up voice calibration \n");

	int acdb_id_tx = 0;
	int acdb_id_rx = 0;
	acdb_mapper_get_acdb_id_from_dev_id(device_id_tx, &acdb_id_tx);
	acdb_mapper_get_acdb_id_from_dev_id(device_id_rx, &acdb_id_rx);

	acdb_loader_send_voice_cal(acdb_id_rx, acdb_id_tx);

	ret = msm_start_voice();
	if (ret < 0)
		printf("Error %d starting voice \n", ret);

	ret = msm_set_voice_rx_vol(4);
	if (ret < 0)
		printf("Error %d setting volume \n", ret);

	ret = msm_set_voice_tx_mute(0);
	if (ret < 0)
		printf("Error %d un-muting \n", ret);

	struct test_frame_type test_frame;

	int i = 0;
	lp_stop = 0;
	printf(" start to read and write data \n");
	while (!lp_stop) {
		ret = read(fd, &test_frame, sizeof(test_frame));
		if (ret > 0) {
			ret = write(fd, &test_frame, sizeof(test_frame));
			if (ret < 0)
				printf("MVS write returned %d \n", ret);
		} else {
			printf("MVS read returned %d \n", ret);
		}

	}
	printf(" end to read and write data \n");
	/* Mute and disable the device. */
	ret = msm_set_voice_rx_vol(0);
	if (ret <0)
		printf("Error %d setting volume\n", ret);

	ret = msm_set_voice_tx_mute(1);
	if (ret < 0)
		printf("Error %d setting mute\n", ret);

	ret = msm_end_voice();
	if (ret < 0)
		printf("Error %d ending voice\n", ret);

	devmgr_disable_voice_device();

	ret = msm_mixer_close();
	if (ret < 0)
		printf("Error %d closing mixer\n", ret);

	ret = ioctl(fd, AUDIO_STOP, NULL);
	printf("MVS stop returned %d \n", ret);

	ret = close(fd);
	printf("MVS close returned %d \n", ret);

	return 0;
}

void* mvstest_thread(void* arg) {
        struct audiotest_thread_context *context =
                (struct audiotest_thread_context*) arg;
        int ret_val;

        ret_val = mvs_lp_test(&context->config);
        free_context(context);
        pthread_exit((void*) ret_val);

    return NULL;
}


int mvstest_read_params(void) {
        struct audiotest_thread_context *context;
        char *token;
        int ret_val = 0;

        if ((context = get_free_context()) == NULL) {
                ret_val = -1;
        } else {
		struct mvs_pvt_data *mvs_data;
                mvs_data = (struct mvs_pvt_data *) malloc(sizeof(struct mvs_pvt_data));
                if(!mvs_data) {
                        printf("error allocating mvs instance structure \n");
                        free_context(context);
                        ret_val = -1;
                } else {
			token = strtok(NULL, " ");
			while (token != NULL) {
				if (!memcmp(token,"-id=", (sizeof("-id=")-1))) {
					context->cxt_id = atoi(&token[sizeof("-id=") - 1]);
				} else if (!memcmp(token,"-mvs_mode=", (sizeof("-mvs_mode=") - 1))) {
					mvs_data->g_mvs_mode = atoi(&token[sizeof("-mvs_mode=") - 1]);
				} else if (!memcmp(token,"-rate_type=", (sizeof("-rate_type=") - 1))) {
					mvs_data->g_rate_type = atoi(&token[sizeof("-rate_type=") - 1]);
				} else if (!memcmp(token,"-min_rate=",(sizeof("-min_rate=")-1))) {
					mvs_data->g_min_max_rate.min_rate = atoi(&token[sizeof("-min_rate=") - 1]);
				} else if (!memcmp(token,"-max_rate=",(sizeof("-max_rate=")-1))) {
					mvs_data->g_min_max_rate.max_rate = atoi(&token[sizeof("-max_rate=") - 1]);
				} else if (!memcmp(token,"-dtx_mode=", (sizeof("-dtx_mode=") - 1))) {
					mvs_data->g_dtx_mode = atoi(&token[sizeof("-dtx_mode=") - 1]);
				} else if (!memcmp(token,"-rx_devid=", (sizeof("-rx_devid=") - 1))) {
					mvs_data->g_rx_devid = atoi(&token[sizeof("-rx_devid=") - 1]);
				} else if (!memcmp(token,"-tx_devid=", (sizeof("-tx_devid=") - 1))) {
					mvs_data->g_tx_devid = atoi(&token[sizeof("-tx-devid=") - 1]);
				} else {
					context->config.file_name = token;
				}
					token = strtok(NULL, " ");
                        }
                        context->type = AUDIOTEST_TEST_MODE_MVS_LP;
			context->config.private_data = (struct mvs_pvt_data *) mvs_data;
                        pthread_create( &context->thread, NULL,
                                        mvstest_thread, (void*) context);
		}
       }

        return ret_val;
}

int mvs_lp_test_control_handler(void* private_data) {
        int /* drvfd ,*/ ret_val = 0;
        char *token;

        token = strtok(NULL, " ");
        if ((private_data != NULL) &&
                (token != NULL)) {
                /* drvfd = (int) private_data */
                if(!memcmp(token,"-cmd=", (sizeof("-cmd=") -1))) {
                        token = &token[sizeof("-cmd=") - 1];
                        printf("%s: cmd %s\n", __FUNCTION__, token);
                        if (!strcmp(token, "stop")) {
                                lp_stop = 1;
                        }
                }
        } else {
                ret_val = -1;
        }

        return ret_val;
}

const char *mvstest_help_txt =
        "MVS Loopback Test: \n\
push acdb file first:\n\
   adb shell \"mount -t vfat -o remount,rw /dev/block/mmcblk0p1 /system/\"\n\
   adb push audio_cal.acdb etc/   \n\
\n\
For EVRC,Qcelp 4gv use (dtx only for 4gv) \n\
echo \"mvstest -id=xxx -mvs_mode=x -min_rate=x -max_rate=x -dtx_mode=x -rx_devid=x -tx_devid=x\" > tmp/audio_test \n\
For other formats \n\
echo \"mvstest -id=xxx -mvs_mode=x -rate_type=x -dtx_mode=x -rx_devid=x -tx_devid=x\" > tmp/audio_test \n\
mvs mode: \n\
	1:     MVS_MODE_IS733 \n\
	2:     MVS_MODE_IS127 \n\
	3:     MVS_MODE_4GV_NB \n\
	4:     MVS_MODE_4GV_WB \n\
	5:     MVS_MODE_AMR \n\
	6:     MVS_MODE_EFR \n\
	7:     MVS_MODE_FR \n\
	8:     MVS_MODE_HR \n\
	9:     MVS_MODE_LINEAR_PCM \n\
	10:    MVS_MODE_G711 \n\
	12:    MVS_MODE_PCM \n\
	13:    MVS_MODE_AMR_WB \n\
	14:    MVS_MODE_G729A \n\
	15:    MVS_MODE_G711A \n\
rate type: \n\
	0:	MVS_AMR_MODE_0475 	\n\
	1:	MVS_AMR_MODE_0515	\n\
	2:	MVS_AMR_MODE_0590	\n\
	3:	MVS_AMR_MODE_0670	\n\
	4:	MVS_AMR_MODE_0740	\n\
	5:	MVS_AMR_MODE_0795	\n\
	6:	MVS_AMR_MODE_1020	\n\
	7:	MVS_AMR_MODE_1220	\n\
	8:	MVS_AMR_MODE_0660	\n\
	9:	MVS_AMR_MODE_0885	\n\
	10:	MVS_AMR_MODE_1265	\n\
	11:	MVS_AMR_MODE_1425	\n\
	12:	MVS_AMR_MODE_1585	\n\
	13:	MVS_AMR_MODE_1825	\n\
	14:	MVS_AMR_MODE_1985	\n\
	15:	MVS_AMR_MODE_2305	\n\
	16:	MVS_AMR_MODE_2385	\n\
					\n\
dtx mode:\n\
	enable - 1 \n\
	disable - 0 \n\
					\n\
Supported control command: stop		\n\
echo \"control_cmd -id=xxx -cmd=stop\" > /data/audio_test \n\
                                         \n";


void mvstest_help_menu(void) {
        printf("%s\n", mvstest_help_txt);
}

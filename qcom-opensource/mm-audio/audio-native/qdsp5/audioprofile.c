/* audioprofile.c - native audio profile test application
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2009-2010, The Linux Foundation. All rights reserved.
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
#include "audiotest_def.h"

#define NUMBER_DECODER_SUPPORTED 5
#define PROFILE_NODE "/sys/devices/platform/msm_adspdec/concurrency"
static const char *decoder_name[] = {
"/sys/devices/platform/msm_adspdec/decoder0",
"/sys/devices/platform/msm_adspdec/decoder1",
"/sys/devices/platform/msm_adspdec/decoder2",
"/sys/devices/platform/msm_adspdec/decoder3",
"/sys/devices/platform/msm_adspdec/decoder4",
};

#ifdef _ANDROID_
static const char *cmdfile = "/data/audio_test";
#else
static const char *cmdfile = "/tmp/audio_test";
#endif

int profile_read_params(void)
{
	char buf[100];
	int ret_val = 0, sz, op = 0,i;
	int afd, decafd[NUMBER_DECODER_SUPPORTED], decfdcnt;
	char *token;
	
	memset(buf, 0, sizeof buf);

	for(decfdcnt =0; decfdcnt < NUMBER_DECODER_SUPPORTED; decfdcnt++){
		decafd[decfdcnt] = open(decoder_name[decfdcnt], O_RDONLY);
		if (decafd[decfdcnt] < 0) {
			printf("error opening decoder node: %s\n", decoder_name[decfdcnt]);
			ret_val = -1;
			goto err;
		}
	}	
	afd = open(PROFILE_NODE, O_RDWR);
	if (afd < 0) {
		printf("error opening profile node: %s\n", PROFILE_NODE);
		ret_val = -1;
		goto err;
	}

	token = strtok(NULL, " ");
	while (token != NULL) {
		if (!memcmp(token, "set", (sizeof("set") - 1))) {
			op = 1;
		}else if (!memcmp(token, "-value=", (sizeof("-value=") - 1))) {
			memcpy(buf,(&token[sizeof("-value=") - 1]),2);
		}else if (!memcmp(token, "get", (sizeof("get") - 1))) {
			op = 2;
		}
		token = strtok(NULL, " ");
	}

	if( op == 2) {
		memset(buf, 0, sizeof buf);
		sz = read(afd, buf, 2);	
		if ( sz < 0)
			printf("error reading profile\n");
		else
			printf("Current profile %s\n", buf);

		for (i = 0; i < 5; i++) {
			memset(buf, 0, sizeof buf);
			sz = read(decafd[i], buf, sizeof buf);	
			if ( sz < 0)
				printf("error reading decoder DEC%d\n",i);
			else
				printf("DEC%d:%s\n", i, buf);
		}
		
	} else if( op == 1 ) {
		sz = write(afd, buf, 2); 
		if ( sz < 0) {
			printf("error writing profile\n");
		}			
	}

	close(afd);
err:
	while(decfdcnt--){
		close(decafd[decfdcnt]);
	}	
	return ret_val;
}

int profile_control_handler(void *private_data)
{
	/* Below statement for warning removal
	   to keep prototype intact */
	(void)private_data;
	/* Nothing to do */
	return 0;
}

const char *profile_help_txt = "Select/Read audio profile: \n\
echo \"profile get\" > %s \n\
echo \"profile set -value=x\" > %s \n\
value: 0 - Audio LP, 1 -6 different supported profile \n\
examples: \n\
echo \"profile get\" > %s \n\
echo \"profile set -value=<0 - 6>\" > %s \n";

void profile_help_menu(void)
{
	printf(profile_help_txt, cmdfile, cmdfile);
}


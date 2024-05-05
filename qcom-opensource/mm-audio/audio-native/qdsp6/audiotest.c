/* audiotest.c - native audio test server
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2010, 2012 The Linux Foundation. All rights reserved.
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

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <linux/msm_audio.h>
#include "audiotest_cases.h"
#include "audiotest_def.h"

#define AUDIOTEST_DEFAULT_ID 65523

const char *help_txt =
"After running audio_test without arguments, one can find audio_test file in /data or /tmp\n\
Test case is ran by writting a command string into this file\n\
Format of command string depends on the test case you intend to run\n\
Quit audio_test app: type echo \"quit\" > /data/audio_test\n\
If there are test cases still in executing stage, audio_test exits upon \n\
all test cases finishes. \n\
Audio_test app supports concurrency. If one wishes to issue playback \n\
control command to a particular test case(e.g play mp3), one must specify \n\
an id by giving argument -id=xxx (e.g 123 must be a number) so audio_test \n\
knows how to dispatch control command to a particular test case. \n\
Issue control command: echo \"control_cmd -id=xxx -cmd=xxx\" > /data/audio_test \n\
command can be pause, flush, and etc. \n\
Format of command string is test case specific. Run audio_test -format test_case \n\
to see the format of command string for the desired test case. \n";

char cmdstr[256];

volatile int g_terminate_early = 0;

#define AUDIOTEST_MAX_TH_CXT 10 /* Maximum number of thread context */
struct audiotest_thread_context thread_context[AUDIOTEST_MAX_TH_CXT];
pthread_mutex_t audiotest_mutex = PTHREAD_MUTEX_INITIALIZER;

struct audiotest_thread_context* get_free_context(void) {
	unsigned char i;
	struct audiotest_thread_context *ret_val = NULL;

	pthread_mutex_lock(&audiotest_mutex);
	for (i=0; i < AUDIOTEST_MAX_TH_CXT; i++) {
		if (thread_context[i].used == false) {
			thread_context[i].used = true;
			ret_val = &thread_context[i];
			break;
		}
	}
	pthread_mutex_unlock(&audiotest_mutex);

	if (i == AUDIOTEST_MAX_TH_CXT) {
		fprintf(stderr, "%s : no free context struct\n", __FUNCTION__);
	}

	return ret_val;
}

void free_context(struct audiotest_thread_context *context) {
	unsigned char i;

	pthread_mutex_lock(&audiotest_mutex);
	for (i=0; i < AUDIOTEST_MAX_TH_CXT; i++) {
		if (&thread_context[i] == context) {
			thread_context[i].used = false;
			thread_context[i].cxt_id = AUDIOTEST_DEFAULT_ID;
			thread_context[i].config.private_data = NULL;
			break;
		}
	}
	pthread_mutex_unlock(&audiotest_mutex);
}

struct audiotest_thread_context* find_context(int id) {
	unsigned char i;
	struct audiotest_thread_context *ret_val = NULL;

	pthread_mutex_lock(&audiotest_mutex);
	for (i=0; i < AUDIOTEST_MAX_TH_CXT; i++) {
		if (thread_context[i].cxt_id == id) {
			ret_val = &thread_context[i];
			break;
		}
	}
	pthread_mutex_unlock(&audiotest_mutex);

	return ret_val;
}

void wait_child_threads(void) {
	unsigned char i;

	for (i=0; i < AUDIOTEST_MAX_TH_CXT; i++) {
		if (thread_context[i].used == true)
			pthread_join(thread_context[i].thread, NULL);
	}
}

/* Index by test module ID */
pb_control_func audiotest_pb_controllers[AUDIOTEST_MAX_TEST_MOD] = {
	pcm_play_control_handler, pcm_rec_control_handler,
	mp3_play_control_handler, aac_play_control_handler,
	aac_rec_control_handler, amrnb_play_control_handler,
	amrnb_rec_control_handler, qcp_rec_control_handler,
	NULL, NULL, NULL, atutest_control_handler,
	NULL
};

int pb_control_read_params(void) {
	char *token;
	int ret_val = 0, id = 0;
	struct audiotest_thread_context *context;
	/* Look for ID and cmd */

	token = strtok(NULL, " ");
	if ((token != NULL) &&
		(!memcmp(token,"-id=", (sizeof("-id=") -1)))) {
		id = atoi(&token[sizeof("-id=") - 1]);
		printf("Context id =%d\n", id);
		if (((context = find_context(id)) != NULL) &&
			(audiotest_pb_controllers[context->type] != NULL)) {
		/* Call control function of test module */
			ret_val = audiotest_pb_controllers[context->type](
				context->config.private_data);
		} else {
			ret_val = -1;
		}
	} else {
		ret_val = -1;
	}
	return ret_val;
}

struct audiotest_case_type {
	const char     *case_str;
	case_hd_func   case_hd;
	case_help_menu help_menu;
	case_deinit    deinit;
};

struct audiotest_case_type audiotest_case_list[] = {
	{ "playpcm", pcmplay_read_params, pcmplay_help_menu, NULL },
	{ "recpcm", pcmrec_read_params, pcmrec_help_menu, NULL },
	{ "playmp3", mp3play_read_params, mp3play_help_menu, NULL },
	{ "playaac", aacplay_read_params, aacplay_help_menu, NULL },
	{ "recaac", aacrec_read_params, aacrec_help_menu, NULL  },
	{ "playamrnb", amrnbplay_read_params, amrnbplay_help_menu, NULL },
	{ "recamrnb", amrnbrec_read_params, amrnbrec_help_menu, NULL },
	{ "recqcp", qcprec_read_params, qcprec_help_menu, NULL },
	{ "switchdev", switchdev_read_params, switchdev_help_menu, NULL },
	{ "mastervol", mastervol_read_params, mastervol_help_menu, NULL },
	{ "mastermute", mastermute_read_params, mastermute_help_menu, NULL },
	{ "playtone", atutest_read_params, atutest_help_menu, NULL },
	{ "control_cmd", pb_control_read_params, NULL, NULL },
};

#define AUDIOTEST_MAX_NUM_CASES \
	(sizeof(audiotest_case_list) / sizeof(struct audiotest_case_type))

void audiotest_case_deinit(void) {
	unsigned char i;

	for (i=0; i < AUDIOTEST_MAX_NUM_CASES; i++) {
		if (audiotest_case_list[i].deinit != NULL) {
			audiotest_case_list[i].deinit();
		}
	}
}

void audiotest_cmd_svr(void) {
	const char *exit_str = "quit";
	int fd;
	ssize_t read_count;
	char *token;
#ifdef AUDIOV2
	int control = 0;
#endif
#ifdef _ANDROID_
	if (mknod("/data/audio_test", S_IFIFO | 0666, 0) == 0) {
		fd = open("/data/audio_test", O_RDONLY);
#else
	if (mknod("/tmp/audio_test", S_IFIFO | 0666, 0) == 0) {
		fd = open("/tmp/audio_test", O_RDONLY);
#endif

#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
	audiotest_init_devmgr();
#endif

		while (1) {
			cmdstr[0] = '\0';
			read_count = read(fd, cmdstr, 255);
			if (read_count == 0) {
				/* end of stream */
				sleep(2);
			} else if (read_count < 0) {
				fprintf(stderr, "audio_test: error reading cmd\n");
				break;
			} else {
				cmdstr[read_count-1] = ' ';
				cmdstr[read_count] = '\0'; /* make last byte
							      zero to terminate
							      sucessfully */
				printf("%s\n", cmdstr);
				token = strtok(cmdstr, " ");

				if (!strcmp(token, exit_str)) {
					break; /* given command to quit */
				} else {
					unsigned char i;
					for (i=0; i < AUDIOTEST_MAX_NUM_CASES; i++) {
						if ((!strcmp(token, audiotest_case_list[i].case_str)) &&
							(audiotest_case_list[i].case_hd != NULL)) {
							audiotest_case_list[i].case_hd();
							break;
						}
					}
					if (i == AUDIOTEST_MAX_NUM_CASES) {
						fprintf(stderr, "audio_test: cmd not found\n");
					}
				}
			}
		};
		printf("audio_test: exit server mode\n");
		close(fd);
#ifdef _ANDROID_
        remove("/data/audio_test");
#else
		remove("/tmp/audio_test");
#endif

#if defined(TARGET_USES_QCOM_MM_AUDIO) && defined(AUDIOV2)
		audiotest_deinit_devmgr();
#endif
		wait_child_threads();
		audiotest_case_deinit();
	} else {
		fprintf(stderr, "audio_test: Failed to create server\n");
	}
}

void print_help_menu(void) {
	unsigned char i;

	printf("%s\n", help_txt);
	printf("list of test cases:\n");
	for (i=0; i < AUDIOTEST_MAX_NUM_CASES; i++) {
		printf("%s\n", audiotest_case_list[i].case_str);
	}
}

void printf_format_menu(char *case_name) {
	unsigned char i;

	for (i=0; i < AUDIOTEST_MAX_NUM_CASES; i++) {
		if ((!strcmp(case_name, audiotest_case_list[i].case_str)) &&
			(audiotest_case_list[i].help_menu != NULL)) {
			audiotest_case_list[i].help_menu();
			break;
		}
	}

	if (i == AUDIOTEST_MAX_NUM_CASES) {
		fprintf(stderr, "audio_test: test case does not exist\n");
	}
}

int main(int argc, char **argv)
{
	argc--;
	argv++;

	if (argc > 0) {
		if (!strcmp(argv[0], "-help")) {
			print_help_menu();
		} else if ((!strcmp(argv[0], "-format")) &&
			(argc > 1)) {
			printf_format_menu(argv[1]);
		}
	} else {
		unsigned int i;
		for (i=0; i < AUDIOTEST_MAX_TH_CXT; i++) {
			thread_context[i].used = false;
			thread_context[i].cxt_id = AUDIOTEST_DEFAULT_ID;
		}
		audiotest_cmd_svr();
	}

	return 0;
}

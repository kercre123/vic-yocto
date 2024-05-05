/* Copyright (c) 2010-2011, 2012 The Linux Foundation. All rights reserved.

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
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "audiotest_def.h"
#include <linux/msm_audio.h>

#if defined(TARGET_USES_QCOM_MM_AUDIO)
#include "control.h"

#ifdef QDSP6V2
#include "acdb-loader.h"
#include "acdb-id-mapper.h"
#endif

#endif


#ifdef AUDIOV2
#define DEVMGR_MAX_PLAYBACK_SESSION 4
#define DEVMGR_MAX_RECORDING_SESSION 2
#define DEVMGR_DEFAULT_SID 65523
#define ANC_FF_STEREO_RX_ACDB_ID 26

static int devmgr_devid_rx;
static int devmgr_devid_tx;
static int devmgr_dev_count_tx;
static int devmgr_dev_count_rx;
static int devmgr_init_flag;
static int devmgr_mixer_init_flag;
unsigned short devmgr_sid_rx_array[DEVMGR_MAX_PLAYBACK_SESSION];
unsigned short devmgr_sid_tx_array[DEVMGR_MAX_PLAYBACK_SESSION];
static int devmgr_sid_count_rx = 0;
static int devmgr_sid_count_tx = 0;

#ifdef QDSP6V2
static int acdb_init_count;
static int devmgr_mvslp_devid_rx;
static int devmgr_mvslp_devid_tx;
static int mvs_lp_flag = 0;
#endif

const char *devctl_help_text =
"\nDevice Control Help: MAINLY USED FOR SWITCHING THE AUDIO DEVICE.	\n\
All Active playbacks will be routed to Device mentioned in this        \n\
command. Device IDs are generated dynamically from the driver.		\n"
#ifdef QDSP6V2
"\nTo Switch Rx Devices\n"
"Usage: echo \"devctl -cmd=dev_switch_rx -dev_id=x\" > /data/audio_test \n\n"
" \nTo test Proxy port:\n"
"Usage: echo \"devctl -cmd=dev_switch_rx -proxyporttest=<proxy port dev-id>\n \
-time=<out file log time, this is actually a loopcounter> -outfile=<out put \
file URL> -samplerate=<rate>\n \
-channels=<chan>\" > /data/audio_test	\n"
"Note:First play a decode session and then use this cmd to log pcm from proxy\n\n"
#else
"Usage: echo \"devctl -cmd=dev_switch_rx -dev_id=x\" > /data/audio_test \n"
#endif
"\nTo Switch Tx Devices\n"
"echo \"devctl -cmd=dev_switch_tx -dev_id=x\" > /data/audio_test	\n\n\
For making voice loopback from application side:-	    	\n\n\
To start a voice call, input these commands:		    	\n"
#ifdef QDSP6V2
"\nTo load voice acdb                                             \n\
echo \"devctl -cmd=load_voice_acdb -txdev_id=x -rxdev_id=y\" >  \n\
/data/audio_test                                                \n"
#endif
"echo \"devctl -cmd=voice_route -txdev_id=x -rxdev_id=y\" >  	\n\
/data/audio_test					    	\n\
echo \"devctl -cmd=enable_dev -dev_id=x\" > /data/audio_test	\n\
echo \"devctl -cmd=enable_dev -dev_id=y\" > /data/audio_test	\n\
echo \"devctl -cmd=start_voice\" > /data/audio_test	    	\n\n\
\nTo mute/unmute:						    	\n\
echo \"devctl -cmd=voice_tx_mute -mute=z\" > /data/audio_test	\n\
To end started voice call, input these commands: 	    	\n\
echo \"devctl -cmd=end_voice\" > /data/audio_test	    	\n\
echo \"devctl -cmd=disable_dev -dev_id=x\" > /data/audio_test	\n\
echo \"devctl -cmd=disable_dev -dev_id=y\" > /data/audio_test	\n\
where x,y = any of the supported device IDs listed below,           	\n\
z = 0/1 where 0 is unmute, 1 is mute 				    	\n\
echo \"devctl -cmd=loopback_set -txdev_id=x -rxdev_id=y\" >/data/audio_test\n\
echo \"devctl -cmd=loopback_reset -txdev_id=x -rxdev_id=y\" >  	\n\
/data/audio_test					    	\n"
#ifdef QDSP5V2
"echo \"devctl -cmd=mute_dev -dev_id=x -mute=0/1\" > /data/audio_test\n"
#endif
#ifdef QDSP6V2
"\nTo enable active noise cancellation:				\n\
echo \"devctl -cmd=enable_dev -dev_id=v\" > /data/audio_test	\n\
echo \"devctl -cmd=enable_anc\" > /data/audio_test		\n\
where v = ANC Device ID							\n\
If ANC Device is already enabled, then just run the enable_anc command  \n\
To disable active noise cancellation:				\n\
echo \"devctl -cmd=disable_anc\" > /data/audio_test		\n\
echo \"devctl -cmd=loopback_set -txdev_id=x -rxdev_id=y\" >/data/audio_test\n\
echo \"devctl -cmd=loopback_reset -txdev_id=x -rxdev_id=y\" > /data/audio_test\n\
\nTo do device switch during VoIP call				\n\
echo \"devctl -cmd=mvs_dev_switch -rxdev_id=x -rxdev_id=x\"      \n > /data/audio_test	\n"
#endif
"Note:                                                               	\n\
(i)   Handset RX/TX is set as default device for all playbacks/recordings \n\
(ii)  After a device switch, audio will be routed to the last set   	\n\
      device                                                        	\n\
(iii) Device List and their corresponding IDs can be got using      	\n\
      \"mm-audio-native-test -format devctl\" and also is displayed 	\n\
      during beginning of any playback session                      \n";

void devctl_help_menu(void)
{

	int i, alsa_ctl, dev_cnt, device_id;
	const char **device_names;

	printf("%s\n", devctl_help_text);
	if (!devmgr_mixer_init_flag) {
		alsa_ctl = msm_mixer_open("/dev/snd/controlC0", 0);
		if (alsa_ctl < 0)
			perror("Fail to open ALSA MIXER\n");
		else
			devmgr_mixer_init_flag = 1;
	}
	if (devmgr_mixer_init_flag) {
		dev_cnt = msm_get_device_count();
		device_names = msm_get_device_list();
		for (i = 0; i < dev_cnt;) {
			device_id = msm_get_device(device_names[i]);
			if (device_id >= 0)
				printf("device name %s:dev_id: %d\n",
							device_names[i],
			device_id);
			i++;
		}
	}
}

int devmgr_disable_device(int dev_id, unsigned short route_dir)
{
	if (route_dir == DIR_RX){
		devmgr_dev_count_rx--;
		if (devmgr_dev_count_rx == 0) {
			if (msm_en_device(dev_id, 0) < 0)
				return -1;
		}
	}
	else if (route_dir == DIR_TX){
		devmgr_dev_count_tx--;
		if (devmgr_dev_count_tx == 0) {
		if (msm_en_device(dev_id, 0) < 0)
			return -1;
	}
	}
	else
		perror("devmgr_disable_device: Invalid route direction\n");
	return 0;
}

int devmgr_enable_device(int dev_id, unsigned short route_dir)
{

	if (msm_en_device(dev_id, 1) < 0)
		return -1;
	if (route_dir == DIR_RX)
		devmgr_dev_count_rx++;
	else if (route_dir == DIR_TX)
		devmgr_dev_count_tx++;
	else
		perror("devmgr_enable_device: Invalid route direction\n");
	return 0;
}

int devmgr_register_session(unsigned short session_id, unsigned short route_dir)
{
#ifdef QDSP6V2
	int acdb_id;
#endif

	printf("devmgr_register_session: Registering Session ID = %d\n",
								session_id);
	if (route_dir == DIR_RX){
		if ((devmgr_sid_count_rx < DEVMGR_MAX_PLAYBACK_SESSION) &&
		(devmgr_sid_rx_array[devmgr_sid_count_rx] == DEVMGR_DEFAULT_SID))
			devmgr_sid_rx_array[devmgr_sid_count_rx++] = session_id;

		if (devmgr_enable_device(devmgr_devid_rx, DIR_RX) < 0){
			perror("could not enable RX device\n");
			return -1;
		}
#ifdef QDSP6V2
		acdb_mapper_get_acdb_id_from_dev_id(devmgr_devid_rx, &acdb_id);
		acdb_loader_send_audio_cal(acdb_id, msm_get_device_capability(devmgr_devid_rx));
#endif
		if (msm_route_stream(DIR_RX, session_id, devmgr_devid_rx, 1) < 0) {
			perror("could not route stream to Device\n");
			if (devmgr_disable_device(devmgr_devid_rx, DIR_RX) < 0)
				perror("could not disable device\n");
			return -1;
		}
	}
	else if (route_dir == DIR_TX){
		if ((devmgr_sid_count_tx < DEVMGR_MAX_RECORDING_SESSION) &&
		(devmgr_sid_tx_array[devmgr_sid_count_tx] == DEVMGR_DEFAULT_SID))
			devmgr_sid_tx_array[devmgr_sid_count_tx++] = session_id;
		if (devmgr_enable_device(devmgr_devid_tx, DIR_TX) < 0){
			perror("could not enable TX device\n");
			return -1;
		}
#ifdef QDSP6V2
		acdb_mapper_get_acdb_id_from_dev_id(devmgr_devid_tx, &acdb_id);
		acdb_loader_send_audio_cal(acdb_id, msm_get_device_capability(devmgr_devid_tx));
#endif
		if (msm_route_stream(DIR_TX, session_id, devmgr_devid_tx, 1) < 0) {
		perror("could not route stream to Device\n");
			if (devmgr_disable_device(devmgr_devid_tx, DIR_TX) < 0)
			perror("could not disable device\n");
		return -1;
	}
	}
	else
		perror("devmgr_register_session: Invalid route direction.\n");
	return 0;
}

int devmgr_unregister_session(unsigned short session_id, unsigned short route_dir)
{

	int index = 0;
	printf("devmgr_unregister_session: Unregistering Session ID = %d\n",
	session_id);
	if (route_dir == DIR_RX){
		while (index < devmgr_sid_count_rx) {
			if (session_id == devmgr_sid_rx_array[index])
			break;
		index++;
	}
		while (index < (devmgr_sid_count_rx-1)) {
			devmgr_sid_rx_array[index]  =  devmgr_sid_rx_array[index+1];
		index++;
	}
	/* Reset the last entry */
		devmgr_sid_rx_array[index]         = DEVMGR_DEFAULT_SID;
		devmgr_sid_count_rx--;

		if (msm_route_stream(DIR_RX, session_id, devmgr_devid_rx, 0) < 0)
		perror("could not de-route stream to Device\n");

		if (devmgr_disable_device(devmgr_devid_rx, DIR_RX) < 0){
			perror("could not disable RX device\n");
		}
	}else if(route_dir == DIR_TX){
		while (index < devmgr_sid_count_tx) {
			if (session_id == devmgr_sid_tx_array[index])
				break;
			index++;
		}
		while (index < (devmgr_sid_count_tx-1)) {
			devmgr_sid_tx_array[index]  =  devmgr_sid_tx_array[index+1];
			index++;
		}
		/* Reset the last entry */
		devmgr_sid_tx_array[index]         = DEVMGR_DEFAULT_SID;
		devmgr_sid_count_tx--;

		if (msm_route_stream(DIR_TX, session_id, devmgr_devid_tx, 0) < 0)
			perror("could not de-route stream to Device\n");

		if (devmgr_disable_device(devmgr_devid_tx, DIR_TX) < 0){
			perror("could not disable TX device\n");
		}
	}
	else
		perror("devmgr_unregister_session: Invalid route direction\n");
	return 0;
}
#ifdef QDSP6V2
int devmgr_enable_voice_device(int devid_rx, int devid_tx)
{
	int ret = 0;

	devmgr_mvslp_devid_rx = devid_rx;
	devmgr_mvslp_devid_tx = devid_tx;

	ret = msm_en_device(devmgr_mvslp_devid_rx, 1);
        if (ret < 0)
                printf("Error %d enabling rx device  devid=%d \n", ret, devmgr_mvslp_devid_rx);

        ret = msm_en_device(devmgr_mvslp_devid_tx, 1);
        if (ret < 0)
                printf("Error %d enabling tx device  devid=%d\n", ret, devmgr_mvslp_devid_tx);

	mvs_lp_flag = 1;

	return ret;

}

int devmgr_disable_voice_device(void)
{
	int ret = 0;

	ret = msm_en_device(devmgr_mvslp_devid_rx, 0);
        if (ret < 0)
                printf("Error %d disabling rx device  devid=%d\n", ret, devmgr_mvslp_devid_rx);

        ret = msm_en_device(devmgr_mvslp_devid_tx, 0);
        if (ret < 0)
                printf("Error %d disabling tx device  devid=%d\n", ret, devmgr_mvslp_devid_tx);

	mvs_lp_flag = 0;

	return ret;

}


static int devmgr_mvs_dev_switch(int devid_rx, int devid_tx)
{
	int acdb_id_tx = 0;
        int acdb_id_rx = 0;

	printf(" route to new device: rxid=%d, txid=%d \n", devid_rx, devid_tx);
	msm_route_voice(devid_rx, devid_tx, 1);

	printf(" disable current device: rx=%d, tx=%d\n", devmgr_mvslp_devid_rx, devmgr_mvslp_devid_tx);
	msm_en_device(devmgr_mvslp_devid_rx, 0);
	msm_en_device(devmgr_mvslp_devid_tx, 0);

	printf(" load new acdb \n");
	acdb_mapper_get_acdb_id_from_dev_id(devid_tx, &acdb_id_tx);
        acdb_mapper_get_acdb_id_from_dev_id(devid_rx, &acdb_id_rx);

        acdb_loader_send_voice_cal(acdb_id_rx, acdb_id_tx);

	devmgr_mvslp_devid_rx = devid_rx;
	devmgr_mvslp_devid_tx = devid_tx;

	printf(" enable new devices \n");
        msm_en_device(devmgr_mvslp_devid_rx, 1);
	msm_en_device(devmgr_mvslp_devid_tx, 1);

	return 0;


}
#endif

void audiotest_deinit_devmgr(void)
{

	int alsa_ctl;

	if (devmgr_init_flag && devmgr_mixer_init_flag) {
		alsa_ctl = msm_mixer_close();
		if (alsa_ctl < 0)
			perror("Fail to close ALSA MIXER\n");
		else{
			printf("%s: Closed ALSA MIXER\n", __func__);
			devmgr_mixer_init_flag = 0;
			devmgr_init_flag = 0;
		}
	}
#ifdef QDSP6V2
        if (acdb_init_count) {
                acdb_loader_deallocate_ACDB();
                acdb_init_count--;
        }
#endif
}

void audiotest_init_devmgr(void)
{

	int i, alsa_ctl, device_id, dev_cnt;
	const char **device_names;
	const char *def_device_rx = "handset_rx";
	const char *def_device_tx = "handset_tx";

	if (!devmgr_mixer_init_flag) {
		alsa_ctl = msm_mixer_open("/dev/snd/controlC0", 0);
		if (alsa_ctl < 0)
			perror("Fail to open ALSA MIXER\n");
		else
			devmgr_mixer_init_flag = 1;
	}

#ifdef QDSP6V2
	if (!acdb_init_count) {
		if (acdb_loader_init_ACDB() != 0) {
			printf("ACDB init failed!\n");
			return ;
		}
		acdb_init_count++;
	}
#endif
	if (devmgr_mixer_init_flag) {
		printf("Device Manager: List of Devices supported: \n");
		dev_cnt = msm_get_device_count();
		device_names = msm_get_device_list();
		for (i = 0; i < dev_cnt;) {
			device_id = msm_get_device(device_names[i]);
			if (device_id >= 0)
				printf("device name %s:dev_id: %d\n", device_names[i], device_id);
			i++;
		}
		printf("Setting default RX =  %d\n", devmgr_devid_rx);
		devmgr_devid_rx = msm_get_device(def_device_rx);
#ifdef QDSP6V2
		devmgr_mvslp_devid_rx = devmgr_devid_rx;
#endif
		printf("Setting default TX =  %d\n", devmgr_devid_tx);
		devmgr_devid_tx = msm_get_device(def_device_tx);
#ifdef QDSP6V2
		devmgr_mvslp_devid_tx = devmgr_devid_tx;
#endif
		for (i = 0; i < DEVMGR_MAX_PLAYBACK_SESSION; i++)
			devmgr_sid_rx_array[i] = DEVMGR_DEFAULT_SID;
		for (i = 0; i < DEVMGR_MAX_RECORDING_SESSION; i++)
			devmgr_sid_tx_array[i] = DEVMGR_DEFAULT_SID;
		devmgr_init_flag = 1;
	}
    return;
}

struct audio_pvt_data audio_test;

#ifdef QDSP6V2

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

static void create_wav_header(int Datasize)
{
	append_header.Chunk_size = Datasize + 8 + 16 + 12;
	append_header.Chunk_data_size = Datasize;
	append_header.Sample_rate= audio_test.freq;
	append_header.Number_Channels= audio_test.channels;

	return;
}

void proxyport_test( int value)
{
	int proxydrvrfd = 0;
	FILE *outfilefp = 0;
	struct msm_audio_config config;
	int retval = 0;
	int i = 0;
	int len = 0;
	int totallen = 0;
	char *buff = 0;
	int buffsize = 0;

	proxydrvrfd = open("/dev/msm_pcm_in_proxy",O_RDONLY);

	if (proxydrvrfd <= 0) {
		perror("proxyporttest:open drvr failed\n");
		goto err;
	}

	memset(&config, 0, sizeof(config));
	retval = ioctl(proxydrvrfd, AUDIO_GET_CONFIG, &config);
	if (retval < 0) {
		perror("Proxyporttest:set config failed\n");
		goto err;
	}
	config.sample_rate = audio_test.freq;
	config.channel_count= audio_test.channels;

	retval = ioctl(proxydrvrfd, AUDIO_SET_CONFIG, &config);
	if (retval < 0) {
		perror("Proxyporttest:set config failed\n");
		goto err;
	}

	/* to know the buffer size calculated by driver
	 * corresponding to the sample rate and channels set
	 */
	retval = ioctl(proxydrvrfd, AUDIO_GET_CONFIG, &config);
	if (retval < 0) {
		perror("Proxyporttest:set config failed\n");
		goto err;
	}

	buffsize = config.buffer_size;
	buff = malloc(buffsize);

	printf("sample rate %d channels %d\n",
		config.sample_rate, config.channel_count);

	retval = ioctl(proxydrvrfd, AUDIO_START, NULL);
	if (retval < 0) {
		perror("proxyporttest:START failed\n");
		goto err;
	}

	outfilefp  = fopen(audio_test.outfile, "wb");

	if (outfilefp == 0) {
		perror("proxyporttest:out file drvr open failed\n");
		goto err;
	}

	/* Set aside Space for Wave Header */
	fseek(outfilefp , sizeof(append_header), SEEK_SET);

	for(i = 0, totallen = 0; i < value; i++, totallen+=len){
		len = read(proxydrvrfd, buff, buffsize);
		if (len == buffsize)
			printf("read chuck(%d) from proxy port success\n", i);

		retval = fwrite(buff, buffsize, 1, outfilefp);
		if (retval > 0)
			printf("write chuck(%d) to file success\n", i);
	}

	create_wav_header(totallen);
	fseek(outfilefp, 0, SEEK_SET);
	fwrite((char *)&append_header, sizeof(append_header), 1, outfilefp);
	fclose(outfilefp);
	close(proxydrvrfd);
	if(buff)
		free(buff);
	printf("\n\nProxy port test SUCCESS, find the PCM samples(bytes %d) at %s\n\n",
		totallen, audio_test.outfile);
	return;

err:
	printf("\n\nProxy port test failed.\n\n");
	return;
}
#endif


int devmgr_devctl_handler()
{

	char *token;
	int ret_val = 0, sid, dev_source, dev_dest, index, dev_id,
		mute, txdev_id, rxdev_id;
	int loopcounter = 1000;
#ifdef QDSP6V2
	char *anc_device = "anc_headset_stereo_rx";
	int device = 0;
	int txdev_acdb_id, rxdev_acdb_id;
#endif
	token = strtok(NULL, " ");

	if (token != NULL) {
		if (!memcmp(token, "-cmd=", (sizeof("-cmd=") - 1))) {
			token = &token[sizeof("-cmd=") - 1];
			if (!strcmp(token, "dev_switch_rx")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-dev_id=",
						(sizeof("-dev_id=") - 1))) {
					dev_dest = atoi(&token
						[sizeof("-dev_id=") - 1]);
					dev_source = devmgr_devid_rx;
					if (devmgr_mixer_init_flag &&
							devmgr_init_flag) {
						if (dev_source != dev_dest) {
							printf("%s: Device Switch from = %d to = %d\n",	__func__,
						dev_source, dev_dest);
							if (devmgr_sid_count_rx == 0){
								devmgr_devid_rx = dev_dest;
								printf("%s: Device Switch Success\n",__func__);
							}

							for (index = 0; index < devmgr_sid_count_rx;
								 index++) {
#ifdef QDSP6V2
								acdb_mapper_get_acdb_id_from_dev_id(
									devmgr_sid_rx_array[index], &rxdev_acdb_id);
								acdb_loader_send_audio_cal(rxdev_acdb_id,
									msm_get_device_capability(
										devmgr_sid_rx_array[index]));
#endif
								msm_route_stream
                          (DIR_RX, devmgr_sid_rx_array[index],
								dev_dest, 1);
								if (
						(devmgr_disable_device
                                (dev_source, DIR_RX)) == 0) {
									msm_route_stream(DIR_RX, devmgr_sid_rx_array[index],
								dev_source, 0);
									if (
                                (devmgr_enable_device(dev_dest, DIR_RX)
                                    ) == 0) {
                            printf("%s: Device Switch Success\n",__func__);
							devmgr_devid_rx = dev_dest;
											}
							        }
							}
						} else {
							printf("%s(): Device has not changed as current device is:%d\n",
						__func__, dev_dest);
						}
					}
				}
#ifdef QDSP6V2
				if (!memcmp(token, "-proxyporttest=", (sizeof("-proxyporttest=") - 1))) {
					dev_id = atoi(&token [sizeof("-proxyporttest=") - 1]);
				printf("-->proxy port dev id %d\n", dev_id);

				/* setup defaults */
				audio_test.freq = 48000;
				audio_test.channels = 2;
				loopcounter = 1000;

				while (token != NULL) {
					if (!memcmp(token, "-outfile=", (sizeof("-outfile=") - 1))) {
						token = &token[sizeof("-outfile=") - 1];
						audio_test.outfile = token;
						printf("-->outfile %s\n", token);
					} else if (!memcmp(token, "-channels=", (sizeof("-channels=") - 1))) {
						audio_test.channels=atoi(&token[sizeof("-channels=") - 1]);
						printf("-->channels %d\n", audio_test.channels);
					} else if (!memcmp(token,"-samplerate=", (sizeof("-samplerate=" - 1)))) {
						audio_test.freq = atoi(&token[sizeof("-samplerate=") - 1]);
						printf("-->sample rate %d\n", audio_test.freq);
					} else if (!memcmp(token, "-time=", (sizeof("-time=") - 1))) {
						loopcounter = atoi(&token[sizeof("-time=") - 1]);
						printf("-->loopcounter %d\n", loopcounter);
					}
					token = strtok(NULL, " ");
				}

					for (index = 0; index < devmgr_sid_count_rx; index++) {
						msm_route_stream (DIR_RX, devmgr_sid_rx_array[index], dev_id, 1);
						if ( (devmgr_enable_device(dev_id, DIR_RX) ) == 0) {
							printf("Proxy Device Switch on Success\n");
						}
						else
							printf("Proxy Device switch on failure\n");
					}

					proxyport_test(loopcounter);

					for (index = 0; index < devmgr_sid_count_rx; index++) {
						msm_route_stream (DIR_RX, devmgr_sid_rx_array[index], dev_id, 0);
						if ( (devmgr_disable_device(dev_id, DIR_RX) ) == 0) {
							printf("Proxy Device Switch off Success\n");
						}
						else
							printf("Proxy Device switch off failure\n");
					}

				}
#endif /*QDSP6V2*/
			} else if (!strcmp(token, "dev_switch_tx")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-dev_id=",
						(sizeof("-dev_id=") - 1))) {
					dev_dest = atoi(&token
						[sizeof("-dev_id=") - 1]);
					dev_source = devmgr_devid_tx;
					if (devmgr_mixer_init_flag &&
							devmgr_init_flag) {
						if (dev_source != dev_dest) {
							printf("%s: Device Switch from = %d to = %d\n",	__func__,
						dev_source, dev_dest);
							if (devmgr_sid_count_tx == 0){
								devmgr_devid_tx = dev_dest;
								printf("%s: Device Switch Success\n",__func__);
							}

							for (index = 0; index < devmgr_sid_count_tx; index++) {
#ifdef QDSP6V2
								acdb_mapper_get_acdb_id_from_dev_id(
									devmgr_sid_tx_array[index], &txdev_acdb_id);
								acdb_loader_send_audio_cal(txdev_acdb_id,
									msm_get_device_capability(
										devmgr_sid_tx_array[index]));
#endif
								msm_route_stream
                          (DIR_TX, devmgr_sid_tx_array[index],
                                         dev_dest, 1);
								if (
                          (devmgr_disable_device
                                (dev_source, DIR_TX)) == 0) {
									msm_route_stream(DIR_TX, devmgr_sid_tx_array[index],
								dev_source, 0);
									if (
                                (devmgr_enable_device(dev_dest, DIR_TX)
							) == 0) {
					printf("%s: Device Switch Success\n",__func__);
							devmgr_devid_tx = dev_dest;
									}
								}
							}
						} else {
							printf("%s(): Device has not changed as current device is:%d\n",
						__func__, dev_dest);
						}
					}
				}
			} else if (!strcmp(token, "enable_dev")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-dev_id=", (sizeof
					("-dev_id=") - 1))) {
					dev_id = atoi(&token[sizeof("-dev_id=")
									- 1]);
					msm_en_device(dev_id, 1);
				}
			} else if (!strcmp(token, "disable_dev")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-dev_id=", (sizeof
					("-dev_id=") - 1))) {
					dev_id = atoi(&token[sizeof("-dev_id=")
									- 1]);
					msm_en_device(dev_id, 0);
				}
			} else if (!strcmp(token, "rx_route")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-dev_id=", (sizeof
					("-dev_id=") - 1))) {
					dev_id = atoi(&token[sizeof("-dev_id=")
									- 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-sid=", (sizeof
							("-sid=") - 1))) {
						sid = atoi(&token[sizeof
							("-sid=") - 1]);
						devmgr_devid_rx = dev_id;
						devmgr_register_session
							(sid, DIR_RX);
					}
				}
			} else if (!strcmp(token, "mute_dev")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-dev_id=", (sizeof
					("-dev_id=") - 1))) {
					dev_id = atoi(&token[sizeof("-dev_id=")
									- 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-mute=", (sizeof
							("-mute=") - 1))) {
						mute = atoi(&token[sizeof
							("-mute=") - 1]);
						msm_device_mute
							(dev_id, mute);
					}
				}
			} else if (!strcmp(token, "tx_route")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-dev_id=", (sizeof
					("-dev_id=") - 1))) {
					dev_id = atoi(&token[sizeof("-dev_id=")
									- 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-sid=", (sizeof
							("-sid=") - 1))) {
						sid = atoi(&token[sizeof
							("-sid=") - 1]);
						devmgr_devid_tx = dev_id;
						devmgr_register_session
							(sid, DIR_TX);
					}
				}
			} else if (!strcmp(token, "rx_deroute")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-dev_id=", (sizeof
							("-dev_id=") - 1))) {
					dev_id = atoi(&token[sizeof("-dev_id=")
									- 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-sid=", (sizeof
							("-sid=") - 1))) {
						sid = atoi(&token[sizeof
							("-sid=") - 1]);
						msm_route_stream
							(DIR_RX, sid, dev_id, 0);
					}
				}
			} else if (!strcmp(token, "tx_deroute")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-dev_id=", (sizeof
							("-dev_id=") - 1))) {
					dev_id = atoi(&token[sizeof("-dev_id=")
									- 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-sid=", (sizeof
							("-sid=") - 1))) {
						sid = atoi(&token[sizeof
							("-sid=") - 1]);
						msm_route_stream
							(DIR_TX, sid, dev_id, 0);
					}
				}
			} else if (!strcmp(token, "voice_route")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-txdev_id=", (sizeof
							("-txdev_id=") - 1))) {
					txdev_id = atoi(&token[sizeof
							("-txdev_id=") - 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-rxdev_id=",
						(sizeof("-rxdev_id=") - 1))) {
						rxdev_id = atoi(&token[sizeof
							("-rxdev_id=") - 1]);
						msm_route_voice
							(rxdev_id, txdev_id, 1);
					}
				}
			} else if (!strcmp(token, "voice_deroute")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-txdev_id=", (sizeof
							("-txdev_id=") - 1))) {
					txdev_id = atoi(&token[sizeof
							("-txdev_id=") - 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-rxdev_id=",
						(sizeof("-rxdev_id=") - 1))) {
						rxdev_id = atoi(&token[sizeof
							("-rxdev_id=") - 1]);
						msm_route_voice
							(txdev_id, rxdev_id, 0);
					}
				}
			} else if (!strcmp(token, "voice_rx_vol")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-volume=", (sizeof
							("-volume=") - 1))) {
					int volume = atoi(&token[sizeof
							("-volume=") - 1]);
					msm_set_voice_rx_vol(volume);
				}
			} else if (!strcmp(token, "start_voice")) {
			       /*
				* User will be able to do voice loopback and
				* mute or unmute voice path. This test app
				* exercises the voice call API just from
				* application side; Same should be invoked
				* from modem side to make it work
				*/
				msm_start_voice();
			} else if (!strcmp(token, "end_voice")) {
				msm_end_voice();
			} else if (!strcmp(token, "voice_tx_mute")) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-mute=", (sizeof
							("-mute=") - 1))) {
					int mute = atoi(&token[sizeof
							("-mute=") - 1]);
					msm_set_voice_tx_mute(mute);
				}
			}
#ifdef QDSP6V2
			else if (!strcmp(token, "enable_anc")) {
				ret_val |= acdb_loader_send_anc_cal(ANC_FF_STEREO_RX_ACDB_ID);
				device = msm_get_device(anc_device);
				ret_val |= msm_enable_anc(device, 1);
				printf("Returned from enabling anc %d\n", ret_val);
			} else if (!strcmp(token, "disable_anc")) {
				device = msm_get_device(anc_device);
				ret_val = msm_enable_anc(device, 0);
				printf("Returned from disabling anc %d\n", ret_val);
			} else if (!strcmp(token, "load_voice_acdb")) {
				token = strtok(NULL, " ");
                                if (!memcmp(token, "-txdev_id=", (sizeof
                                                        ("-txdev_id=") - 1))) {
                                        txdev_id = atoi(&token[sizeof
                                                        ("-txdev_id=") - 1]);
                                        token = strtok(NULL, " ");
                                        if (!memcmp(token, "-rxdev_id=",
                                                (sizeof("-rxdev_id=") - 1))) {
                                                rxdev_id = atoi(&token[sizeof
                                                        ("-rxdev_id=") - 1]);

						ret_val = acdb_mapper_get_acdb_id_from_dev_id(txdev_id, &txdev_acdb_id);
						if (ret_val) {
							printf("Error %d: Get ACDB id failed.\n", ret_val);
							return ret_val;
						} else {
							ret_val = acdb_mapper_get_acdb_id_from_dev_id(rxdev_id, &rxdev_acdb_id);
							if (ret_val) {
								printf("Error %d: Get ACDB id failed.\n", ret_val);
								return ret_val;
							}
						}
						acdb_loader_send_voice_cal(txdev_acdb_id, rxdev_acdb_id);
                                        }
                                }
			}
#endif
			else if (!strcmp(token, "loopback_set")) {
				printf("Loopback Set\n");
				token = strtok(NULL, " ");
				if (!memcmp(token, "-txdev_id=", (sizeof
							("-txdev_id=") - 1))) {
					txdev_id = atoi(&token[sizeof
							("-txdev_id=") - 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-rxdev_id=",
						(sizeof("-rxdev_id=") - 1))) {
						rxdev_id = atoi(&token[sizeof
							("-rxdev_id=") - 1]);
						msm_snd_dev_loopback
							(rxdev_id, txdev_id, 1);
					}
				}
			} else if (!strcmp(token, "loopback_reset")) {
				printf("Loopback Reset\n");
				token = strtok(NULL, " ");
				if (!memcmp(token, "-txdev_id=", (sizeof
							("-txdev_id=") - 1))) {
					txdev_id = atoi(&token[sizeof
							("-txdev_id=") - 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-rxdev_id=",
						(sizeof("-rxdev_id=") - 1))) {
						rxdev_id = atoi(&token[sizeof
							("-rxdev_id=") - 1]);
						msm_snd_dev_loopback
							(rxdev_id, txdev_id, 0);
					}
				}
			}
#ifdef QDSP6V2
			else if ((!strcmp(token, "mvs_dev_switch")) && (mvs_lp_flag == 1)) {
				token = strtok(NULL, " ");
				if (!memcmp(token, "-rxdev_id=", (sizeof
							("-rxdev_id=") - 1))) {
					rxdev_id = atoi(&token[sizeof
							("-rxdev_id=") - 1]);
					token = strtok(NULL, " ");
					if (!memcmp(token, "-txdev_id=",
						(sizeof("-txdev_id=") - 1))) {
						txdev_id = atoi(&token[sizeof
							("-txdev_id=") - 1]);
						devmgr_mvs_dev_switch(rxdev_id, txdev_id);
					}
				}
			}
#endif
			else {
				printf("%s: Invalid command", __func__);
				printf("%s\n", devctl_help_text);
				ret_val = -1;
			}
		} else {
			printf("%s: Not a devmgr command\n", __func__);
			printf("%s\n", devctl_help_text);
			ret_val = -1;
		}
	}

	return ret_val;
}

int devctl_read_params(void)
{

	if (devmgr_devctl_handler() < 0) {
		printf("%s() Invalid Command\n", __func__);
		return -1;
	}
	return 0;
}
#endif



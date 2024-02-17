/* lpatest.c - native lpa rendering test application
 *
 * Based on native fm test application platform/system/extras/sound/playwav.c
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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "audiotest_def.h"
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/msm_audio.h>
#include <errno.h>
#define MP3TEST_BUFSZ 250000	//153600, 524288
#define MP3TEST_NUM_BUF 4

#define EOS 1
#define MAX_BITSTREAM_ERROR 5

struct pmem_buf {
	struct msm_audio_aio_buf pbuf;
	int flag;
};

typedef struct TIMESTAMP {
	unsigned long LowPart;
	unsigned long HighPart;
} __attribute__ ((packed)) TIMESTAMP;

struct meta_in {
	unsigned short offset;
	TIMESTAMP ntimestamp;
	unsigned int nflags;
} __attribute__ ((packed));

struct meta_out {
	unsigned short offset;
	TIMESTAMP ntimestamp;
	unsigned int nflags;
	unsigned short errflag;
	unsigned short sample_frequency;
	unsigned short channel;
	unsigned int tick_count;
} __attribute__ ((packed));

static struct pmem_buf data_buf[4];
/* static int start_dec; */
static pthread_mutex_t avail_lock;
static pthread_cond_t avail_cond;
static pthread_mutex_t consumed_lock;
static pthread_cond_t consumed_cond;
static int data_is_available = 0;
static int data_is_consumed = 0;
static unsigned int pcm_sourced = 0;

static int in_free_cnt;
static int eof = 0;
static int complete_close;

static void wait_for_data(void)
{
	pthread_mutex_lock(&avail_lock);

	while (data_is_available == 0) {
		pthread_cond_wait(&avail_cond, &avail_lock);
	}
	data_is_available = 0;
	pthread_mutex_unlock(&avail_lock);
}

void data_available(void)
{
	pthread_mutex_lock(&avail_lock);
	if (data_is_available == 0) {
		data_is_available = 1;
		pthread_cond_broadcast(&avail_cond);
	}
	pthread_mutex_unlock(&avail_lock);
}

static int fill_buffer(void *buf, unsigned sz, void *cookie)
{
	struct meta_in meta;
	struct audio_pvt_data *audio_data =
	    (struct audio_pvt_data *)cookie;
	unsigned cpy_size =
	    (sz < audio_data->avail ? sz : audio_data->avail);

	if (audio_data->avail == 0) {
		printf(" no data avail \n");
		return -1;
	}

	meta.ntimestamp.LowPart =
	    ((audio_data->frame_count * 20000) & 0xFFFFFFFF);
	meta.ntimestamp.HighPart =
	    (((unsigned long long)(audio_data->frame_count *
				   20000) >> 32) & 0xFFFFFFFF);

	meta.offset = sizeof(struct meta_in);
	meta.nflags = 0;
	audio_data->frame_count++;
#ifdef DEBUG_LOCAL
	printf("Meta In High part is %lu\n", meta.ntimestamp.HighPart);
	printf("Meta In Low part is %lu\n", meta.ntimestamp.LowPart);
	printf("Meta In ntimestamp: %llu\n", (((unsigned long long)
					      meta.ntimestamp.
					       HighPart << 32) +
					      meta.ntimestamp.LowPart));
#endif
	memcpy(buf, &meta, sizeof(struct meta_in));
	memcpy(((char*)buf + sizeof(struct meta_in)), audio_data->next, cpy_size);

	audio_data->next += cpy_size;
	audio_data->avail -= cpy_size;

	return cpy_size + sizeof(struct meta_in);

}

static void *lpa_dec(void *arg)
{
	struct meta_out meta;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *)arg;
	int afd = audio_data->afd;
	int ntfd = audio_data->ntfd;
	unsigned long long *time;
	int len, total_len;
	len = 0;
	total_len = 0;
	int index = 0;
	int rc = 0;

	printf("lpa_read Thread, recsize=%d \n", audio_data->recsize);

	do {
		if (audio_data->bitstream_error == 1) {
			printf
			    ("Bitstream error notified, exit read thread\n");
			break;
		}
		if (in_free_cnt == 0) {
			wait_for_data();
			printf("wait for LPA free buffer\n");
		}
		len = read(ntfd, audio_data->recbuf, audio_data->recsize);
		if (len < 0) {
			if ((audio_data->flush_enable == 1 ||
			     audio_data->outport_flush_enable == 1)
			    && errno == EBUSY) {
				printf("Flush in progress\n");
				usleep(5000);
				if (audio_data->outport_flush_enable == 1)
					audio_data->outport_flush_enable =
					    0;
				continue;
			} else {
				printf("error reading the PCM samples \n");
				continue;
			}
		} else if (len != 0) {
			memcpy(&meta, audio_data->recbuf,
			       sizeof(struct meta_out));
			time =
			    (unsigned long long *)(audio_data->recbuf + 2);
			meta.ntimestamp.LowPart = (*time & 0xFFFFFFFF);
			meta.ntimestamp.HighPart =
			    ((*time >> 32) & 0xFFFFFFFF);
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
				printf
				    ("Reached end of file total PCM %d\n",
				     pcm_sourced);
				eof = 1;
			}
			len = (len - sizeof(struct meta_out));
			pcm_sourced += len;
			//              printf("len=%d, in_free_cnt=%d\n", len, in_free_cnt);
			if (len > 0) {
				memcpy(((char*)data_buf[index].pbuf.buf_addr +
					total_len),
				       (audio_data->recbuf +
					sizeof(struct meta_out)), len);
				total_len += len;
				if (eof) {
					printf
					    ("Last buffer sent to LPA\n");
					data_buf[index].pbuf.buf_len =
					    total_len;
					data_buf[index].pbuf.data_len =
					    total_len;
#ifdef DEBUG_LOCAL
					printf("pbuf.buf_addr %p\n",
					       data_buf[index].pbuf.
					       buf_addr);
					printf
					    ("write to lpa driver total_len =%d, index=%d, in_free_cnt=%d\n",
					     total_len, index,
					     in_free_cnt);
#endif
					rc = ioctl(afd, AUDIO_ASYNC_WRITE,
						   &data_buf[index].pbuf);
					if (rc < 0) {
						printf
						    ("error on async write=%d\n",
						     rc);
						break;
					}
					pthread_mutex_lock(&avail_lock);
					in_free_cnt--;
					if (in_free_cnt <= 0) {
						printf
						    ("there is no free buffer \n");
					}
					pthread_mutex_unlock(&avail_lock);
					break;

				}
				/* Next buffer overflowing */
				if ((total_len +
				     (audio_data->recsize -
				      sizeof(struct meta_out))) >
				    MP3TEST_BUFSZ) {
					/* send data to kernel */
					data_buf[index].pbuf.buf_len =
					    total_len;
					data_buf[index].pbuf.data_len =
					    total_len;
#ifdef DEBUG_LOCAL
					printf("pbuf.buf_addr %p\n",
					       data_buf[index].pbuf.
					       buf_addr);
					printf
					    ("write to lpa driver total_len =%d, index=%d, in_free_cnt=%d\n",
					     total_len, index,
					     in_free_cnt);
#endif
					rc = ioctl(afd, AUDIO_ASYNC_WRITE,
						   &data_buf[index].pbuf);
					if (rc < 0) {
						printf
						    ("error on async write=%d\n",
						     rc);
						break;
					}
					pthread_mutex_lock(&avail_lock);
					in_free_cnt--;
					if (in_free_cnt <= 0) {
						printf
						    ("there is no free buffer \n");
					}
					pthread_mutex_unlock(&avail_lock);
					index++;
					if (index >= MP3TEST_NUM_BUF)
						index = 0;
					total_len = 0;
				}
			} else if (len == 0) {
				if ((eof == 1) && (total_len > 0)) {
					printf
					    ("Last buffer sent to LPA\n");
					data_buf[index].pbuf.buf_len =
					    total_len;
					data_buf[index].pbuf.data_len =
					    total_len;
#ifdef DEBUG_LOCAL
					printf("pbuf.buf_addr %p\n",
					       data_buf[index].pbuf.
					       buf_addr);
					printf
					    ("write to lpa driver total_len =%d, index=%d, in_free_cnt=%d\n",
					     total_len, index,
					     in_free_cnt);
#endif
					rc = ioctl(afd, AUDIO_ASYNC_WRITE,
						   &data_buf[index].pbuf);
					if (rc < 0) {
						printf
						    ("error on async write=%d\n",
						     rc);
						break;
					}
					pthread_mutex_lock(&avail_lock);
					in_free_cnt--;
					if (in_free_cnt <= 0) {
						printf
						    ("there is no free buffer \n");
					}
					pthread_mutex_unlock(&avail_lock);
					break;
				} else if (eof == 1) {
					break;
				}
				printf
				    ("Unexpected case: read count zero and not EOF\n");
			}
		}
	} while (1);
	free(audio_data->recbuf);
	ioctl(ntfd, AUDIO_ABORT_GET_EVENT, 0);
	printf("lpa_dec thread close\n");
	return NULL;
}

static void *event_notify(void *arg)
{
	long ret_drv;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *)arg;
	int afd;
	int bitstream_err_cnt = 0;

	struct msm_audio_event suspend_event;

	afd = audio_data->ntfd;

	do {
#ifdef DEBUG_LOCAL
		printf("event_notify thread started\n");
#endif
		suspend_event.timeout_ms = 0;
		ret_drv = ioctl(afd, AUDIO_GET_EVENT, &suspend_event);
		if (ret_drv < 0) {
			printf
			    ("event_notify thread exiting: Got Abort event or timedout\n");
			break;
		} else {
			if (suspend_event.event_type ==
			    AUDIO_EVENT_SUSPEND) {
				printf
				    ("event_notify: AUDIO_EVENT_SUSPEND\n");
				audio_data->suspend = 1;
			} else if
			    (suspend_event.event_type ==
			     AUDIO_EVENT_RESUME) {
				printf
				    ("event_notify: AUDIO_EVENT_RESUME\n");
				audio_data->suspend = 0;
			} else if
			    (suspend_event.event_type ==
			     AUDIO_EVENT_STREAM_INFO) {
				printf
				    ("event_notify: AUDIO_EVENT_STREAM_INFO\n");
				printf
				    ("codec_type : %d\nchan_info : %d\nsample_rate : %d\nstream_info: %d\n",
				     suspend_event.event_payload.
				     stream_info.codec_type,
				     suspend_event.event_payload.
				     stream_info.chan_info,
				     suspend_event.event_payload.
				     stream_info.sample_rate,
				     suspend_event.event_payload.
				     stream_info.bit_stream_info);
				audio_data->freq =
				    suspend_event.event_payload.
				    stream_info.sample_rate;
				audio_data->channels =
				    suspend_event.event_payload.
				    stream_info.chan_info;
				if (audio_data->streaminfo_received)
					printf
					    ("warning bitstream has multiple stream info\n");
				audio_data->streaminfo_received = 1;
				if (!audio_data->suspend) {
					audio_data->outport_flush_enable =
					    1;
					ioctl(afd, AUDIO_OUTPORT_FLUSH, 0);
				}
			} else if
			    (suspend_event.event_type ==
			     AUDIO_EVENT_BITSTREAM_ERROR_INFO) {
				printf
				    ("event_notify: AUDIO_EVENT_BITSTREAM_ERROR_INFO\n");
				printf
				    ("codec_type : %d\nerror_count : %d\nerror_type : 0x%8x\n",
				     suspend_event.event_payload.
				     error_info.dec_id,
				     (0x0000FFFF & suspend_event.
				      event_payload.error_info.
				      err_msg_indicator),
				     suspend_event.event_payload.
				     error_info.err_type);
				if (audio_data->streaminfo_received)
					bitstream_err_cnt++;
				if (bitstream_err_cnt >
				    MAX_BITSTREAM_ERROR) {
					printf
					    ("Too much bit stream error, stop decoding\n");
					audio_data->bitstream_error = 1;
					/* exit from read and write call */
					ioctl(afd, AUDIO_FLUSH, 0);
				}
			}
		}
	} while (1);
	printf("event_notify thread close\n");
	return NULL;
}

static void *lpa_event(void *arg)
{
	int rc;
	struct msm_audio_event event;
	event.timeout_ms = 0;
	struct audio_pvt_data *audio_data = (struct audio_pvt_data *)arg;
	int afd = audio_data->afd;
	int pcm_consumed = 0;

	printf("lpa_event thread started\n");
	do {

		/* read event from lpa */
		event.timeout_ms = 0;
		if (!eof && (in_free_cnt < MP3TEST_NUM_BUF)) {
			printf("%s: AUDIO_GET_EVENT\n", __func__);
			rc = ioctl(afd, AUDIO_GET_EVENT, &event);
#ifdef DEBUG_LOCAL
			printf
			    ("event.event_payload.aio_buf.buf_addr = %p\n",
			     event.event_payload.aio_buf.buf_addr);
#endif
			if (rc < 0) {
				printf("%s: errno #%d", __func__, errno);
				if (eof) {
					//complete_close = 1;
					break;
				} else
					continue;
			}
			printf("%s: event %d in_free_cnt %d \n", __func__,
			       event.event_type, in_free_cnt);
			switch (event.event_type) {
			case AUDIO_EVENT_WRITE_DONE:
				pthread_mutex_lock(&avail_lock);
				in_free_cnt++;
				pthread_mutex_unlock(&avail_lock);
				data_available();
#ifdef DEBUG_LOCAL
				fprintf(stderr,
					"WRITE_DONE: addr %p len %d, in_free_cnt:%d\n",
					event.event_payload.aio_buf.
					buf_addr,
					event.event_payload.aio_buf.
					buf_len, in_free_cnt);
#endif
				pcm_consumed +=
				    event.event_payload.aio_buf.data_len;
#ifdef DEBUG_LOCAL
				printf
				    ("%s: AUDIO_EVENT_WRITE_DONE. So far pcm consumed %d\n",
				     __func__, pcm_consumed);
#endif
			}
		} else {
			struct msm_audio_stats stats;
			rc = ioctl(afd, AUDIO_GET_STATS, &stats);
			printf
			    ("current sample consumed %d bytes consumed %d\n",
			     stats.sample_count, stats.byte_count);
			if (eof) {
				if ((stats.byte_count + MP3TEST_BUFSZ) >
				    pcm_sourced) {
					/* At last buffer */
					fsync(afd);
					complete_close = 1;
					break;
				} else {
					usleep(10000);
				}
			}
		}
	} while (1);
	printf("lpa_event thread closed\n");
	return NULL;
}

static int initiate_play(struct audtest_config *clnt_config,
			 int (*fill) (void *buf, unsigned sz,
				      void *cookie), void *cookie)
{
	struct msm_audio_config config;
	struct msm_audio_config lpa_config;
	unsigned n = 0;
	pthread_t thread;
	pthread_t evt_thread1, evt_thread2;
	int sz;
	int rc;
	char *buf;
	int afd, ntfd, ipmem_fd[MP3TEST_NUM_BUF];
	int cntW = 0;
	void *pmem_ptr[MP3TEST_NUM_BUF];
	struct msm_audio_pmem_info pmem_info;
	unsigned short dec_id = 0;
	struct audio_pvt_data *audio_data =
	    (struct audio_pvt_data *)clnt_config->private_data;

	/* =========== open NT and LPA drivers */
	ntfd = open("/dev/msm_mp3", O_RDWR);
	if (ntfd < 0) {
		perror("lpa_play: cannot open nt MP3 device");
		return -1;
	}

	audio_data->ntfd = ntfd;	/* Store */

	afd = open("/dev/msm_pcm_lp_dec", O_WRONLY | O_NONBLOCK);
	if (afd < 0) {
		perror("pcm_play: cannot open pcm_lp device");
		return -1;
	}

	eof = 0;
	complete_close = 0;
	audio_data->afd = afd;	/* store */

	pthread_create(&evt_thread1, NULL, lpa_event, (void *)audio_data);
	pthread_create(&evt_thread2, NULL, event_notify,
		       (void *)audio_data);

	if (ioctl(ntfd, AUDIO_GET_CONFIG, &config)) {
		perror("could not get config");
		goto err_state;
	}

	config.meta_field = 1;
	if (ioctl
	    (ntfd, AUDIO_SET_ERR_THRESHOLD_VALUE,
	     &audio_data->err_threshold_value)) {
		perror("could not set error threshold value");
		goto err_state;
	}

	config.sample_rate = clnt_config->sample_rate;
	printf(" in initate_play, sample_rate=%d\n", config.sample_rate);
	if (ioctl(ntfd, AUDIO_SET_CONFIG, &config)) {
		perror("could not set config");
		goto err_state;
	}
	printf(" buffer_size=%d\n", config.buffer_size);
	buf = (char *)malloc(sizeof(char) * config.buffer_size);
	if (buf == NULL) {
		perror("fail to allocate buffer\n");
		goto err_state;
	}

	config.buffer_size = (config.buffer_size - sizeof(struct meta_in));

	printf
	    ("initiate_play: buffer_size=%d, buffer_count=%d, sample_rate=%d\n",
	     config.buffer_size, config.buffer_count, config.sample_rate);

	fprintf(stderr, "prefill\n");
	/* non - tunnel portion */
	struct msm_audio_pcm_config config_rec;
	printf(" selected non-tunnel part\n");
	if (ioctl(ntfd, AUDIO_GET_PCM_CONFIG, &config_rec)) {
		printf("could not get PCM config\n");
		free(buf);
		goto err_state;
	}
	printf("config_rec.pcm_feedback = %d, \
		config_rec.buffer_count = %d , \
		config_rec.buffer_size=%d \n", config_rec.pcm_feedback, config_rec.buffer_count, config_rec.buffer_size);
		config_rec.pcm_feedback = 1;

	audio_data->recsize = config_rec.buffer_size;
	audio_data->recbuf = (char *)malloc(config_rec.buffer_size);
	if (!audio_data->recbuf) {
		printf("could not allocate memory for decoding\n");
		free(buf);
		goto err_state;
	}
	memset(audio_data->recbuf, 0, config_rec.buffer_size);

	if (ioctl(ntfd, AUDIO_SET_PCM_CONFIG, &config_rec)) {
		printf("could not set PCM config\n");
		free(audio_data->recbuf);
		free(buf);
		goto err_state;
	}

	if (ioctl(afd, AUDIO_GET_SESSION_ID, &dec_id)) {
		perror("could not get decoder session id\n");
		close(afd);
		return -1;
	}
#if defined(TARGET_USES_QCOM_MM_AUDIO)
	if (devmgr_register_session(dec_id, DIR_RX) < 0) {
		return -1;
	}
#endif

	pthread_cond_init(&avail_cond, 0);
	pthread_mutex_init(&avail_lock, 0);
	pthread_cond_init(&consumed_cond, 0);
	pthread_mutex_init(&consumed_lock, 0);
	data_is_available = 0;
	data_is_consumed = 0;

/* start NT playback */
	printf(" start play back \n");
	for (n = 0; n < config.buffer_count; n++) {
		if ((sz = fill(buf, config.buffer_size, cookie)) < 0)
			break;
		if (write(ntfd, buf, sz) != sz)
			break;
	}
	cntW = cntW + config.buffer_count;

	fprintf(stderr, "start playback\n");
	if (ioctl(ntfd, AUDIO_START, 0) >= 0) {
		if (ioctl(afd, AUDIO_GET_CONFIG, &lpa_config)) {
			perror("could not get config");
			close(afd);
			return -1;
		}
		lpa_config.sample_rate = clnt_config->sample_rate;
		printf(" LPA: sample rate =%d\n", lpa_config.sample_rate);
		if (ioctl(afd, AUDIO_SET_CONFIG, &lpa_config)) {
			perror("could not set config");
			close(afd);
			return -1;
		}
		if ((ioctl(afd, AUDIO_START, 0)) >= 0) {
			fprintf(stderr, "register pmem\n");
			for (n = 0; n < MP3TEST_NUM_BUF; n++) {
				ipmem_fd[n] =
				    open("/dev/pmem_adsp", O_RDWR);
				printf("%s: ipmem_fd %x\n", __func__,
				       ipmem_fd[n]);
				pmem_ptr[n] =
				    mmap(0, MP3TEST_BUFSZ,
					 PROT_READ | PROT_WRITE,
					 MAP_SHARED, ipmem_fd[n], 0);
				printf("%s:pmem_ptr[n] =%p\n", __func__,
				       pmem_ptr[n]);
				pmem_info.fd = ipmem_fd[n];
				pmem_info.vaddr = pmem_ptr[n];

				rc = ioctl(afd, AUDIO_REGISTER_PMEM,
					   &pmem_info);
				if (rc < 0) {
					printf
					    ("error on register pmem=%d\n",
					     rc);
					break;
				}
				data_buf[n].pbuf.buf_addr = pmem_ptr[n];
				data_buf[n].flag = 0;
			}
		}
		in_free_cnt = MP3TEST_NUM_BUF;
		pcm_sourced = 0;
		printf(" in_free_cnt =%d\n", in_free_cnt);
		pthread_create(&thread, NULL, lpa_dec, (void *)audio_data);
		for (; audio_data->bitstream_error != 1;) {
			if (((sz =
			      fill(buf, config.buffer_size, cookie)) < 0)
			    || audio_data->quit
			    || audio_data->bitstream_error) {
				printf
				    (" File reached end or quit cmd issued, exit loop, sz=%d, quit=%d,bitstream_error=%d \n",
				     sz, audio_data->quit,
				     audio_data->bitstream_error);
				if (audio_data->bitstream_error == 1) {
					printf(" bit stream error \n");
					break;
				}
				struct meta_in meta;
				meta.offset = sizeof(struct meta_in);
				meta.ntimestamp.LowPart =
				    ((audio_data->frame_count *
				      20000) & 0xFFFFFFFF);
				meta.ntimestamp.HighPart =
				    (((unsigned long long)(audio_data->
							   frame_count *
							   20000) >> 32) &
				     0xFFFFFFFF);
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
				       (sizeof(config.buffer_size) +
					sizeof(struct meta_in)));
				memcpy(buf, &meta, sizeof(struct meta_in));
				if (write(ntfd, buf,
					  sizeof(struct meta_in)) < 0)
					printf("writing buffer\
                                                for EOS failed\n");
				break;
			}
			if (write(ntfd, buf, sz) != sz) {
				if (audio_data->flush_enable == 1
				    && errno == EBUSY) {
					printf("Flush in progress\n");
					while (write(ntfd, buf, sz) < 0)
						usleep(10000);
					audio_data->avail =
					    audio_data->org_avail;
					audio_data->next =
					    audio_data->org_next;
					audio_data->flush_enable = 0;
					printf("Flush done");
					continue;
				}
				printf
				    ("write return not equal to sz, exit loop\n");
				break;
			} else {
				cntW++;
				printf("lpa_play: instance=%d cntW=%d\n",
				       (int)audio_data, cntW);
			}
		}
		printf("end of lpa play\n");
	} else {
		printf("lpa_play: Unable to start driver\n");
	}
	free(buf);
	while (!complete_close)
		usleep(10000);
	ioctl(ntfd, AUDIO_STOP, 0);
	ioctl(afd, AUDIO_STOP, 0);
	for (n = 0; n < MP3TEST_NUM_BUF; n++) {
		munmap(pmem_ptr[n], MP3TEST_BUFSZ);
		close(ipmem_fd[n]);
	}
err_state:
#if defined(TARGET_USES_QCOM_MM_AUDIO)
	if (devmgr_unregister_session(dec_id, DIR_RX) < 0)
		return -1;
#endif
	close(ntfd);
	close(afd);
	return 0;
}

/* http://ccrma.stanford.edu/courses/422/projects/WaveFormat/ */

int play_file(struct audtest_config *config, int fd, size_t count)
{
	struct audio_pvt_data *audio_data =
	    (struct audio_pvt_data *)config->private_data;

	int ret_val = 0;
	char *content_buf;

	audio_data->next = (char *)malloc(count);
	printf(" play_file: count=%d,next=%p\n", count,
	       audio_data->next);
	if (!audio_data->next) {
		fprintf(stderr, "could not allocate %d bytes\n", count);
		return -1;
	}

	audio_data->org_next = audio_data->next;
	content_buf = audio_data->org_next;

	printf(" play_file: count=%d,next=%d\n", count,
	       (int)audio_data->next);
	if (read(fd, audio_data->next, count) != (ssize_t)count) {
		fprintf(stderr, "could not read %d bytes\n", count);
		free(content_buf);
		return -1;
	}

	printf(" play_file: count=%d,next=%d\n", count,
	       (int)audio_data->next);
	audio_data->avail = count;
	audio_data->org_avail = audio_data->avail;

	ret_val = initiate_play(config, fill_buffer, audio_data);
	free(content_buf);
	return ret_val;

}

int lpa_play(struct audtest_config *config)
{
	struct stat stat_buf;
	int fd;

	if (config == NULL) {
		return -1;
	}

	fd = open(config->file_name, O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "playlpa: cannot open '%s'\n",
			config->file_name);
		return -1;
	}

	(void)fstat(fd, &stat_buf);

	return play_file(config, fd, stat_buf.st_size);;
}

void *playlpa_thread(void *arg)
{
	struct audiotest_thread_context *context =
	    (struct audiotest_thread_context *)arg;
	int ret_val;

	ret_val = lpa_play(&context->config);
	free(context->config.private_data);
	free_context(context);
	pthread_exit((void *)ret_val);

	return NULL;
}

int lpaplay_read_params(void)
{
	struct audiotest_thread_context *context;
	char *token;
	int ret_val = 0;

	if ((context = get_free_context()) == NULL) {
		ret_val = -1;
	} else {
		context->config.sample_rate = 44100;
		context->config.channel_mode = 2;
		context->config.file_name = "/data/data.mp3";
		struct audio_pvt_data *audio_data;
		audio_data =
		    (struct audio_pvt_data *)
		    malloc(sizeof(struct audio_pvt_data));

		if (!audio_data) {
			printf
			    ("error allocating audio instance structure \n");
			free_context(context);
			ret_val = -1;
		}

		audio_data->err_threshold_value = 1;
		audio_data->bitstream_error = 0;
		audio_data->streaminfo_received = 0;
		context->config.sample_rate = 44100;
		context->config.channel_mode = 2;
		context->config.file_name = "/data/data.mp3";
		context->type = AUDIOTEST_TEST_MOD_LPA;
		audio_data->suspend = 0;
		audio_data->mode = 1;

		token = strtok(NULL, " ");
		while (token != NULL) {

			if (!memcmp
			    (token, "-rate=", (sizeof("-rate=" - 1)))) {
				context->config.sample_rate =
				    atoi(&token[sizeof("-rate=") - 1]);
			} else
			    if (!memcmp
				(token, "-id=", (sizeof("-id=" - 1)))) {
				context->cxt_id =
				    atoi(&token[sizeof("-id=") - 1]);
			} else {
				context->config.file_name = token;
			}
			token = strtok(NULL, " ");
		}

		context->config.private_data =
		    (struct audio_pvt_data *)audio_data;
		printf("%s : sample_rate=%d id=%d\n", __FUNCTION__,
		       context->config.sample_rate, context->cxt_id);
		pthread_create(&context->thread, NULL, playlpa_thread,
			       (void *)context);

	}
	return 0;
}

int lpa_play_control_handler(void *private_data)
{
	int drvfd, ret_val = 0;
	char *token;
	struct audio_pvt_data *audio_data =
	    (struct audio_pvt_data *)private_data;

	token = strtok(NULL, " ");
	if ((private_data != NULL) && (token != NULL)) {
		drvfd = (int)audio_data->afd;

		if (!memcmp(token, "-cmd=", (sizeof("-cmd=") - 1))) {
			token = &token[sizeof("-cmd=") - 1];
			printf("%s: cmd %s\n", __FUNCTION__, token);
#if defined(TARGET_USES_QCOM_MM_AUDIO)
			if (!strcmp(token, "volume")) {
				int rc;
				unsigned short dec_id;
				int volume;
				token = strtok(NULL, " ");
				if (!memcmp(token, "-value=",
					    (sizeof("-value=") - 1))) {
					volume =
					    atoi(&token
						 [sizeof("-value=") - 1]);
					if (ioctl
					    (drvfd, AUDIO_GET_SESSION_ID,
					     &dec_id)) {
						perror
						    ("could not get decoder session id\n");
					} else {
						printf
						    ("session %d - volume %d \n",
						     dec_id, volume);
						rc = msm_set_volume(dec_id,
								    volume);
						printf
						    ("session volume result %d\n",
						     rc);
					}
				}
			}
#endif
		}
	} else {
		ret_val = -1;
	}

	return ret_val;
}

const char *lpaplay_help_txt = "Play lpa file: type \n \
echo \"playlpa path_of_mp3_file -id=xxx\" > /tmp/audio_test \n\
Supported control command: volume \n ";

void lpaplay_help_menu(void)
{
	printf("%s\n", lpaplay_help_txt);
}

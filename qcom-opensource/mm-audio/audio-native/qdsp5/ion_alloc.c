/* ion_alloc.c - native MP3 test application
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2012 The Linux Foundation. All rights reserved.
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
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
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/msm_audio.h>
#include <linux/ion.h>
#include <errno.h>

#include "ion_alloc.h"

void* alloc_ion_buffer(int ion_fd, unsigned int bufsize)
{
	struct mmap_info *ion_data = NULL;

	ion_data = (struct mmap_info*)calloc(sizeof(struct mmap_info), 1);
	if (!ion_data) {
		printf("\n alloc_ion_buffer: ion_data allocation failed\n");
		return NULL;
	}

	/* Align the size wrt the page boundary size of 4k */
	ion_data->map_buf_size = (bufsize + 4095) & (~4095);
	ion_data->ion_alloc_data.len = ion_data->map_buf_size;
	ion_data->ion_alloc_data.align = ION_ALLOC_ALIGN;
	ion_data->ion_alloc_data.flags = ION_HEAP(ION_AUDIO_HEAP_ID);
	int rc = ioctl(ion_fd, ION_IOC_ALLOC, &ion_data->ion_alloc_data);
	if (rc < 0) {
		printf("ION_IOC_ALLOC ioctl failed\n");
		free(ion_data);
		ion_data = NULL;
		return NULL;
	}

	ion_data->ion_fd_data.handle = ion_data->ion_alloc_data.handle;
	rc = ioctl(ion_fd, ION_IOC_SHARE, &ion_data->ion_fd_data);
	if (rc < 0) {
		printf("ION_IOC SHARE ioctl failed\n");
		rc = ioctl(ion_fd, ION_IOC_FREE, &(ion_data->ion_alloc_data.handle));
		if (rc < 0)
			printf("ION_IOC_FREE ioctl failed\n");
		free(ion_data);
		ion_data = NULL;
		return NULL;
	}
	/* Map the ION shared file descriptor into current process address space */
	ion_data->pBuffer = mmap(NULL, ion_data->map_buf_size, PROT_READ | PROT_WRITE,
			MAP_SHARED, ion_data->ion_fd_data.fd, 0);
	if (MAP_FAILED == ion_data->pBuffer) {
		printf("mmap() failed \n");
		close(ion_data->ion_fd_data.fd);
		rc = ioctl(ion_fd, ION_IOC_FREE, &(ion_data->ion_alloc_data.handle));
		if (rc < 0)
			printf("ION_IOC_FREE failed\n");
		free(ion_data);
		ion_data = NULL;
		return NULL;
	}
	return ion_data;
}

int audio_register_ion(int drv_fd, struct mmap_info *ion_buf)
{
	struct msm_audio_ion_info audio_ion_buf;

	if (!ion_buf) {
		printf("%s ion null", __FUNCTION__);
		return -1;
	}

	/* Register the mapped ION buffer with the AAC driver */
	audio_ion_buf.fd = ion_buf->ion_fd_data.fd;
	audio_ion_buf.vaddr = ion_buf->pBuffer;

	if (0 > ioctl(drv_fd, AUDIO_REGISTER_ION, &audio_ion_buf)) {
		printf("\n Error in ioctl AUDIO_REGISTER_ION\n");
		return -1;
	}
	return 0;
}

int audio_deregister_ion(int drv_fd, struct mmap_info *ion_buf)
{
	struct msm_audio_ion_info audio_ion_buf;

	if (!ion_buf) {
		printf("%s ion null", __FUNCTION__);
		return -1;
	}

	audio_ion_buf.fd = ion_buf->ion_fd_data.fd;
	audio_ion_buf.vaddr = ion_buf->pBuffer;

	if (0 > ioctl(drv_fd, AUDIO_DEREGISTER_ION, &audio_ion_buf)) {
		printf("%s W-D, Buf, DEREG-ION,fd[%d] ion-buf[%p]", __FUNCTION__, ion_buf->ion_fd_data.fd, ion_buf->pBuffer);
		return -1;
	}
	return 0;
}

void free_ion_buffer(int ion_fd, struct mmap_info **ion_data)
{
	int rc;

	if (ion_data && (*ion_data)) {
		if ((*ion_data)->pBuffer &&
				(EINVAL == munmap ((*ion_data)->pBuffer,
						   (*ion_data)->map_buf_size))) {
			printf("\n Error in Unmapping the buffer %p\n",
					(*ion_data)->pBuffer);
		}
		(*ion_data)->pBuffer = NULL;

		close((*ion_data)->ion_fd_data.fd);
		rc = ioctl(ion_fd, ION_IOC_FREE, &((*ion_data)->ion_alloc_data.handle));
		if (rc < 0)
			printf("ION_IOC_FREE failed\n");
		free(*ion_data);
		*ion_data = NULL;
	} else
		printf("\n free_ion_buffer: Invalid input parameter\n");
}


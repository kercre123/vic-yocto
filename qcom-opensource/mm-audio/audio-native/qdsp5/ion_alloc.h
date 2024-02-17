/* ion_alloc.h - ION registration and deregistartion functions
 *
 * Based on native pcm test application platform/system/extras/sound/playwav.c
 *
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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
#include <linux/ion.h>

#define ION_ALLOC_ALIGN 0x1000

struct mmap_info {
        void* pBuffer;
        unsigned map_buf_size;
        unsigned filled_len;
        struct ion_fd_data ion_fd_data;
        struct ion_allocation_data ion_alloc_data;
};

void* alloc_ion_buffer(int ion_fd, unsigned int bufsize);

int audio_register_ion(int drv_fd, struct mmap_info *ion_buf);

int audio_deregister_ion(int drv_fd, struct mmap_info *ion_buf);

void free_ion_buffer(int ion_fd, struct mmap_info **ion_buffer);

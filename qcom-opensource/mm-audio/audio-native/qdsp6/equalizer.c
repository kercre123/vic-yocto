/* equalizer.c - native equalizer test application
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

#include <sys/ioctl.h>
#include <sys/stat.h>

#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <linux/ioctl.h>
#include <linux/msm_audio.h>

#include "equalizer.h"

static struct adsp_audio_eq_cfg eq_cfg;

uint32_t band_count = 0;
uint16_t band_id = 0;
uint32_t centre_freq[MAX_PRESETS][MAX_BAND_COUNT]=
	{{310,600,1000,3000,6000,0,0,0,0,0,0,0},
	{60,170,310,3000,6000,12000,14000,0,0,0,0,0},
	{60,170,310,600,1000,3000,6000,12000,0,0,0,0},
	{60,170,310,600,1000,3000,6000,12000,14000,16000,0,0},
	{310,600,1000,3000,6000,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0}};


uint32_t filter_type[MAX_PRESETS][MAX_BAND_COUNT]=
	{{NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE},
	{BASS_BOOST,BAND_BOOST,BAND_BOOST,BAND_CUT,BAND_CUT,BAND_CUT,TREBLE_CUT,NONE,NONE,NONE,NONE,NONE},
	{BASS_BOOST,BAND_BOOST,BAND_BOOST,BAND_CUT,BAND_CUT,BAND_BOOST,BAND_BOOST,TREBLE_BOOST,NONE,NONE,NONE,NONE},
	{BASS_CUT,BAND_BOOST,BAND_BOOST,BAND_BOOST,BAND_BOOST,BAND_BOOST,BAND_BOOST,BAND_BOOST,BAND_BOOST,TREBLE_BOOST,NONE,NONE},
	{BAND_CUT,BAND_CUT,BAND_BOOST,BAND_BOOST,BAND_BOOST,NONE,NONE,NONE,NONE,NONE,NONE,NONE},
	{NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE},
	{NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE},
	{NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE},
	{NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE},
	{NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE},
	{NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE},
	{NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE,NONE}};


int32_t filter_gain[MAX_PRESETS][MAX_BAND_COUNT] =
	{{3,4,4,4,3,0,0,0,0,0,0,0},
	{5,4,2,3,4,4,2,0,0,0,0,0},
	{5,3,0,-3,-2,0,5,6,0,0,0,0},
	{3,0,2,2,3,3,3,2,2,1,0,0},
	{1,3,0,4,4,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0}};


int32_t q_factor[MAX_PRESETS][MAX_BAND_COUNT] =
	{{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0}};


void set_pcm_eq_center_freq(uint32_t eq_center_freq, uint16_t eq_band_id)
{
	eq_cfg.eq_bands[eq_band_id].center_freq_hz = eq_center_freq;
}

void set_pcm_eq_filter_gain(int32_t eq_filter_gain, uint16_t eq_band_id)
{
	if ((eq_filter_gain >= FILTER_GAIN_MIN) || (eq_filter_gain <= FILTER_GAIN_MAX))
		eq_cfg.eq_bands[eq_band_id].filter_gain = eq_filter_gain;
	else
		printf("invalid filter gain: %d\n", eq_filter_gain);
}


void set_pcm_eq_filter_type(uint32_t eq_filter_type, uint16_t eq_band_id)
{
	switch (eq_filter_type)
	{
	case 0:
		eq_cfg.eq_bands[eq_band_id].filter_type = BASS_BOOST;
		break;
	case 1:
		eq_cfg.eq_bands[eq_band_id].filter_type = BASS_CUT;
		break;
	case 2:
		eq_cfg.eq_bands[eq_band_id].filter_type = BAND_BOOST;
		break;
	case 3:
		eq_cfg.eq_bands[eq_band_id].filter_type = BAND_CUT;
		break;
	case 4:
		eq_cfg.eq_bands[eq_band_id].filter_type = TREBLE_BOOST;
		break;
	case 5:
		eq_cfg.eq_bands[eq_band_id].filter_type = TREBLE_CUT;
		break;
	case 6:
		eq_cfg.eq_bands[eq_band_id].filter_type = NONE;
		break;
	}
}


void set_pcm_eq_q_factor(int32_t eq_q_factor, uint16_t eq_band_id)
{
	eq_cfg.eq_bands[eq_band_id].q_factor = eq_q_factor;
}

int set_pcm_default_eq_values(int fd, uint32_t band_value, int32_t i)
{
	int rc = 0;
	eq_cfg.enable = 1;

	band_count = band_value;
	if (band_count < MAX_BAND_COUNT)
	{
		eq_cfg.num_bands = band_count;
		for (band_id = 0; band_id < band_count; band_id++)
		{
			set_pcm_eq_center_freq(centre_freq[i][band_id], band_id);
			set_pcm_eq_filter_gain(filter_gain[i][band_id], band_id);
			set_pcm_eq_filter_type(filter_type[i][band_id], band_id);
			set_pcm_eq_q_factor(q_factor[i][band_id], band_id);
		}
	}

	if (ioctl(fd, AUDIO_SET_EQ, &eq_cfg)) {
		perror("could not set eq params");
		rc = -1;
	}

	return rc;
}



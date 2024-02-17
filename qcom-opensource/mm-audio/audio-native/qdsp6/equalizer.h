/* equalizer.h - native equalizer test application header
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
#include <linux/ioctl.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>


#define BASS_BOOST	0
#define BASS_CUT	1
#define BAND_BOOST	2
#define BAND_CUT	3
#define TREBLE_BOOST	4
#define TREBLE_CUT	5
#define NONE		6

#define FILTER_GAIN_MAX	6
#define FILTER_GAIN_MIN	-6

#define MAX_PRESETS	12
#define MAX_BAND_COUNT	12

#define PRESET_COUNT	5
#define BAND_CLUB	5
#define BAND_DANCE	7
#define BAND_TECHNO	8
#define BAND_LIVE	10
#define BAND_REGGAE	5

#define ADSP_AUDIO_MAX_EQ_BANDS 12

struct adsp_audio_eq_band {
	/* The band index, 0 .. 11 */
	uint16_t	band_idx;
	/* Filter band type */
	uint32_t	filter_type;
	/* Filter band center frequency */
	uint32_t	center_freq_hz;
	/* Filter band initial gain (dB) */
	/* Range is +12 dB to -12 dB with 1dB increments. */
	int32_t		filter_gain;
	/* Filter band quality factor expressed as q-8 number, */
	/* i.e. fixed point number with q factor of 8, */
	/* e.g. 3000/(2^8) */
	int32_t		q_factor;
} __attribute__ ((packed));

struct adsp_audio_eq_cfg {
	uint32_t			enable;
	/* Number of consequtive bands specified */
	uint32_t			num_bands;
	struct adsp_audio_eq_band	eq_bands[ADSP_AUDIO_MAX_EQ_BANDS];
} __attribute__ ((packed));

int set_pcm_default_eq_values(int fd, uint32_t band_value, int32_t i);


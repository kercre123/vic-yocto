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
#include <linux/msm_audio.h>
#include "equalizer.h"

struct eq_filter {
        uint32_t center_freq_hz;
        uint32_t filter_type;
        int32_t  filter_gain;
        int32_t	 q_factor;
} __attribute__ ((packed));

struct eq_filter eq[MAX_PRESETS][MAX_BAND_COUNT] = {
        { /* EQ blank */
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ club */
                {310,BAND_BOOST,3,1},
                {600,BAND_BOOST,4,1},
                {1000,BAND_BOOST,4,1},
                {3000,BAND_BOOST,4,1},
                {6000,BAND_BOOST,3,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ Dance */
                {60,BASS_BOOST,5,1},
                {170,BAND_BOOST,4,1},
                {310,BAND_BOOST,2,1},
                {3000,BAND_CUT,3,1},
                {6000,BAND_CUT,4,1},
                {12000,BAND_CUT,4,1},
                {14000,TREBLE_CUT,2,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ Full bass */
                {310,BASS_BOOST,6,1},
                {600,BAND_BOOST,3,1},
                {1000,BAND_BOOST,1,1},
                {3000,BAND_CUT,2,1},
                {6000,BAND_CUT,3,1},
                {12000,TREBLE_CUT,9,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ Full basstreble */
                {60,BASS_BOOST,4,1},
                {170,BAND_BOOST,3,1},
                {310,BAND_BOOST,0,1},
                {600,BAND_CUT,4,1},
                {1000,BAND_CUT,3,1},
                {3000,BAND_BOOST,1,1},
                {6000,BAND_BOOST,4,1},
                {12000,TREBLE_BOOST,6,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ Full treble */
                {310,BAND_CUT,6,1},
                {600,BAND_CUT,3,1},
                {1000,BAND_BOOST,2,1},
                {3000,BAND_BOOST,6,1},
                {6000,TREBLE_BOOST,9,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ laptop speakers/phones */
                {60,BASS_BOOST,3,1},
                {170,BAND_BOOST,6,1},
                {310,BAND_BOOST,3,1},
                {600,BAND_CUT,2,1},
                {1000,BAND_CUT,1,1},
                {3000,BAND_BOOST,1,1},
                {6000,BAND_BOOST,3,1},
                {12000,BAND_BOOST,6,1},
                {14000,BAND_BOOST,8,1},
                {16000,TREBLE_BOOST,9,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ large hall */
                {170,BASS_BOOST,6,1},
                {310,BAND_BOOST,3,1},
                {600,BAND_BOOST,0,1},
                {1000,BAND_BOOST,0,1},
                {3000,BAND_CUT,3,1},
                {6000,BAND_CUT,3,1},
                {12000,BAND_CUT,3,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ live */
                {60,BASS_CUT,3,1},
                {170,BAND_BOOST,0,1},
                {310,BAND_BOOST,2,1},
                {600,BAND_BOOST,2,1},
                {1000,BAND_BOOST,3,1},
                {3000,BAND_BOOST,3,1},
                {6000,BAND_BOOST,3,1},
                {12000,BAND_BOOST,2,1},
                {14000,BAND_BOOST,1,1},
                {16000,TREBLE_BOOST,1,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ party */
                {170,BASS_BOOST,4,0},
                {14000,TREBLE_BOOST,4,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ Pop */
                {60,BASS_CUT,1,1},
                {170,BAND_BOOST,3,1},
                {310,BAND_BOOST,4,1},
                {600,BAND_BOOST,5,1},
                {1000,BAND_BOOST,3,1},
                {3000,BAND_BOOST,0,1},
                {6000,TREBLE_CUT,1,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ Reggae */
                {310,BAND_CUT,1,1},
                {600,BAND_CUT,3,1},
                {1000,BAND_BOOST,0,1},
                {3000,BAND_BOOST,4,1},
                {6000,BAND_BOOST,4,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ rock */
                {60,BASS_BOOST,5,1},
                {170,BAND_BOOST,3,1},
                {310,BAND_CUT,3,1},
                {600,BAND_CUT,5,1},
                {1000,BAND_CUT,2,1},
                {3000,BAND_BOOST,2,1},
                {6000,TREBLE_BOOST,6,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ ska */
                {60,BASS_CUT,2,1},
                {170,BAND_CUT,3,1},
                {310,BAND_CUT,3,1},
                {600,BAND_CUT,1,1},
                {1000,BAND_BOOST,3,1},
                {3000,BAND_BOOST,4,1},
                {6000,BAND_BOOST,5,1},
                {12000,TREBLE_BOOST,5,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ soft */
                {60,BASS_BOOST,3,1},
                {170,BAND_BOOST,1,1},
                {310,BAND_CUT,1,1},
                {600,BAND_CUT,2,1},
                {1000,BAND_CUT,1,1},
                {3000,BAND_BOOST,2,1},
                {6000,BAND_BOOST,4,1},
                {12000,BAND_BOOST,5,1},
                {14000,TREBLE_BOOST,6,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ softrock */
                {170,BASS_BOOST,3,1},
                {310,BAND_BOOST,1,1},
                {600,BAND_CUT,1,1},
                {1000,BAND_CUT,2,1},
                {3000,BAND_CUT,3,1},
                {6000,BAND_CUT,2,1},
                {12000,BAND_CUT,1,1},
                {14000,BAND_CUT,2,1},
                {16000,BAND_BOOST,6,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
        { /* EQ Techno */
                {60,BASS_BOOST,5,1},
                {170,BAND_BOOST,3,1},
                {310,BAND_BOOST,0,1},
                {600,BAND_CUT,-3,1},
                {1000,BAND_CUT,-2,1},
                {3000,BAND_BOOST,0,1},
                {6000,BAND_BOOST,5,1},
                {12000,TREBLE_BOOST,6,1},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
                {0,NONE,0,0},
        },
};

#ifdef QDSP6V2
int set_pcm_default_eq_values(int fd, int preset)
{
        int rc = 0;
        struct msm_audio_eq_stream_config eq_cfg;
        int band_id = 0;

        printf("EQUALIZER PRESET: %d\n", preset);
	memset(&eq_cfg, 0 , sizeof(struct msm_audio_eq_stream_config));
	while (eq[preset][band_id].center_freq_hz)
	{
                eq_cfg.eq_bands[band_id].band_idx = band_id;
                eq_cfg.eq_bands[band_id].center_freq_hz =
                                      eq[preset][band_id].center_freq_hz;
                eq_cfg.eq_bands[band_id].filter_gain =
                                      eq[preset][band_id].filter_gain;
                eq_cfg.eq_bands[band_id].filter_type =
                                      eq[preset][band_id].filter_type;
                eq_cfg.eq_bands[band_id].q_factor =
                                      eq[preset][band_id].q_factor;
		++band_id;
        }
        eq_cfg.enable = 1;
        eq_cfg.num_bands = band_id;
	printf("EQUALIZER: NUM_BANDS=%d\n", band_id);

        if (ioctl(fd, AUDIO_SET_EQ, &eq_cfg)) {
                perror("could not set eq params");
                rc = -1;
        }
        return rc;
}
#endif
